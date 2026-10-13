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



#define CGLTF_IMPLEMENTATION
#include "model3d.h"
#include "cgltf.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <numbers>

namespace goe::models {
namespace {
using gltfData = std::unique_ptr<cgltf_data, decltype(&cgltf_free)>;

/// the folder of a file, with its last slash, for the files a .gltf refers to
std::string folderOf(const std::string &file)
{
    const auto slash = file.find_last_of("/\\");
    return slash == std::string::npos ? std::string() : file.substr(0, slash + 1);
}

/// the picture a material paints with, as stored in the file or next to it
bool readImage(const cgltf_image &img, const std::string &folder, mesh &out)
{
    const std::string_view mime = img.mime_type ? img.mime_type : "";
    if (img.buffer_view && img.buffer_view->buffer->data) {
        const auto *start = static_cast<const unsigned char *>(img.buffer_view->buffer->data)
                            + img.buffer_view->offset;
        out.image.assign(start, start + img.buffer_view->size);
        out.imageType = mime == "image/jpeg" ? ".jpg" : ".png";
        return true;
    }
    if (!img.uri || std::string_view(img.uri).starts_with("data:"))
        return false;
    std::ifstream f(folder + img.uri, std::ios::binary);
    if (!f)
        return false;
    out.image.assign(std::istreambuf_iterator<char>(f), {});
    const std::string_view uri = img.uri;
    out.imageType = uri.ends_with(".jpg") || uri.ends_with(".jpeg") ? ".jpg" : ".png";
    return true;
}

const cgltf_image *imageOf(const cgltf_material *m)
{
    if (!m || !m->has_pbr_metallic_roughness)
        return nullptr;
    const cgltf_texture *t = m->pbr_metallic_roughness.base_color_texture.texture;
    return t ? t->image : nullptr;
}

std::array<float, 3> colourOf(const cgltf_material *m)
{
    if (!m || !m->has_pbr_metallic_roughness)
        return {1, 1, 1};
    const float *c = m->pbr_metallic_roughness.base_color_factor;
    return {c[0], c[1], c[2]};
}

/// a primitive's triangles, moved by the node's matrix and turned into the board's axes
void addTriangles(const cgltf_primitive &prim, const std::array<float, 16> &matrix, mesh &out)
{
    const cgltf_accessor *pos = nullptr, *norm = nullptr, *uv = nullptr;
    for (cgltf_size i = 0; i < prim.attributes_count; i++) {
        const auto &a = prim.attributes[i];
        if (a.type == cgltf_attribute_type_position)
            pos = a.data;
        else if (a.type == cgltf_attribute_type_normal)
            norm = a.data;
        else if (a.type == cgltf_attribute_type_texcoord && a.index == 0)
            uv = a.data;
    }
    if (!pos || prim.type != cgltf_primitive_type_triangles)
        return;
    const auto colour = colourOf(prim.material);
    const auto &m = matrix; // column major
    auto corner = [&](cgltf_size i) {
        float p[3]{0, 0, 0}, n[3]{0, 1, 0}, t[2]{0, 0};
        cgltf_accessor_read_float(pos, i, p, 3);
        if (norm)
            cgltf_accessor_read_float(norm, i, n, 3);
        if (uv)
            cgltf_accessor_read_float(uv, i, t, 2);
        const float wx = m[0] * p[0] + m[4] * p[1] + m[8] * p[2] + m[12];
        const float wy = m[1] * p[0] + m[5] * p[1] + m[9] * p[2] + m[13];
        const float wz = m[2] * p[0] + m[6] * p[1] + m[10] * p[2] + m[14];
        float nx = m[0] * n[0] + m[4] * n[1] + m[8] * n[2];
        float ny = m[1] * n[0] + m[5] * n[1] + m[9] * n[2];
        float nz = m[2] * n[0] + m[6] * n[1] + m[10] * n[2];
        const float len = std::hypot(nx, ny, nz);
        if (len > 0) {
            nx /= len;
            ny /= len;
            nz /= len;
        }
        // glTF: x right, y up, z to the front; the board: x east, y south, z up
        return models::corner{wx, wz, wy, nx, nz, ny, t[0], t[1], colour};
    };
    const cgltf_size count = prim.indices ? prim.indices->count : pos->count;
    for (cgltf_size i = 0; i + 2 < count; i += 3)
        for (cgltf_size k = 0; k < 3; k++)
            out.triangles.push_back(corner(prim.indices ? cgltf_accessor_read_index(prim.indices, i + k) : i + k));
}

void forEachCorner(mesh &m, auto &&f)
{
    for (auto &c : m.triangles)
        f(c);
}
} // namespace

box mesh::bounds() const
{
    if (this->triangles.empty())
        return {};
    box b{{this->triangles[0].x, this->triangles[0].y, this->triangles[0].z},
          {this->triangles[0].x, this->triangles[0].y, this->triangles[0].z}};
    for (const auto &c : this->triangles) {
        const std::array<float, 3> p{c.x, c.y, c.z};
        for (std::size_t a = 0; a < 3; a++) {
            b.min[a] = std::min(b.min[a], p[a]);
            b.max[a] = std::max(b.max[a], p[a]);
        }
    }
    return b;
}

std::optional<mesh> load(const std::string &file, std::span<const std::string> nodes)
{
    cgltf_options options{};
    cgltf_data *raw = nullptr;
    if (cgltf_parse_file(&options, file.c_str(), &raw) != cgltf_result_success)
        return std::nullopt;
    gltfData data(raw, &cgltf_free);
    if (cgltf_load_buffers(&options, data.get(), file.c_str()) != cgltf_result_success)
        return std::nullopt;
    mesh out;
    const cgltf_image *picture = nullptr;
    for (cgltf_size i = 0; i < data->nodes_count; i++) {
        const cgltf_node &node = data->nodes[i];
        if (!node.mesh)
            continue;
        if (!nodes.empty()
            && std::ranges::find(nodes, std::string_view(node.name ? node.name : "")) == nodes.end())
            continue;
        std::array<float, 16> matrix{};
        cgltf_node_transform_world(&node, matrix.data());
        for (cgltf_size p = 0; p < node.mesh->primitives_count; p++) {
            const auto &prim = node.mesh->primitives[p];
            const cgltf_image *img = imageOf(prim.material);
            if (img && !picture)
                picture = img;
            else if (img && img != picture)
                std::cout << file << ": only the first texture is used\n";
            addTriangles(prim, matrix, out);
        }
    }
    if (picture && !readImage(*picture, folderOf(file), out))
        std::cout << file << ": its texture could not be read\n";
    if (out.triangles.empty())
        return std::nullopt;
    return out;
}

std::optional<layout> parseLayout(std::string_view name)
{
    if (name == "cell")
        return layout::cell;
    if (name == "sides")
        return layout::sides;
    if (name == "between")
        return layout::between;
    return std::nullopt;
}

std::array<float, 3> spec::tintFor(int kind) const
{
    if (this->tints.empty())
        return this->tint;
    const int n = (int) this->tints.size();
    return this->tints[(std::size_t) (((kind % n) + n) % n)];
}

const spec *pick(std::span<const spec> specs, int subType, int facing)
{
    for (const auto &s : specs)
        if ((s.subType == -1 || s.subType == subType) && (s.facing == -1 || s.facing == facing))
            return &s;
    return nullptr;
}

mesh placed(mesh m, const spec &s)
{
    const box b = m.bounds();
    const float midX = (b.min[0] + b.max[0]) / 2;
    const float midY = (b.min[1] + b.max[1]) / 2;
    const float footprint = std::max(b.size(0), b.size(1));
    const float scale = s.scale.value_or(footprint > 0 ? 1.0f / footprint : 1.0f);
    const float angle = s.turn * std::numbers::pi_v<float> / 180.0f;
    const float co = std::cos(angle), si = std::sin(angle);
    forEachCorner(m, [&](corner &c) {
        const float x = c.x - midX, y = c.y - midY;
        // clockwise seen from above: south (0, 1) turns to west (-1, 0)
        c.x = (x * co - y * si) * scale * s.stretch[0];
        c.y = (x * si + y * co) * scale * s.stretch[1];
        c.z = c.z * scale * s.stretch[2] + s.lift;
        const float nx = c.nx * co - c.ny * si, ny = c.nx * si + c.ny * co;
        // a stretch changes which way faces look: normals scale by its inverse
        const float sx = nx / s.stretch[0], sy = ny / s.stretch[1], sz = c.nz / s.stretch[2];
        const float len = std::hypot(sx, sy, sz);
        if (len > 0) {
            c.nx = sx / len;
            c.ny = sy / len;
            c.nz = sz / len;
        }
    });
    if (s.place == layout::sides) {
        const float front = m.bounds().max[1];
        forEachCorner(m, [&](corner &c) { c.y += 0.5f - front; });
    }
    return m;
}

std::optional<mesh> lid(const mesh &m)
{
    const corner *best = nullptr;
    float bestZ = 0;
    for (std::size_t i = 0; i + 2 < m.triangles.size(); i += 3) {
        const auto &a = m.triangles[i];
        const float z = (a.z + m.triangles[i + 1].z + m.triangles[i + 2].z) / 3;
        if (a.nz > 0.9f && (!best || z > bestZ)) {
            best = &a;
            bestZ = z;
        }
    }
    if (!best)
        return std::nullopt;
    // the middle of that face's texture, so the lid takes its colour, not an edge of it
    const auto *t = best;
    const float u = (t[0].u + t[1].u + t[2].u) / 3, v = (t[0].v + t[1].v + t[2].v) / 3;
    const float z = m.bounds().max[2] - 0.002f; // just under the segments' own tops
    auto at = [&](float x, float y) { return corner{x, y, z, 0, 0, 1, u, v, best->colour}; };
    mesh out;
    out.image = m.image;
    out.imageType = m.imageType;
    out.triangles = {at(-0.5f, -0.5f), at(0.5f, -0.5f), at(0.5f, 0.5f), at(-0.5f, -0.5f), at(0.5f, 0.5f), at(-0.5f, 0.5f)};
    return out;
}

mesh turned(const mesh &m, int quarters)
{
    mesh out = m;
    for (int q = 0; q < ((quarters % 4) + 4) % 4; q++)
        forEachCorner(out, [](corner &c) {
            c = corner{-c.y, c.x, c.z, -c.ny, c.nx, c.nz, c.u, c.v, c.colour};
        });
    return out;
}

float lightOf(float nx, float ny, float nz)
{
    // from above, a little from the south west, where the camera is
    constexpr float lx = -0.3f, ly = 0.45f, lz = 1.0f;
    const float len = std::hypot(lx, ly, lz);
    const float facing = (nx * lx + ny * ly + nz * lz) / len;
    return 0.55f + 0.45f * std::max(facing, -0.4f);
}

int quartersToFace(int direction)
{
    // up, left, down, right are 0 to 3; seen from the south and turning clockwise: S, W, N, E
    if (direction < 0 || direction > 3)
        return 0;
    return ((2 - direction) % 4 + 4) % 4;
}

} // namespace goe::models
