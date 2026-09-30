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
#include "elements.h"
#include "chamber.h"
#include "difficulty.h"
#include "gameSerializer.h"
#include "randomLevelGenerator.h"
#include "randomStreams.h"
#include "worldBuilder.h"
#include <gtest/gtest.h>
#include <deque>
#include <set>

namespace {
/// a fresh endless world from a fixed seed
std::shared_ptr<chamber> newWorld(goe::rng::seed seed = 555)
{
    inputManager::getInstance(true);
    gameSerializer::clearWorld();
    goe::rng::setWorldSeed(seed);
    return worldBuilder::startNew();
}

bool isWall(const std::shared_ptr<chamber> &world, coords cell)
{
    auto e = world->getElement(cell);
    return e && e->getType() == bElemTypes::_wallType;
}

/// what every cell of a chunk holds, bottom to top, by type
std::vector<std::vector<int>> layout(const std::shared_ptr<chamber> &world, coords chunk)
{
    std::vector<std::vector<int>> res;
    const coords first = chamber::chunkOrigin(chunk);
    for (int x = 0; x < chamber::chunkSize; x++)
        for (int y = 0; y < chamber::chunkSize; y++) {
            std::vector<int> stack;
            for (auto e = world->getElement(first + coords(x, y)); e; e = e->getStats()->getSteppingOn())
                stack.push_back(e->getType());
            res.push_back(stack);
        }
    return res;
}
} // namespace

TEST(WorldTests, ChunksAreFoundByFloorDivision)
{
    EXPECT_TRUE(chamber::chunkOf(coords(0, 0)) == coords(0, 0));
    EXPECT_TRUE(chamber::chunkOf(coords(63, 64)) == coords(0, 1));
    EXPECT_TRUE(chamber::chunkOf(coords(-1, -64)) == coords(-1, -1));
    EXPECT_TRUE(chamber::chunkOf(coords(-65, 5)) == coords(-2, 0));
    EXPECT_TRUE(chamber::chunkOrigin(coords(-2, 3)) == coords(-128, 192));
    EXPECT_EQ(floorDiv(-1, 64), -1);
    EXPECT_EQ(floorMod(-1, 64), 63);
    EXPECT_EQ(floorMod(130, 64), 2);
}

TEST(WorldTests, CellsExistOnlyInBuiltChunks)
{
    inputManager::getInstance(true);
    auto world = chamber::makeWorld();
    EXPECT_FALSE(world->isBounded());
    EXPECT_FALSE(world->getElement(-5, -5));
    world->addChunk(coords(-1, -1));
    ASSERT_TRUE(world->getElement(-5, -5));
    EXPECT_TRUE(world->getElement(-5, -5)->getStats()->getMyPosition() == coords(-5, -5));
    EXPECT_FALSE(world->getElement(0, 0));
    // a neighbour in an unbuilt chunk is nothing, and nothing steps there
    auto edge = world->getElement(-1, -1);
    EXPECT_FALSE(edge->getElementInDirection(dir::direction::RIGHT));
    EXPECT_FALSE(edge->isSteppableDirection(dir::direction::RIGHT));
    EXPECT_TRUE(edge->isSteppableDirection(dir::direction::LEFT));
    world->setVisible(coords(-5, -5), 7);
    EXPECT_EQ(world->isVisible(coords(-5, -5)), 7);
    EXPECT_FALSE(world->isVisible(coords(5, 5)));
}

TEST(WorldTests, NewGameIsOneBoardAroundThePlayer)
{
    auto world = newWorld();
    // no other chambers: the whole game is this one board
    ASSERT_EQ(chamber::allChambers.size(), 1u);
    EXPECT_TRUE(chamber::allChambers.front() == world);
    auto plr = player::getActivePlayer();
    ASSERT_TRUE(plr);
    EXPECT_TRUE(plr->getBoard() == world);
    EXPECT_TRUE(chamber::chunkOf(plr->getStats()->getMyPosition()) == coords(0, 0));
    EXPECT_FALSE(world->origin == NOCOORDS);
    const int r = worldBuilder::buildRadius;
    EXPECT_EQ(world->chunkKeys().size(), (std::size_t) ((2 * r + 1) * (2 * r + 1)));
    for (int x = -r; x <= r; x++)
        for (int y = -r; y <= r; y++)
            EXPECT_TRUE(world->hasChunk(coords(x, y)));
    EXPECT_FALSE(world->hasChunk(coords(r + 1, 0)));
}

TEST(WorldTests, WorldGrowsOneChunkAtATimeNearestFirst)
{
    auto world = newWorld();
    const coords far = chamber::chunkOrigin(coords(10, 0)) + coords(5, 5);
    const auto before = world->chunkKeys().size();
    ASSERT_TRUE(worldBuilder::growAround(world, far));
    ASSERT_EQ(world->chunkKeys().size(), before + 1);
    EXPECT_TRUE(world->chunkKeys().back() == coords(10, 0));
    while (worldBuilder::growAround(world, far))
        ;
    EXPECT_EQ(world->chunkKeys().size(), before + 25);
    // a bounded board never grows
    EXPECT_FALSE(worldBuilder::growAround(chamber::makeNewChamber(coords(10, 10)), coords(5, 5)));
}

