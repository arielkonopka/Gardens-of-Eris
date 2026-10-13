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


#ifndef GLTFFILE_H
#define GLTFFILE_H
#include "cgltf.h"
#include "model3d.h"
#include <array>
#include <memory>
#include <span>
#include <string>

/// reading glTF files with cgltf, shared by the static models (model3d) and the animated ones (rig)
namespace goe::models::gltf {
using data = std::unique_ptr<cgltf_data, decltype(&cgltf_free)>;

/// the file with its buffers read, or empty when it is missing or broken
data open(const std::string &file);
/// whether a node is one of the names asked for (all are when none are)
bool wanted(const cgltf_node &node, std::span<const std::string> nodes);
/// the folder of a file, with its last slash, for the files a .gltf refers to
std::string folderOf(const std::string &file);
/// the picture a material paints with, as stored in the file or next to it
bool readImage(const cgltf_image &img, const std::string &folder, mesh &out);
const cgltf_image *imageOf(const cgltf_material *m);
/// a material's plain colour
std::array<float, 3> colourOf(const cgltf_material *m);

/// the one texture a model is painted with: the first one its primitives name
struct picture {
    const cgltf_image *image = nullptr;
    void take(const cgltf_primitive &prim, const std::string &file);
    /// puts the picture's bytes into the mesh
    void paint(mesh &out, const std::string &file) const;
};
} // namespace goe::models::gltf

#endif // GLTFFILE_H
