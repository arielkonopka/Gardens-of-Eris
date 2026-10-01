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
#include "gameSerializer.h"
#include "player.h"
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <map>
#include <mutex>

namespace {
std::atomic<std::size_t> generated = 0;

/// the fixed patterns: set by an agent between ticks, read when a chunk is made
struct patternTable
{
    std::mutex lock;
    std::map<std::pair<int, int>, std::shared_ptr<const goe::chunkPattern>> byChunk;
    std::shared_ptr<const goe::chunkPattern> fallback;
};

patternTable &patterns()
{
    static patternTable table;
    return table;
}

void buildChunk(const std::shared_ptr<chamber> &world, coords chunk, bool start)
{
    randomLevelGenerator(world, chunk).generateChunk(start, worldBuilder::patternFor(chunk));
    generated++;
}
}

std::shared_ptr<chamber> worldBuilder::startNew()
{
    auto world = chamber::makeWorld();
    buildChunk(world, coords(0, 0), true);
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
                if (!world->isSwapped(chunk) || !gameSerializer::swapInChunk(world, chunk))
                    buildChunk(world, chunk, false);
                return true;
            }
    return false;
}

bool worldBuilder::shrinkAround(const std::shared_ptr<chamber> &world, coords cell)
{
    if (!world || world->isBounded() || cell == NOCOORDS)
        return false;
    const coords centre = chamber::chunkOf(cell);
    std::vector<std::pair<int, coords>> far;
    for (const coords &chunk : world->chunkKeys()) {
        const int ring = std::max(std::abs(chunk.x - centre.x), std::abs(chunk.y - centre.y));
        if (ring > worldBuilder::keepRadius)
            far.emplace_back(ring, chunk);
    }
    // the furthest first; a chunk where an avatar stands is refused, so try the next one
    std::sort(far.begin(), far.end(), [](const auto &a, const auto &b) { return a.first > b.first; });
    for (const auto &[ring, chunk] : far)
        if (gameSerializer::swapOutChunk(world, chunk))
            return true;
    return false;
}

void worldBuilder::bringIn(const std::shared_ptr<chamber> &world, coords cell)
{
    if (!world || cell == NOCOORDS)
        return;
    const coords chunk = chamber::chunkOf(cell);
    if (world->isSwapped(chunk))
        gameSerializer::swapInChunk(world, chunk);
}

bool worldBuilder::movePlayerTo(const std::shared_ptr<chamber> &world, coords chunk)
{
    const auto plr = player::getActivePlayer();
    if (!world || !plr)
        return false;
    const coords first = chamber::chunkOrigin(chunk);
    const coords middle = first + chamber::chunkSize / 2;
    while (worldBuilder::growAround(world, middle))
        ;
    std::shared_ptr<bElem> best;
    for (int x = first.x; x < first.x + chamber::chunkSize; x++)
        for (int y = first.y; y < first.y + chamber::chunkSize; y++) {
            const auto e = world->getElement(coords(x, y));
            if (!e || e->getType() != bElemTypes::_floorType || !e->getAttrs()->isSteppable())
                continue;
            if (!best || coords(x, y).distance(middle) < best->getStats()->getMyPosition().distance(middle))
                best = e;
        }
    return best && plr->stepOnElement(best);
}

std::size_t worldBuilder::chunksGenerated()
{
    return generated;
}

void worldBuilder::setPattern(coords chunk, std::shared_ptr<const goe::chunkPattern> pattern)
{
    auto &t = patterns();
    std::lock_guard guard(t.lock);
    if (pattern)
        t.byChunk[{chunk.x, chunk.y}] = std::move(pattern);
    else
        t.byChunk.erase({chunk.x, chunk.y});
}

void worldBuilder::setDefaultPattern(std::shared_ptr<const goe::chunkPattern> pattern)
{
    auto &t = patterns();
    std::lock_guard guard(t.lock);
    t.fallback = std::move(pattern);
}

void worldBuilder::clearPatterns()
{
    auto &t = patterns();
    std::lock_guard guard(t.lock);
    t.byChunk.clear();
    t.fallback = nullptr;
}

std::shared_ptr<const goe::chunkPattern> worldBuilder::patternFor(coords chunk)
{
    auto &t = patterns();
    std::lock_guard guard(t.lock);
    const auto it = t.byChunk.find({chunk.x, chunk.y});
    return it != t.byChunk.end() ? it->second : t.fallback;
}
