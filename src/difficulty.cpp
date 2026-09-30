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

#include "difficulty.h"
#include "bElem.h"
#include "chamber.h"
#include "player.h"
#include <cstdlib>

int difficulty::playerLevel(int shots)
{
    long long next = 5;
    int level = 0;
    while (shots + 1LL >= next) {
        level++;
        next *= 5;
    }
    return level;
}

int difficulty::playerLevel(const std::shared_ptr<bElem> &who)
{
    return who ? difficulty::playerLevel(who->getStats()->getPoints(SHOOT)) : 0;
}

int difficulty::distanceLevel(coords from, coords to)
{
    if (from == NOCOORDS || to == NOCOORDS)
        return 0;
    long long steps = 1LL + std::max(std::abs(to.x - from.x), std::abs(to.y - from.y)) / distanceUnit;
    int level = 0;
    while (steps > 1) {
        steps >>= 1;
        level++;
    }
    return level;
}

coords difficulty::areaOf(coords cell)
{
    // rounded down, so negative cells of the endless world land in the right area too
    return coords(floorDiv(cell.x, distanceUnit), floorDiv(cell.y, distanceUnit));
}

int difficulty::chunkDepth(coords chunk)
{
    const int level = difficulty::distanceLevel(chamber::chunkOrigin(coords(0, 0)), chamber::chunkOrigin(chunk));
    return std::clamp(level, 0, difficulty::five - 1);
}

int difficulty::of(const std::shared_ptr<bElem> &who)
{
    if (!who)
        return 0;
    int d = difficulty::playerLevel(who);
    if (auto board = who->getBoard())
        d += difficulty::distanceLevel(board->origin, who->getStats()->getMyPosition());
    return d;
}

int difficulty::current()
{
    return difficulty::of(player::getActivePlayer());
}
