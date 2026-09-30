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
#include "worldBuilder.h"
#include "chamber.h"
#include "randomLevelGenerator.h"
#include <cstdlib>

std::shared_ptr<chamber> worldBuilder::startNew()
{
    auto world = chamber::makeWorld();
    randomLevelGenerator(world, coords(0, 0)).generateChunk(true);
    while (worldBuilder::growAround(world, world->origin))
        ;
    return world;
}

bool worldBuilder::growAround(const std::shared_ptr<chamber> &world, coords cell)
{
    if (!world || world->isBounded() || cell == NOCOORDS)
        return false;
    const coords centre = chamber::chunkOf(cell);
    // ring by ring, so the chunks nearest to the player come first
    for (int ring = 0; ring <= worldBuilder::buildRadius; ring++)
        for (int dx = -ring; dx <= ring; dx++)
            for (int dy = -ring; dy <= ring; dy++) {
                if (std::max(std::abs(dx), std::abs(dy)) != ring)
                    continue;
                const coords chunk = centre + coords(dx, dy);
                if (world->hasChunk(chunk))
                    continue;
                randomLevelGenerator(world, chunk).generateChunk(false);
                return true;
            }
    return false;
}
