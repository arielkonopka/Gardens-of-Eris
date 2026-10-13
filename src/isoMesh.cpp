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

#include "isoMesh.h"
#include <cmath>

namespace goe::iso {

shape defaultShape(int type)
{
    switch (type) {
    case bElemTypes::_floorType:
        return shape::ground;
    case bElemTypes::_wallType:
    case bElemTypes::_brickClusterType:
    case bElemTypes::_door:
        return shape::block;
    case bElemTypes::_landmineType:
        return shape::flat;
    default:
        return shape::standee;
    }
}

std::optional<shape> parseShape(std::string_view name)
{
    if (name == "ground")
        return shape::ground;
    if (name == "block")
        return shape::block;
    if (name == "standee")
        return shape::standee;
    if (name == "flat")
        return shape::flat;
    return std::nullopt;
}

meshBuilder::meshBuilder(float towardsEyeX, float towardsEyeY)
{
    const float len = std::hypot(towardsEyeX, towardsEyeY);
    this->eyeX = len > 0 ? towardsEyeX / len : 0.0f;
    this->eyeY = len > 0 ? towardsEyeY / len : 1.0f;
}

bool hides(float eyeX, float eyeY, float dx, float dy)
{
    const float len = std::hypot(eyeX, eyeY);
    if (len <= 0)
        return false;
    const float ex = eyeX / len, ey = eyeY / len;
    const float towards = dx * ex + dy * ey;
    return towards > 0 && towards <= ghostTowards && std::abs(dx * ey - dy * ex) <= ghostAcross;
}

void meshBuilder::add(shape s, float x, float y, const picture &p)
{
    switch (s) {
    case shape::ground:
        this->ground(x, y, p);
        break;
    case shape::block:
        this->block(x, y, p);
        break;
    case shape::standee:
        this->standee(x, y, p);
        break;
    case shape::flat:
        this->flat(x, y, p);
        break;
    }
}

void meshBuilder::quad(const vertex &a, const vertex &b, const vertex &c, const vertex &d)
{
    this->corners.insert(this->corners.end(), {a, b, c, a, c, d});
}

void meshBuilder::lying(float x, float y, float z, const picture &p, bool backdrop)
{
    const float u0 = (float) p.at.x, v0 = (float) p.at.y;
    const float u1 = u0 + (float) p.size.x, v1 = v0 + (float) p.size.y;
    this->quad({x, y, z, u0, v0, light::top, backdrop},
               {x + 1, y, z, u1, v0, light::top, backdrop},
               {x + 1, y + 1, z, u1, v1, light::top, backdrop},
               {x, y + 1, z, u0, v1, light::top, backdrop});
}

void meshBuilder::ground(float x, float y, const picture &p)
{
    this->lying(x, y, 0, p, true);
}

void meshBuilder::flat(float x, float y, const picture &p)
{
    this->lying(x, y, flatLift, p, true);
}

void meshBuilder::block(float x, float y, const picture &p, float height)
{
    const float u0 = (float) p.at.x, v0 = (float) p.at.y;
    const float u1 = u0 + (float) p.size.x, v1 = v0 + (float) p.size.y;
    const float h = height;
    this->lying(x, y, h, p, false);
    // the sides the camera can see, each with the picture upright
    if (this->eyeY > 0) // south
        this->quad({x, y + 1, h, u0, v0, light::southFace},
                   {x + 1, y + 1, h, u1, v0, light::southFace},
                   {x + 1, y + 1, 0, u1, v1, light::southFace},
                   {x, y + 1, 0, u0, v1, light::southFace});
    if (this->eyeY < 0) // north
        this->quad({x + 1, y, h, u0, v0, light::northFace},
                   {x, y, h, u1, v0, light::northFace},
                   {x, y, 0, u1, v1, light::northFace},
                   {x + 1, y, 0, u0, v1, light::northFace});
    if (this->eyeX > 0) // east
        this->quad({x + 1, y + 1, h, u0, v0, light::eastFace},
                   {x + 1, y, h, u1, v0, light::eastFace},
                   {x + 1, y, 0, u1, v1, light::eastFace},
                   {x + 1, y + 1, 0, u0, v1, light::eastFace});
    if (this->eyeX < 0) // west
        this->quad({x, y, h, u0, v0, light::westFace},
                   {x, y + 1, h, u1, v0, light::westFace},
                   {x, y + 1, 0, u1, v1, light::westFace},
                   {x, y, 0, u0, v1, light::westFace});
}

void meshBuilder::standee(float x, float y, const picture &p)
{
    const float u0 = (float) p.at.x, v0 = (float) p.at.y;
    const float u1 = u0 + (float) p.size.x, v1 = v0 + (float) p.size.y;
    // across the line of sight, standing in the middle of its cell
    const float half = standeeSize / 2;
    const float rx = this->eyeY * half, ry = -this->eyeX * half;
    const float cx = x + 0.5f, cy = y + 0.5f;
    const float h = standeeSize;
    this->quad({cx - rx, cy - ry, h, u0, v0, light::standee},
               {cx + rx, cy + ry, h, u1, v0, light::standee},
               {cx + rx, cy + ry, 0, u1, v1, light::standee},
               {cx - rx, cy - ry, 0, u0, v1, light::standee});
}

} // namespace goe::iso
