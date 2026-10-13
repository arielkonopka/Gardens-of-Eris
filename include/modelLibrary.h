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



#ifndef MODELLIBRARY_H
#define MODELLIBRARY_H
#include "allegroHandles.h"
#include "model3d.h"
#include "rig.h"
#include "videoElementDef.h"
#include <allegro5/allegro_primitives.h>
#include <array>
#include <map>
#include <memory>
#include <span>
#include <tuple>
#include <vector>

/**
 * The 3D models of the skin, ready to draw: each one read once, when the 3D view first needs it,
 * into vertex buffers on the graphics card, one for each quarter turn with its light baked in.
 * Models painted with the same picture share one texture. An animated model is posed on the CPU
 * instead, each moment it is shown in, once a frame however many elements show it. Needs the
 * display.
 */
class modelLibrary
{
public:
    struct model {
        /// the model turned by 0 to 3 quarters, clockwise seen from above
        std::array<goe::vertexBufferHandle, 4> turns;
        int corners = 0;
        /// for a model laid out by sides: the lid over its cell
        goe::vertexBufferHandle lid;
        std::shared_ptr<ALLEGRO_BITMAP> texture;
        /// how tall it stands, in cells
        float height = 0;
        const goe::models::spec *spec = nullptr;
        /// an animated model: its skeleton and clips, posed for each frame instead of the turns
        std::unique_ptr<goe::models::rig> rig;
        /// how every pose is placed, worked out from the first moment of standing about
        goe::models::fitting fitting;
        /// the rig's clip for each move (goe::models::move), or -1
        std::array<int, goe::models::moveCount> clips{-1, -1, -1};
        bool plays(goe::models::move m) const { return this->rig && this->clips[(std::size_t) m] >= 0; }
    };
    /// poses shown more often than this a second are the same pose
    static constexpr float posesPerSecond = 30;
    modelLibrary();
    /// the model of an element of this kind, subtype and facing; nullptr when the skin gives it
    /// none, or its file cannot be read (then the 3D view draws the element's shape instead)
    const model *find(const videoElement::videoElementDef &def, int subType, int facing);
    /**
     * An animated model's corners, ready to draw: in a move at `at` (seconds into standing about
     * or walking, looped; the part of dying done, 0 to 1), turned by eighths (clockwise from
     * above) and lit. Made once a frame for each model, move, moment and turn.
     */
    std::span<const ALLEGRO_VERTEX> posed(const model &m, goe::models::move move, float at, int eighths);
    /// a new frame: the poses of the last one are forgotten
    void startFrame() { this->poses.clear(); }

private:
    std::unique_ptr<model> make(const goe::models::spec &s);
    std::shared_ptr<ALLEGRO_BITMAP> textureOf(const goe::models::mesh &m);
    /// the mesh in a vertex buffer, its light baked in; uv in the texture's pixels, as Allegro wants
    static goe::vertexBufferHandle upload(const goe::models::mesh &m, ALLEGRO_BITMAP *texture);
    /// the corners lit, the texture points in the texture's pixels
    static std::vector<ALLEGRO_VERTEX> lit(const goe::models::mesh &m, ALLEGRO_BITMAP *texture);
    /// this frame's poses, by model, move, frame of the clip and eighths
    std::map<std::tuple<const model *, goe::models::move, int, int>, std::vector<ALLEGRO_VERTEX>> poses;
    /// by the entry of skins.json it was made from; a file that could not be read stays empty
    std::map<const goe::models::spec *, std::unique_ptr<model>> models;
    /// by the picture's bytes, so models painted alike share it
    std::map<std::size_t, std::shared_ptr<ALLEGRO_BITMAP>> textures;
    /// for models with plain colours
    std::shared_ptr<ALLEGRO_BITMAP> white;
};

#endif // MODELLIBRARY_H
