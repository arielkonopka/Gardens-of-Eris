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

#include "lineOfSight.h"
#include "bElem.h"
#include "chamber.h"
#include <cstdlib>

bool goe::sight::opaque(const std::shared_ptr<bElem> &e)
{
    if (!e)
        return true;
    switch (e->getType()) {
    case bElemTypes::_wallType:
    case bElemTypes::_brickClusterType:
    case bElemTypes::_bunker:
    case bElemTypes::_teleporter:
        return true;
    case bElemTypes::_door:
        return !e->getAttrs()->isSteppable();
    default:
        return false;
    }
}

bool goe::sight::clear(const std::shared_ptr<chamber> &board, coords from, coords to)
{
    if (!board)
        return false;
    int dx = std::abs(to.x - from.x), dy = -std::abs(to.y - from.y);
    int sx = from.x < to.x ? 1 : -1, sy = from.y < to.y ? 1 : -1;
    int err = dx + dy;
    coords c = from;
    while (!(c == to)) {
        int e2 = 2 * err;
        bool stepX = e2 >= dy, stepY = e2 <= dx;
        // a diagonal step passes between the two cells beside it: both closed means no gap
        if (stepX && stepY && opaque(board->getElement(coords(c.x + sx, c.y)))
            && opaque(board->getElement(coords(c.x, c.y + sy))))
            return false;
        if (stepX) {
            err += dy;
            c.x += sx;
        }
        if (stepY) {
            err += dx;
            c.y += sy;
        }
        if (!(c == to) && opaque(board->getElement(c)))
            return false;
    }
    return true;
}
