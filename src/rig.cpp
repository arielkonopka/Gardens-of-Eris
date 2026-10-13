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


#include "rig.h"
#include "gltfFile.h"
#include <algorithm>
#include <cmath>
#include <map>

namespace goe::models {
namespace {
constexpr matrix identity{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

matrix times(const matrix &a, const matrix &b)
{
    matrix c{};
    for (std::size_t col = 0; col < 4; col++)
        for (std::size_t row = 0; row < 4; row++) {
            float sum = 0;
            for (std::size_t k = 0; k < 4; k++)
                sum += a[k * 4 + row] * b[col * 4 + k];
            c[col * 4 + row] = sum;
        }
    return c;
}

/// translation, then rotation (a unit quaternion x, y, z, w), then scale, as glTF composes them
matrix compose(const std::array<float, 3> &t, const std::array<float, 4> &q, const std::array<float, 3> &s)
{
    const float x = q[0], y = q[1], z = q[2], w = q[3];
    return {(1 - 2 * (y * y + z * z)) * s[0], 2 * (x * y + z * w) * s[0], 2 * (x * z - y * w) * s[0], 0,
            2 * (x * y - z * w) * s[1], (1 - 2 * (x * x + z * z)) * s[1], 2 * (y * z + x * w) * s[1], 0,
            2 * (x * z + y * w) * s[2], 2 * (y * z - x * w) * s[2], (1 - 2 * (x * x + y * y)) * s[2], 0,
            t[0], t[1], t[2], 1};
}

std::array<float, 4> normalised(std::array<float, 4> q)
{
    const float len = std::sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    if (len > 0)
        for (auto &v : q)
            v /= len;
    return q;
}

/// a node's matrix taken apart, for the few files that store matrices instead of the parts
void takeApart(const float *m, std::array<float, 3> &t, std::array<float, 4> &q, std::array<float, 3> &s)
{
    t = {m[12], m[13], m[14]};
    for (std::size_t c = 0; c < 3; c++)
        s[c] = std::hypot(m[c * 4], m[c * 4 + 1], m[c * 4 + 2]);
    auto r = [&](int row, int col) { return s[(std::size_t) col] > 0 ? m[col * 4 + row] / s[(std::size_t) col] : 0.0f; };
    const float trace = r(0, 0) + r(1, 1) + r(2, 2);
    if (trace > 0) {
        const float k = 0.5f / std::sqrt(trace + 1);
        q = {(r(2, 1) - r(1, 2)) * k, (r(0, 2) - r(2, 0)) * k, (r(1, 0) - r(0, 1)) * k, 0.25f / k};
    } else if (r(0, 0) > r(1, 1) && r(0, 0) > r(2, 2)) {
        const float k = 2 * std::sqrt(1 + r(0, 0) - r(1, 1) - r(2, 2));
        q = {0.25f * k, (r(0, 1) + r(1, 0)) / k, (r(0, 2) + r(2, 0)) / k, (r(2, 1) - r(1, 2)) / k};
    } else if (r(1, 1) > r(2, 2)) {
        const float k = 2 * std::sqrt(1 + r(1, 1) - r(0, 0) - r(2, 2));
        q = {(r(0, 1) + r(1, 0)) / k, 0.25f * k, (r(1, 2) + r(2, 1)) / k, (r(0, 2) - r(2, 0)) / k};
    } else {
        const float k = 2 * std::sqrt(1 + r(2, 2) - r(0, 0) - r(1, 1));
        q = {(r(0, 2) + r(2, 0)) / k, (r(1, 2) + r(2, 1)) / k, 0.25f * k, (r(1, 0) - r(0, 1)) / k};
    }
    q = normalised(q);
}

std::vector<float> floats(const cgltf_accessor *a, std::size_t perKey)
{
    std::vector<float> out(a->count * perKey);
    for (cgltf_size i = 0; i < a->count; i++)
        cgltf_accessor_read_float(a, i, &out[i * perKey], perKey);
    return out;
}
} // namespace

std::optional<rig> rig::load(const std::string &file, std::span<const std::string> nodes)
{
    const auto data = gltf::open(file);
    if (!data)
        return std::nullopt;
    rig r;
    const auto indexOf = [&](const cgltf_node *n) { return (int) (n - data->nodes); };
    r.nodes.resize(data->nodes_count);
    for (cgltf_size i = 0; i < data->nodes_count; i++) {
        const cgltf_node &n = data->nodes[i];
        node &out = r.nodes[i];
        out.parent = n.parent ? indexOf(n.parent) : -1;
        if (n.has_matrix)
            takeApart(n.matrix, out.translation, out.rotation, out.scale);
        if (n.has_translation)
            std::copy_n(n.translation, 3, out.translation.begin());
        if (n.has_rotation)
            std::copy_n(n.rotation, 4, out.rotation.begin());
        if (n.has_scale)
            std::copy_n(n.scale, 3, out.scale.begin());
    }
    // parents before children: walk down from every root
    std::vector<int> stack;
    for (int i = (int) r.nodes.size() - 1; i >= 0; i--)
        if (r.nodes[(std::size_t) i].parent < 0)
            stack.push_back(i);
    while (!stack.empty()) {
        const int i = stack.back();
        stack.pop_back();
        r.order.push_back(i);
        const cgltf_node &n = data->nodes[i];
        for (cgltf_size c = n.children_count; c-- > 0;)
            stack.push_back(indexOf(n.children[c]));
    }

    std::map<const cgltf_skin *, std::vector<std::uint16_t>> skinBones;
    auto bonesOf = [&](const cgltf_skin *skin) -> const std::vector<std::uint16_t> & {
        auto found = skinBones.find(skin);
        if (found != skinBones.end())
            return found->second;
        std::vector<std::uint16_t> list;
        for (cgltf_size j = 0; j < skin->joints_count; j++) {
            matrix inverse = identity;
            if (skin->inverse_bind_matrices)
                cgltf_accessor_read_float(skin->inverse_bind_matrices, j, inverse.data(), 16);
            list.push_back((std::uint16_t) r.bones.size());
            r.bones.push_back({indexOf(skin->joints[j]), inverse});
        }
        return skinBones.emplace(skin, std::move(list)).first->second;
    };
    gltf::picture picture;
    for (cgltf_size i = 0; i < data->nodes_count; i++) {
        const cgltf_node &n = data->nodes[i];
        if (!n.mesh || !gltf::wanted(n, nodes))
            continue;
        // a skinned mesh hangs on its joints; any other moves with its own node (a sword in a hand)
        const std::vector<std::uint16_t> *joints = n.skin ? &bonesOf(n.skin) : nullptr;
        const auto own = (std::uint16_t) r.bones.size();
        if (!joints)
            r.bones.push_back({(int) i, identity});
        for (cgltf_size p = 0; p < n.mesh->primitives_count; p++) {
            const cgltf_primitive &prim = n.mesh->primitives[p];
            const cgltf_accessor *pos = nullptr, *norm = nullptr, *uv = nullptr, *js = nullptr, *ws = nullptr;
            for (cgltf_size a = 0; a < prim.attributes_count; a++) {
                const auto &at = prim.attributes[a];
                if (at.type == cgltf_attribute_type_position)
                    pos = at.data;
                else if (at.type == cgltf_attribute_type_normal)
                    norm = at.data;
                else if (at.type == cgltf_attribute_type_texcoord && at.index == 0)
                    uv = at.data;
                else if (at.type == cgltf_attribute_type_joints && at.index == 0)
                    js = at.data;
                else if (at.type == cgltf_attribute_type_weights && at.index == 0)
                    ws = at.data;
            }
            if (!pos || prim.type != cgltf_primitive_type_triangles)
                continue;
            picture.take(prim, file);
            const auto colour = gltf::colourOf(prim.material);
            const cgltf_size count = prim.indices ? prim.indices->count : pos->count;
            for (cgltf_size k = 0; k + 2 < count; k += 3)
                for (cgltf_size c = 0; c < 3; c++) {
                    const cgltf_size v = prim.indices ? cgltf_accessor_read_index(prim.indices, k + c) : k + c;
                    float xyz[3]{0, 0, 0}, nrm[3]{0, 1, 0}, st[2]{0, 0};
                    cgltf_accessor_read_float(pos, v, xyz, 3);
                    if (norm)
                        cgltf_accessor_read_float(norm, v, nrm, 3);
                    if (uv)
                        cgltf_accessor_read_float(uv, v, st, 2);
                    r.bound.triangles.push_back({xyz[0], xyz[1], xyz[2], nrm[0], nrm[1], nrm[2], st[0], st[1], colour});
                    influence inf;
                    if (joints && js && ws) {
                        cgltf_uint j[4]{0, 0, 0, 0};
                        float w[4]{0, 0, 0, 0};
                        cgltf_accessor_read_uint(js, v, j, 4);
                        cgltf_accessor_read_float(ws, v, w, 4);
                        const float sum = w[0] + w[1] + w[2] + w[3];
                        for (std::size_t b = 0; b < 4; b++) {
                            inf.bones[b] = j[b] < joints->size() ? (*joints)[j[b]] : (*joints)[0];
                            inf.weights[b] = sum > 0 ? w[b] / sum : (b == 0 ? 1.0f : 0.0f);
                        }
                    } else
                        inf.bones = {own, own, own, own};
                    r.influences.push_back(inf);
                }
        }
    }
    if (r.bound.triangles.empty())
        return std::nullopt;
    picture.paint(r.bound, file);

    for (cgltf_size a = 0; a < data->animations_count; a++) {
        const cgltf_animation &anim = data->animations[a];
        animation out;
        out.name = anim.name ? anim.name : "";
        for (cgltf_size c = 0; c < anim.channels_count; c++) {
            const cgltf_animation_channel &ch = anim.channels[c];
            if (!ch.target_node || !ch.sampler || !ch.sampler->input || !ch.sampler->output)
                continue;
            channel k;
            k.node = indexOf(ch.target_node);
            std::size_t width = 3;
            if (ch.target_path == cgltf_animation_path_type_translation)
                k.what = channel::part::translation;
            else if (ch.target_path == cgltf_animation_path_type_rotation) {
                k.what = channel::part::rotation;
                width = 4;
            } else if (ch.target_path == cgltf_animation_path_type_scale)
                k.what = channel::part::scale;
            else
                continue; // morph weights: none of the packs uses them
            k.times = floats(ch.sampler->input, 1);
            auto values = floats(ch.sampler->output, width);
            if (ch.sampler->interpolation == cgltf_interpolation_type_cubic_spline) {
                // three per key (in tangent, value, out tangent): the values are enough
                std::vector<float> only;
                for (std::size_t key = 0; key < k.times.size(); key++)
                    only.insert(only.end(), values.begin() + (long) ((3 * key + 1) * width),
                                values.begin() + (long) ((3 * key + 2) * width));
                values = std::move(only);
            }
            if (k.times.empty() || values.size() < k.times.size() * width)
                continue;
            k.step = ch.sampler->interpolation == cgltf_interpolation_type_step;
            k.values = std::move(values);
            out.length = std::max(out.length, k.times.back());
            out.channels.push_back(std::move(k));
        }
        r.animations.push_back(std::move(out));
    }
    return r;
}

int rig::clip(std::string_view name) const
{
    for (std::size_t i = 0; i < this->animations.size(); i++)
        if (this->animations[i].name == name)
            return (int) i;
    return -1;
}

float rig::length(int clip) const
{
    return clip >= 0 && clip < (int) this->animations.size() ? this->animations[(std::size_t) clip].length : 0.0f;
}

mesh rig::pose(int clip, float seconds) const
{
    std::vector<node> now = this->nodes;
    if (clip >= 0 && clip < (int) this->animations.size())
        for (const auto &ch : this->animations[(std::size_t) clip].channels) {
            const std::size_t width = ch.what == channel::part::rotation ? 4 : 3;
            const auto after = std::ranges::upper_bound(ch.times, seconds);
            const std::size_t hi = std::min((std::size_t) (after - ch.times.begin()), ch.times.size() - 1);
            const std::size_t lo = hi > 0 ? hi - 1 : 0;
            float k = 0;
            if (hi != lo && !ch.step && ch.times[hi] > ch.times[lo])
                k = std::clamp((seconds - ch.times[lo]) / (ch.times[hi] - ch.times[lo]), 0.0f, 1.0f);
            if (seconds >= ch.times.back())
                k = 1;
            const float *a = &ch.values[lo * width], *b = &ch.values[hi * width];
            node &n = now[(std::size_t) ch.node];
            if (ch.what == channel::part::rotation) {
                // the short way round, then back onto the unit sphere
                const float dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
                const float sign = dot < 0 ? -1.0f : 1.0f;
                std::array<float, 4> q{};
                for (std::size_t i = 0; i < 4; i++)
                    q[i] = a[i] * (1 - k) + sign * b[i] * k;
                n.rotation = normalised(q);
            } else {
                auto &v = ch.what == channel::part::translation ? n.translation : n.scale;
                for (std::size_t i = 0; i < 3; i++)
                    v[i] = a[i] * (1 - k) + b[i] * k;
            }
        }
    std::vector<matrix> world(now.size(), identity);
    for (const int i : this->order) {
        const node &n = now[(std::size_t) i];
        const matrix local = compose(n.translation, n.rotation, n.scale);
        world[(std::size_t) i] = n.parent >= 0 ? times(world[(std::size_t) n.parent], local) : local;
    }
    std::vector<matrix> skinning;
    skinning.reserve(this->bones.size());
    for (const auto &b : this->bones)
        skinning.push_back(times(world[(std::size_t) b.node], b.inverse));

    mesh out;
    out.triangles.reserve(this->bound.triangles.size());
    for (std::size_t i = 0; i < this->bound.triangles.size(); i++) {
        const corner &c = this->bound.triangles[i];
        const influence &inf = this->influences[i];
        matrix m{};
        for (std::size_t b = 0; b < 4; b++) {
            const float w = inf.weights[b];
            if (w == 0)
                continue;
            const matrix &s = skinning[inf.bones[b]];
            for (std::size_t e = 0; e < 16; e++)
                m[e] += w * s[e];
        }
        const float x = m[0] * c.x + m[4] * c.y + m[8] * c.z + m[12];
        const float y = m[1] * c.x + m[5] * c.y + m[9] * c.z + m[13];
        const float z = m[2] * c.x + m[6] * c.y + m[10] * c.z + m[14];
        float nx = m[0] * c.nx + m[4] * c.ny + m[8] * c.nz;
        float ny = m[1] * c.nx + m[5] * c.ny + m[9] * c.nz;
        float nz = m[2] * c.nx + m[6] * c.ny + m[10] * c.nz;
        const float len = std::hypot(nx, ny, nz);
        if (len > 0) {
            nx /= len;
            ny /= len;
            nz /= len;
        }
        // glTF: x right, y up, z to the front; the board: x east, y south, z up
        out.triangles.push_back({x, z, y, nx, nz, ny, c.u, c.v, c.colour});
    }
    return out;
}

} // namespace goe::models
