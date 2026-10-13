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

#include "fieldCamera.h"
#include <algorithm>

fieldCamera::fieldCamera(coords tileSize)
    : tileSize(tileSize)
{}

void fieldCamera::follow(coords centre, coords tilesOnScreen, std::optional<coords> boardSize)
{
    coords wanted = centre - coords(tilesOnScreen.x / 2, tilesOnScreen.y / 2);
    if (boardSize) {
        wanted.x = std::max(0, std::min(boardSize->x - tilesOnScreen.x, wanted.x));
        wanted.y = std::max(0, std::min(boardSize->y - tilesOnScreen.y, wanted.y));
    }
    coords d = wanted - this->cell;
    // rounded down, so the view scrolls the same way where the endless world goes negative
    if (d.x == 0 && floorMod(this->pixels.x, this->tileSize.x) > 0)
        d.x = -1;
    if (d.y == 0 && floorMod(this->pixels.y, this->tileSize.y) > 0)
        d.y = -1;
    this->pixels = this->pixels + (d * step);
    this->cell = coords(floorDiv(this->pixels.x, this->tileSize.x), floorDiv(this->pixels.y, this->tileSize.y));
}

coords fieldCamera::scroll() const
{
    return coords(floorMod(this->pixels.x, this->tileSize.x), floorMod(this->pixels.y, this->tileSize.y));
}
