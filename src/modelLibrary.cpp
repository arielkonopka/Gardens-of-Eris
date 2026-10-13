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



#include "modelLibrary.h"
#include <allegro5/allegro_memfile.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <numbers>
#include <iostream>
#include <string_view>

namespace {
std::shared_ptr<ALLEGRO_BITMAP> shared(ALLEGRO_BITMAP *bmp)
{
    return {bmp, [](ALLEGRO_BITMAP *b) {
                if (b)
                    al_destroy_bitmap(b);
            }};
}
} // namespace

modelLibrary::modelLibrary()
{
    ALLEGRO_BITMAP *target = al_get_target_bitmap();
    this->white = shared(al_create_bitmap(1, 1));
    if (this->white) {
        al_set_target_bitmap(this->white.get());
        al_clear_to_color(al_map_rgb(255, 255, 255));
        al_set_target_bitmap(target);
    }
}

std::shared_ptr<ALLEGRO_BITMAP> modelLibrary::textureOf(const goe::models::mesh &m)
{
    if (m.image.empty())
        return this->white;
    const std::size_t key = std::hash<std::string_view>{}(
        std::string_view(reinterpret_cast<const char *>(m.image.data()), m.image.size()));
    if (auto found = this->textures.find(key); found != this->textures.end())
        return found->second;
    // al_open_memfile only reads here, despite taking a pointer it could write through
    ALLEGRO_FILE *file = al_open_memfile(const_cast<unsigned char *>(m.image.data()), (int64_t) m.image.size(), "r");
    if (!file)
        return this->white;
    const int oldFlags = al_get_new_bitmap_flags();
    al_set_new_bitmap_flags(oldFlags | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_MIPMAP);
    auto texture = shared(al_load_bitmap_f(file, m.imageType.c_str()));
    al_set_new_bitmap_flags(oldFlags);
    al_fclose(file);
    if (!texture)
        return this->white;
    this->textures[key] = texture;
    return texture;
}

std::vector<ALLEGRO_VERTEX> modelLibrary::lit(const goe::models::mesh &m, ALLEGRO_BITMAP *texture)
{
    const float tw = (float) al_get_bitmap_width(texture);
    const float th = (float) al_get_bitmap_height(texture);
    std::vector<ALLEGRO_VERTEX> vertices;
    vertices.reserve(m.triangles.size());
    for (const auto &c : m.triangles) {
        const float l = goe::models::lightOf(c.nx, c.ny, c.nz);
        // alpha 1: the shader cuts out the clear pixels, as it does for standing pictures
        vertices.push_back({c.x, c.y, c.z, c.u * tw, c.v * th,
                            al_map_rgba_f(l * c.colour[0], l * c.colour[1], l * c.colour[2], 1.0f)});
    }
    return vertices;
}

goe::vertexBufferHandle modelLibrary::upload(const goe::models::mesh &m, ALLEGRO_BITMAP *texture)
{
    const auto vertices = lit(m, texture);
    return goe::vertexBufferHandle(
        al_create_vertex_buffer(nullptr, vertices.data(), (int) vertices.size(), ALLEGRO_PRIM_BUFFER_STATIC));
}

std::unique_ptr<modelLibrary::model> modelLibrary::make(const goe::models::spec &s)
{
    if (s.animated()) {
        auto r = goe::models::rig::load(s.file, s.nodes);
        if (!r) {
            std::cout << "Model " << s.file << " could not be read, the 3D view draws the sprite\n";
            return nullptr;
        }
        auto out = std::make_unique<model>();
        out->spec = &s;
        for (std::size_t m = 0; m < goe::models::moveCount; m++) {
            out->clips[m] = s.moves[m].empty() ? -1 : r->clip(s.moves[m]);
            if (!s.moves[m].empty() && out->clips[m] < 0)
                std::cout << "Model " << s.file << " has no clip " << s.moves[m] << "\n";
        }
        goe::models::mesh standing = r->pose(out->clips[(std::size_t) goe::models::move::idle], 0);
        standing.image = r->image();
        standing.imageType = r->imageType();
        out->fitting = goe::models::fit(standing, s);
        const goe::models::mesh m = goe::models::placed(standing, s, out->fitting);
        out->texture = this->textureOf(m);
        out->height = m.bounds().max[2];
        out->corners = (int) m.triangles.size();
        out->rig = std::make_unique<goe::models::rig>(std::move(*r));
        return out;
    }
    const auto read = goe::models::load(s.file, s.nodes);
    if (!read) {
        std::cout << "Model " << s.file << " could not be read, the 3D view draws the sprite\n";
        return nullptr;
    }
    const goe::models::mesh m = goe::models::placed(*read, s);
    auto out = std::make_unique<model>();
    out->texture = this->textureOf(m);
    out->height = m.bounds().max[2];
    out->corners = (int) m.triangles.size();
    out->spec = &s;
    for (int q = 0; q < 4; q++) {
        out->turns[(std::size_t) q] = upload(goe::models::turned(m, q), out->texture.get());
        if (!out->turns[(std::size_t) q]) {
            std::cout << "Model " << s.file << ": no vertex buffer, the 3D view draws the sprite\n";
            return nullptr;
        }
    }
    if (s.place == goe::models::layout::sides)
        if (const auto top = goe::models::lid(m))
            out->lid = upload(*top, out->texture.get());
    return out;
}

const modelLibrary::model *modelLibrary::find(const videoElement::videoElementDef &def, int subType, int facing)
{
    const goe::models::spec *s = goe::models::pick(def.models, subType, facing);
    if (!s)
        return nullptr;
    auto found = this->models.find(s);
    if (found == this->models.end())
        found = this->models.emplace(s, this->make(*s)).first;
    return found->second.get();
}

std::span<const ALLEGRO_VERTEX> modelLibrary::posed(const model &m, goe::models::move move, float at, int eighths)
{
    if (!m.rig)
        return {};
    const int clip = m.clips[(std::size_t) move];
    const float length = m.rig->length(clip);
    // the clip's time: looped for standing and walking, played once over the dying
    float seconds = move == goe::models::move::death ? std::clamp(at, 0.0f, 1.0f) * length
                                                     : (length > 0 ? std::fmod(std::max(at, 0.0f), length) : 0.0f);
    const int frame = (int) std::floor(seconds * posesPerSecond);
    seconds = (float) frame / posesPerSecond;
    const auto key = std::tuple{&m, move, frame, ((eighths % 8) + 8) % 8};
    if (auto found = this->poses.find(key); found != this->poses.end())
        return found->second;
    goe::models::mesh posedMesh = goe::models::placed(m.rig->pose(clip, seconds), *m.spec, m.fitting);
    const float angle = (float) std::get<3>(key) * std::numbers::pi_v<float> / 4;
    const float co = std::cos(angle), si = std::sin(angle);
    // clockwise seen from above, as goe::models::turned turns by quarters
    for (auto &c : posedMesh.triangles) {
        c = goe::models::corner{c.x * co - c.y * si, c.x * si + c.y * co, c.z,
                                c.nx * co - c.ny * si, c.nx * si + c.ny * co, c.nz, c.u, c.v, c.colour};
    }
    return this->poses.emplace(key, lit(posedMesh, m.texture.get())).first->second;
}