TEST(WorldTests, ChunkWallsHaveMatchingGaps)
{
    auto world = newWorld();
    const int n = chamber::chunkSize;
    for (const coords chunk : world->chunkKeys()) {
        const coords first = chamber::chunkOrigin(chunk);
        const auto west = randomLevelGenerator::wallGaps(chunk, true);
        const auto north = randomLevelGenerator::wallGaps(chunk, false);
        ASSERT_FALSE(west.empty());
        ASSERT_FALSE(north.empty());
        for (int o = 1; o < n - 1; o++) {
            const bool westGap = std::find(west.begin(), west.end(), o) != west.end();
            const bool northGap = std::find(north.begin(), north.end(), o) != north.end();
            EXPECT_EQ(isWall(world, first + coords(0, o)), !westGap);
            EXPECT_EQ(isWall(world, first + coords(o, 0)), !northGap);
            // a gap leads somewhere on both sides, whichever chunk was built first
            if (westGap && world->hasChunk(chunk - coords(1, 0))) {
                EXPECT_FALSE(isWall(world, first + coords(1, o)));
                EXPECT_FALSE(isWall(world, first + coords(-1, o)));
            }
            if (northGap && world->hasChunk(chunk - coords(0, 1))) {
                EXPECT_FALSE(isWall(world, first + coords(o, 1)));
                EXPECT_FALSE(isWall(world, first + coords(o, -1)));
            }
        }
    }
}

TEST(WorldTests, EveryBuiltChunkCanBeReachedWithoutWalkingThroughWalls)
{
    auto world = newWorld(2305);
    std::set<std::pair<int, int>> seen;
    std::set<std::pair<int, int>> chunksReached;
    std::deque<coords> queue{world->origin};
    seen.insert({world->origin.x, world->origin.y});
    while (!queue.empty()) {
        const coords c = queue.front();
        queue.pop_front();
        const coords ch = chamber::chunkOf(c);
        chunksReached.insert({ch.x, ch.y});
        for (auto d : dir::allDirections) {
            const coords n = c + dir::dirToCoords(d);
            if (!world->getElement(n) || isWall(world, n) || !seen.insert({n.x, n.y}).second)
                continue;
            queue.push_back(n);
        }
    }
    EXPECT_EQ(chunksReached.size(), world->chunkKeys().size());
}

TEST(WorldTests, AChunkIsTheSameWhateverOrderItIsBuiltIn)
{
    const coords chunk(7, -3);
    auto first = newWorld(4242);
    randomLevelGenerator(first, chunk).generateChunk(false);
    const auto a = layout(first, chunk);

    auto second = newWorld(4242);
    randomLevelGenerator(second, chunk + coords(1, 0)).generateChunk(false);
    randomLevelGenerator(second, chunk - coords(0, 1)).generateChunk(false);
    randomLevelGenerator(second, chunk).generateChunk(false);
    EXPECT_TRUE(layout(second, chunk) == a);

    auto other = newWorld(4243);
    randomLevelGenerator(other, chunk).generateChunk(false);
    EXPECT_FALSE(layout(other, chunk) == a);
}

TEST(WorldTests, TheMazeGetsHarderFurtherOut)
{
    EXPECT_EQ(difficulty::chunkDepth(coords(0, 0)), 0);
    EXPECT_EQ(difficulty::chunkDepth(coords(-1, 1)), 1);
    EXPECT_EQ(difficulty::chunkDepth(coords(0, -3)), 2);
    EXPECT_EQ(difficulty::chunkDepth(coords(7, 0)), 3);
    EXPECT_EQ(difficulty::chunkDepth(coords(15, 2)), 4);
    EXPECT_EQ(difficulty::chunkDepth(coords(-500, 20)), 4);
    EXPECT_EQ(difficulty::mazeHoles(0), 5);
    EXPECT_EQ(difficulty::mazeHoles(4), 1);
    // far out chunks get landmines, the start does not
    auto world = newWorld();
    randomLevelGenerator(world, coords(40, 0)).generateChunk(false);
    int mines = 0;
    const coords first = chamber::chunkOrigin(coords(40, 0));
    for (int x = 0; x < chamber::chunkSize; x++)
        for (int y = 0; y < chamber::chunkSize; y++)
            for (auto e = world->getElement(first + coords(x, y)); e; e = e->getStats()->getSteppingOn())
                mines += e->getType() == bElemTypes::_landmineType;
    EXPECT_GT(mines, 0);
}

TEST(WorldTests, FarAwayElementsWaitForThePlayer)
{
    auto world = newWorld();
    const coords here = world->origin;
    EXPECT_TRUE(world->isActiveNear(here + coords(2 * chamber::chunkSize, 0), here));
    EXPECT_FALSE(world->isActiveNear(here + coords((chamber::activeChunks + 1) * chamber::chunkSize, 0), here));
    EXPECT_TRUE(world->isActiveNear(NOCOORDS, here)); // collected elements always run
    EXPECT_TRUE(chamber::makeNewChamber(coords(5, 5))->isActiveNear(coords(4000, 0), coords(0, 0)));
}

TEST(WorldTests, LocalTeleportersPairWithinTheirRegion)
{
    auto world = newWorld();
    std::set<int> subtypes;
    for (const coords chunk : world->chunkKeys()) {
        const coords first = chamber::chunkOrigin(chunk);
        for (int x = 0; x < chamber::chunkSize; x++)
            for (int y = 0; y < chamber::chunkSize; y++)
                for (auto e = world->getElement(first + coords(x, y)); e; e = e->getStats()->getSteppingOn())
                    if (e->getType() == bElemTypes::_teleporter && e->getAttrs()->getSubtype() != 0)
                        subtypes.insert(e->getAttrs()->getSubtype());
    }
    // the 5 x 5 chunks around the start touch four regions of 5 x 5 chunks
    EXPECT_GE(subtypes.size(), 1u);
    EXPECT_LE(subtypes.size(), 4u);
    for (int s : subtypes)
        EXPECT_GT(s, 0);
}
