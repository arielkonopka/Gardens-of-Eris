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



#ifndef MODEL3D_H
#define MODEL3D_H
#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

/**
 * 3D models for the 3D view, read from glTF 2.0 binary files (.glb), on the CPU only, so the
 * tests need no display. Everything is in the board's world axes: x east, y south, z up, one
 * unit one cell. glTF's own axes (y up, z towards the front) are turned into these on loading,
 * so a model's front faces south.
 */
namespace goe::models {

/// a corner of a triangle: where it is, which way its face looks, its texture point (0 to 1)
/// and its material's colour
struct corner {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    std::array<float, 3> colour{1, 1, 1};
};

struct box {
    std::array<float, 3> min{0, 0, 0};
    std::array<float, 3> max{0, 0, 0};
    float size(int axis) const { return this->max[(std::size_t) axis] - this->min[(std::size_t) axis]; }
};

/// a model's triangles (three corners each) and the picture painted on them
struct mesh {
    std::vector<corner> triangles;
    /// the texture as the file stores it (a PNG or JPEG); empty for a model with plain colours
    std::vector<unsigned char> image;
    /// ".png" or ".jpg", what Allegro needs to know to read the image
    std::string imageType;
    box bounds() const;
};

/**
 * Reads a .glb (or .gltf) file: every node's triangles, moved where the node puts them. With
 * names in nodes, only the nodes of those names are read (a door model without its door).
 * Empty when the file is missing or broken. A name starting with "builtin:" is a shape made in
 * code instead (see builtin).
 */
std::optional<mesh> load(const std::string &file, std::span<const std::string> nodes = {});

/**
 * Shapes made in code, for elements no model pack has: "blob" (a soft, wobbly ball, bouba) and
 * "star" (a spiky star, kiki). White, so a tint colours them; standing on the floor (z from 0).
 */
std::optional<mesh> builtin(std::string_view name);
/// "builtin:", what a model's file name starts with for a shape made in code
constexpr std::string_view builtinPrefix = "builtin:";

/// where a model goes on the board
enum class layout {
    /// in the middle of its cell
    cell,
    /// a wall segment on each side of the cell open to the camera, its front outwards
    sides,
    /// across the cell, between the two neighbours that are blocks (a door in its frame)
    between
};
std::optional<layout> parseLayout(std::string_view name);

/// a "Model" entry of skins.json: which file, for which subtype and facing, and how it sits
struct spec {
    std::string file;
    /// only for this subtype, or every subtype when -1
    int subType = -1;
    /// only while the element faces this way (dir::direction), or always when -1
    int facing = -1;
    /// read only these nodes of the file, or all when empty
    std::vector<std::string> nodes;
    /// cells per model unit; when empty, the model is scaled so its footprint fills one cell
    std::optional<float> scale;
    /// stretches it further along x, y and z (after the turn), e.g. a thin wall into a block
    std::array<float, 3> stretch{1, 1, 1};
    /// degrees to tip it over about the east-west axis, before anything else: -90 lays a standing
    /// arrow down pointing south, 90 pointing north. A tipped model is set on the floor
    float tip = 0;
    /// degrees clockwise, seen from above, to turn the model's front to the south
    float turn = 0;
    /// cells above the floor
    float lift = 0;
    /// multiplies its colours, e.g. a key's colour per subtype
    std::array<float, 3> tint{1, 1, 1};
    /// or one tint per subtype, in place of the one above (key and door colours)
    std::vector<std::array<float, 3>> tints;
    /// turns to face where the element faces (creatures); otherwise it keeps the turn above
    bool turns = false;
    layout place = layout::cell;

    /// the tint for an element of this subtype (kind)
    std::array<float, 3> tintFor(int kind) const;
};

/// the first entry for an element of this subtype facing this way, or nullptr when none fits
const spec *pick(std::span<const spec> specs, int subType, int facing);

/**
 * The model in its cell: its middle over the cell's middle (0, 0), its ground (the file's y = 0)
 * on the floor, scaled to cells, turned, stretched and lifted. A model laid out by sides has its
 * front pushed to the cell's southern edge; the view turns it round to the other sides.
 */
mesh placed(mesh m, const spec &s);

/**
 * A lid for a model laid out by sides: a square over the whole cell at the model's top, painted
 * like the model's highest face looking up (the top of a wall). Empty when nothing looks up.
 */
std::optional<mesh> lid(const mesh &m);

/// the same mesh turned clockwise, seen from above, by quarter turns (0 to 3)
mesh turned(const mesh &m, int quarters);

/// how bright a face looking this way is: the top brightest, the sides the camera sees next
float lightOf(float nx, float ny, float nz);

/// the eighths of a turn, clockwise seen from above, that bring a model's front (south) to face
/// d (dir::direction as an int, diagonals too)
int eighthsToFace(int direction);
/// the quarter turns that bring a model's front (south) to face one of the four sides; 0 for others
int quartersToFace(int direction);

} // namespace goe::models

#endif // MODEL3D_H
