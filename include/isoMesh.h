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


#ifndef ISOMESH_H
#define ISOMESH_H
#include "commons.h"
#include <optional>
#include <span>
#include <string_view>
#include <vector>

/**
 * The 3D game field's geometry, built in world space: one unit is one cell, x runs east, y south
 * (as on the board) and z up. It knows nothing about the camera or the screen, so the same mesh
 * can be drawn by any camera, one per eye in VR too.
 */
namespace goe::iso {

/// how an element stands in the 3D field ("Shape" in skins.json)
enum class shape {
    /// the floor: a square at height 0
    ground,
    /// a cube: walls, brick clusters, doors
    block,
    /// an upright picture facing the camera: creatures and things to pick up
    standee,
    /// a square lying just above the floor: things seen from above, like a landmine
    flat
};

/// the shape of an element type when skins.json does not say
shape defaultShape(int type);
/// "ground", "block", "standee" or "flat"; empty for anything else
std::optional<shape> parseShape(std::string_view name);

/// a corner of a triangle: position, sprite sheet pixel, and how brightly it is lit (0 to 1)
struct vertex {
    float x, y, z;
    float u, v;
    float light;
    /// the picture covers its whole square, laid over the board's colour like the 2D floor;
    /// otherwise clear pixels are cut out, so the shape keeps its outline
    bool backdrop = false;
};

/// a picture on the sprite sheet: its top left pixel and its size
struct picture {
    coords at;
    coords size;
};

/// light by the way a face looks: tops brightest, the sides a little darker each
namespace light {
constexpr float top = 1.0f;
constexpr float southFace = 0.78f;
constexpr float eastFace = 0.62f;
constexpr float northFace = 0.5f;
constexpr float westFace = 0.7f;
constexpr float standee = 0.95f;
} // namespace light

/// Collects triangles, two per square face.
class meshBuilder
{
public:
    /// how tall a block is, in cells; a little less than a standee, so walls hide less
    static constexpr float blockHeight = 0.75f;
    /// how tall a block is when it is cut away, so the avatar behind it shows
    static constexpr float cutHeight = 0.15f;
    /// how far above the floor flat things lie, so they never fight with it
    static constexpr float flatLift = 0.02f;
    /// a standee's width and height, in cells
    static constexpr float standeeSize = 0.9f;

    /**
     * eyeX, eyeY: the direction, on the floor, from the field towards the camera. Standees turn
     * to face it, and block sides facing away from it are left out.
     */
    meshBuilder(float eyeX, float eyeY);

    /// the element whose cell has its north west corner at (x, y), in the given shape; a cut
    /// block is a low one
    void add(shape s, float x, float y, const picture &p, bool cut = false);
    void ground(float x, float y, const picture &p);
    void flat(float x, float y, const picture &p);
    void block(float x, float y, const picture &p, float height = blockHeight);
    void standee(float x, float y, const picture &p);

    std::span<const vertex> vertices() const { return this->corners; }
    void clear() { this->corners.clear(); }

private:
    /// a square with corners a, b, c, d (clockwise as seen from its front), the picture's top
    /// left at a and top right at b
    void quad(const vertex &a, const vertex &b, const vertex &c, const vertex &d);
    /// a picture lying flat at height z, its top towards the north
    void lying(float x, float y, float z, const picture &p, bool backdrop);
    float eyeX;
    float eyeY;
    std::vector<vertex> corners;
};

} // namespace goe::iso

#endif // ISOMESH_H
