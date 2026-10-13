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


#ifndef RIG_H
#define RIG_H
#include "model3d.h"
#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace goe::models {

/// a 4 x 4 matrix, column major as glTF stores it
using matrix = std::array<float, 16>;

/**
 * An animated model from a glTF file: its meshes, the skeleton they hang on, and the animation
 * clips that move the skeleton (walking, standing about, dying). Read on the CPU like the static
 * models, so tests need no display; pose() bends the mesh into one moment of a clip, in the
 * board's axes like load() gives.
 */
class rig
{
public:
    /// the file's meshes (only the named nodes, when names are given) with their skeleton and
    /// clips; empty when the file is missing, broken or has no meshes
    static std::optional<rig> load(const std::string &file, std::span<const std::string> nodes = {});

    /// the clip of this name, or -1 when the file has none
    int clip(std::string_view name) const;
    /// how long a clip runs, in seconds; 0 for -1
    float length(int clip) const;
    std::size_t clips() const { return this->animations.size(); }
    /**
     * The mesh at `seconds` into a clip, held at its ends; clip -1 is the pose the file stores
     * the meshes in (for a character, standing with its arms out). Only the triangles: the
     * picture is image(), the same for every pose.
     */
    mesh pose(int clip, float seconds) const;
    /// the texture as the file stores it, and its type, as mesh::image and mesh::imageType
    const std::vector<unsigned char> &image() const { return this->bound.image; }
    const std::string &imageType() const { return this->bound.imageType; }

private:
    struct node {
        int parent = -1;
        std::array<float, 3> translation{0, 0, 0};
        std::array<float, 4> rotation{0, 0, 0, 1};
        std::array<float, 3> scale{1, 1, 1};
    };
    /// what moves a corner: a node, after the inverse of where it stood when the mesh was bound
    struct bone {
        int node;
        matrix inverse;
    };
    /// up to four bones pull each corner, by weights that add up to 1
    struct influence {
        std::array<std::uint16_t, 4> bones{0, 0, 0, 0};
        std::array<float, 4> weights{1, 0, 0, 0};
    };
    struct channel {
        int node;
        enum class part { translation, rotation, scale } what;
        bool step = false;
        std::vector<float> times;
        /// 3 numbers a key, or 4 for a rotation
        std::vector<float> values;
    };
    struct animation {
        std::string name;
        float length = 0;
        std::vector<channel> channels;
    };
    std::vector<node> nodes;
    /// parents before their children
    std::vector<int> order;
    std::vector<bone> bones;
    /// the corners in the file's own axes and places, as the bones move them
    mesh bound;
    std::vector<influence> influences;
    std::vector<animation> animations;
};

} // namespace goe::models

#endif // RIG_H
