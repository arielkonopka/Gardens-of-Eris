/*
 * Copyright (c) 2026, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// al_set_new_bitmap_depth is still marked unstable in Allegro 5.2; before any Allegro header
#define ALLEGRO_UNSTABLE 1
#include "isoFieldView.h"
#include "chamber.h"
#include "elementView.h"
#include "elements.h"
#include "spriteFrame.h"
#include "videoDriver.h"
#include "videoManager.h"
#include "viewPoint.h"
#include <array>
#include <cmath>

namespace {
/// the camera looks from the south west, so north is up and to the right on screen
constexpr float eyeX = -1.0f;
constexpr float eyeY = 1.0f;
/// how far the camera stands from what it looks at, in cells; it only has to clear the walls
constexpr float eyeDistance = 60.0f;
/// how much higher than the floor the camera aims, so the field sits in the middle of the screen
constexpr float aimHeight = 0.5f;
/// blocks between the avatar and the camera, this far towards the camera and this far to the
/// sides, are cut low, so the avatar is never hidden behind a wall
constexpr float cutTowards = 2.6f;
constexpr float cutAcross = 1.6f;

goe::bitmapHandle depthBitmap(int w, int h)
{
    const int oldDepth = al_get_new_bitmap_depth();
    al_set_new_bitmap_depth(24);
    goe::bitmapHandle bmp(al_create_bitmap(w, h));
    al_set_new_bitmap_depth(oldDepth);
    return bmp;
}

void setVector(const char *name, const std::array<float, 4> &v)
{
    al_set_shader_float_vector(name, 4, v.data(), 1);
}
} // namespace

isoFieldView::isoFieldView(coords fieldSize, coords tile, int sheetSpacing, const std::string &fogBitmap)
    : tileSize(tile)
    , spacing(sheetSpacing)
    , fieldPx(fieldSize)
    , field(depthBitmap(fieldSize.x, fieldSize.y))
    , mesh(eyeX, eyeY)
{
    // how far the screen reaches on the floor: across the line of sight, and along it
    const float ppc = zoom * (float) tile.x;
    const float across = (float) fieldSize.x / 2 / ppc;
    const float along = (float) fieldSize.y / 2 / ppc / std::sin(pitch);
    this->radius = (int) std::ceil(std::hypot(across, along)) + 2;
    const int side = (2 * this->radius + 1);
    this->fog.setup(side * tile.x, side * tile.y, tile, fogBitmap);
    this->shaderId = videoManager::getInstance().setupShader("data/shaders/isoVertex.glvs",
                                                             "data/shaders/isoPixel.glps");
}

bool isoFieldView::ready() const
{
    return this->field && al_get_bitmap_depth(this->field.get()) > 0
           && videoManager::getInstance().getShader(this->shaderId) != nullptr;
}

void isoFieldView::buildMesh(const fieldFrame &frame, coords origin, float cx, float cy)
{
    const auto board = frame.focus->getBoard();
    this->shownBoard = board;
    const float ppc = zoom * (float) this->tileSize.x;
    // on the floor, how far from the centre the screen's edges are, plus room for walls and moves
    const float across = (float) this->fieldPx.x / 2 / ppc + 1.5f;
    const float along = (float) this->fieldPx.y / 2 / ppc / std::sin(pitch) + 2.0f;
    const float len = std::hypot(eyeX, eyeY);
    const float fx = -eyeX / len, fy = -eyeY / len; // the line of sight on the floor
    const auto floorDef = videoDriver::getInstance().getVideoElement(bElemTypes::_floorType);
    const coords sheet = this->tileSize;
    auto pictureOf = [&](coords cell) {
        return goe::iso::picture{goe::sprites::pixelOf(cell, sheet, this->spacing), sheet};
    };
    // where the avatar is, from the window's corner, on its way between cells too
    const coords focusCell = frame.focus->getStats()->getMyPosition() - origin;
    const coords focusMove = elementView::offset(*frame.focus, this->tileSize);
    const float focusX = (float) focusCell.x + (float) focusMove.x / (float) this->tileSize.x;
    const float focusY = (float) focusCell.y + (float) focusMove.y / (float) this->tileSize.y;
    auto inFrontOfFocus = [&](int x, int y) {
        const float dx = (float) x - focusX, dy = (float) y - focusY;
        const float towards = -(dx * fx + dy * fy);
        return towards > 0 && towards <= cutTowards && std::abs(dx * fy - dy * fx) <= cutAcross;
    };
    this->mesh.clear();
    this->placed.clear();
    const int side = 2 * this->radius + 1;
    for (int x = 0; x < side; x++)
        for (int y = 0; y < side; y++) {
            const float dx = (float) x + 0.5f - (cx - (float) origin.x);
            const float dy = (float) y + 0.5f - (cy - (float) origin.y);
            if (std::abs(dx * fy - dy * fx) > across || std::abs(dx * fx + dy * fy) > along)
                continue;
            const coords np = origin + coords(x, y);
            if (board->isVisible(np) >= 255 && !viewPoint::get_instance().isPointVisible(np)) {
                // nobody has been here: plain floor, which the fog covers
                if (floorDef && !floorDef->defArray.empty() && !floorDef->defArray[0].empty()
                    && !floorDef->defArray[0][0].empty()) {
                    if (!this->addModel(*floorDef, 0, 0, np, (float) x, (float) y, false))
                        this->mesh.ground((float) x, (float) y, pictureOf(floorDef->defArray[0][0][0]));
                }
                continue;
            }
            for (auto e = board->getElement(np); e; e = e->getStats()->getSteppingOn()) {
                const auto ve = videoDriver::getInstance().getVideoElement(e->getType());
                if (!ve)
                    continue;
                const auto cell = goe::sprites::frame(*e, *ve);
                if (!cell)
                    continue;
                const coords off = elementView::offset(*e, this->tileSize);
                const float ex = (float) x + (float) off.x / (float) this->tileSize.x;
                const float ey = (float) y + (float) off.y / (float) this->tileSize.y;
                const auto &stats = *e->getStats();
                // exploding, dying and teleporting keep their sprite animations
                const bool changing = stats.isDying() || stats.isDestroying() || stats.isTeleporting();
                if (changing
                    || !this->addModel(*ve, e->getAttrs()->getSubtype(), (int) stats.getFacing(), np, ex, ey,
                                       inFrontOfFocus(x, y)))
                    this->mesh.add(ve->shape, ex, ey, pictureOf(*cell), inFrontOfFocus(x, y));
            }
        }
}

bool isoFieldView::joins(coords at) const
{
    const auto e = this->shownBoard ? this->shownBoard->getElement(at) : nullptr;
    if (!e)
        return false;
    const auto ve = videoDriver::getInstance().getVideoElement(e->getType());
    return ve && ve->shape == goe::iso::shape::block;
}

bool isoFieldView::addModel(const videoElement::videoElementDef &ve,
                            int subType,
                            int facing,
                            coords at,
                            float x,
                            float y,
                            bool cut)
{
    const auto *m = this->models.find(ve, subType, facing);
    if (!m)
        return false;
    const float squash = cut && ve.shape == goe::iso::shape::block && m->height > 0
                             ? std::min(1.0f, goe::iso::meshBuilder::cutHeight / m->height)
                             : 1.0f;
    const auto tint = m->spec->tintFor(subType);
    auto put = [&](int quarters, bool lid = false) { this->placed.push_back({m, x, y, quarters, squash, tint, lid}); };
    switch (m->spec->place) {
    case goe::models::layout::cell:
        put(m->spec->turns ? goe::models::quartersToFace(facing) : 0);
        break;
    case goe::models::layout::sides:
        // a segment on each side the camera sees, unless another block stands there; a lid on top
        for (const auto d : {dir::direction::UP, dir::direction::LEFT, dir::direction::DOWN, dir::direction::RIGHT}) {
            const coords step = dir::directionToCoordsMap[(std::size_t) d];
            if ((float) step.x * eyeX + (float) step.y * eyeY > 0 && !this->joins(at + step))
                put(goe::models::quartersToFace((int) d));
        }
        if (m->lid)
            put(0, true);
        break;
    case goe::models::layout::between:
        put(this->joins(at + coords(-1, 0)) || this->joins(at + coords(1, 0)) ? 0 : 1);
        break;
    }
    return true;
}

void isoFieldView::drawModels()
{
    for (const auto &pm : this->placed) {
        ALLEGRO_TRANSFORM t;
        al_identity_transform(&t);
        al_scale_transform_3d(&t, 1, 1, pm.squash);
        al_translate_transform_3d(&t, pm.x + 0.5f, pm.y + 0.5f, 0);
        al_set_shader_matrix("placement", &t);
        al_set_shader_float_vector("tint", 3, pm.tint.data(), 1);
        if (pm.lid)
            al_draw_vertex_buffer(pm.model->lid.get(), pm.model->texture.get(), 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
        else
            al_draw_vertex_buffer(pm.model->turns[(std::size_t) pm.quarters].get(),
                                  pm.model->texture.get(),
                                  0,
                                  pm.model->corners,
                                  ALLEGRO_PRIM_TRIANGLE_LIST);
    }
}

void isoFieldView::useCamera(float cx, float cy)
{
    const float len = std::hypot(eyeX, eyeY);
    const float flat = std::cos(pitch) * eyeDistance;
    ALLEGRO_TRANSFORM view;
    al_identity_transform(&view);
    al_build_camera_transform(&view,
                              cx + eyeX / len * flat,
                              cy + eyeY / len * flat,
                              aimHeight + std::sin(pitch) * eyeDistance,
                              cx,
                              cy,
                              aimHeight,
                              0,
                              0,
                              1);
    al_use_transform(&view);
    const float ppc = zoom * (float) this->tileSize.x;
    const float hw = (float) this->fieldPx.x / 2 / ppc;
    const float hh = (float) this->fieldPx.y / 2 / ppc;
    ALLEGRO_TRANSFORM projection;
    al_identity_transform(&projection);
    // the board's y runs south, so the world is left handed: x is mirrored to undo it. Allegro
    // maps the camera's z straight to depth, so in front of the camera is near -1 and far -2D.
    al_orthographic_transform(&projection, hw, hh, -1.0f, -hw, -hh, -2 * eyeDistance);
    al_use_projection_transform(&projection);
}

void isoFieldView::setFogUniforms(coords origin)
{
    ALLEGRO_BITMAP *maskBmp = this->fog.maskTexture();
    ALLEGRO_BITMAP *pattern = this->fog.patternTexture();
    const coords maskTex = fogLayer::textureSize(maskBmp);
    const coords patternTex = fogLayer::textureSize(pattern);
    const float cell = (float) fogMask::cellPx;
    const int pw = al_get_bitmap_width(pattern);
    const int ph = al_get_bitmap_height(pattern);
    al_set_shader_sampler("fogMask", maskBmp, 1);
    al_set_shader_sampler("fogPattern", pattern, 2);
    const std::array<float, 2> tilePx{(float) this->tileSize.x, (float) this->tileSize.y};
    al_set_shader_float_vector("tilePx", 2, tilePx.data(), 1);
    setVector("maskMap",
              {1.0f / (cell * (float) maskTex.x),
               1.0f / (cell * (float) maskTex.y),
               (float) this->fog.maskSize().y / (float) maskTex.y,
               0.0f});
    setVector("patternMap",
              {(float) pw, (float) ph, (float) pw / (float) patternTex.x, (float) ph / (float) patternTex.y});
    // kept inside one pattern tile, so the shader never meets huge world coordinates
    const std::array<float, 2> offset{(float) floorMod(origin.x * this->tileSize.x, pw),
                                      (float) floorMod(origin.y * this->tileSize.y, ph)};
    al_set_shader_float_vector("patternOffset", 2, offset.data(), 1);
    al_set_shader_float("minVisibility", fogLayer::minVisibility);
    al_set_shader_float_vector("boardColour", 3, this->boardColour.data(), 1);
}

void isoFieldView::draw(const fieldFrame &frame)
{
    const auto board = frame.focus ? frame.focus->getBoard() : nullptr;
    const auto floorDef = videoDriver::getInstance().getVideoElement(bElemTypes::_floorType);
    ALLEGRO_SHADER *shader = videoManager::getInstance().getShader(this->shaderId);
    if (!board || !this->field || !floorDef || !shader)
        return;
    // the middle of the field, in cells, as the 2D view has it
    const float cx = frame.camera.leftCells() + (float) frame.cells.x / 2;
    const float cy = frame.camera.topCells() + (float) frame.cells.y / 2;
    // everything is placed from the window's corner, so the numbers stay small far out in the maze
    const coords origin((int) std::floor(cx) - this->radius, (int) std::floor(cy) - this->radius);
    this->buildMesh(frame, origin, cx, cy);
    const colour c = board->getChColour();
    this->boardColour = {(float) c.r / 255.0f, (float) c.g / 255.0f, (float) c.b / 255.0f};
    const int side = 2 * this->radius + 1;
    this->fog.refresh(viewPoint::get_instance().getViewPoints(origin, origin + coords(side, side)));

    this->vertices.clear();
    for (const auto &v : this->mesh.vertices())
        this->vertices.push_back({v.x, v.y, v.z, v.u, v.v, al_map_rgba_f(v.light, v.light, v.light, v.backdrop ? 0.0f : 1.0f)});

    ALLEGRO_BITMAP *target = al_get_target_bitmap();
    al_set_target_bitmap(this->field.get());
    this->useCamera(cx - (float) origin.x, cy - (float) origin.y);
    al_set_render_state(ALLEGRO_DEPTH_TEST, 1);
    al_clear_to_color(fogLayer::plainColour());
    al_clear_depth_buffer(1.0f);
    al_use_shader(shader);
    this->setFogUniforms(origin);
    ALLEGRO_TRANSFORM still;
    al_identity_transform(&still);
    al_set_shader_matrix("placement", &still);
    const std::array<float, 3> noTint{1, 1, 1};
    al_set_shader_float_vector("tint", 3, noTint.data(), 1);
    if (!this->vertices.empty())
        al_draw_prim(this->vertices.data(),
                     nullptr,
                     floorDef->sprites.get(),
                     0,
                     (int) this->vertices.size(),
                     ALLEGRO_PRIM_TRIANGLE_LIST);
    this->drawModels();
    al_use_shader(nullptr);
    al_set_render_state(ALLEGRO_DEPTH_TEST, 0);
    al_set_target_bitmap(target);
    al_draw_bitmap(this->field.get(), (float) frame.at.x, (float) frame.at.y, 0);
}
