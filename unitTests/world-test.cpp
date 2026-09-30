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
#include <filesystem>
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

namespace {
/// the fog of war of every cell of a chunk
std::vector<int> fog(const std::shared_ptr<chamber> &world, coords chunk)
{
    std::vector<int> res;
    const coords first = chamber::chunkOrigin(chunk);
    for (int x = 0; x < chamber::chunkSize; x++)
        for (int y = 0; y < chamber::chunkSize; y++)
            res.push_back(world->isVisible(first + coords(x, y)));
    return res;
}

/// a cell of the chunk whose top is plain floor
coords floorIn(const std::shared_ptr<chamber> &world, coords chunk, int skip = 0)
{
    const coords first = chamber::chunkOrigin(chunk);
    for (int x = 1; x < chamber::chunkSize - 1; x++)
        for (int y = 1; y < chamber::chunkSize - 1; y++) {
            auto e = world->getElement(first + coords(x, y));
            if (e && e->getType() == bElemTypes::_floorType && !e->getStats()->getSteppingOn() && skip-- <= 0)
                return first + coords(x, y);
        }
    return NOCOORDS;
}

std::size_t liveIn(const std::shared_ptr<chamber> &world, coords chunk)
{
    return (std::size_t) std::ranges::count_if(world->liveElems, [chunk](const std::shared_ptr<bElem> &e) {
        const coords at = e->getStats()->getMyPosition();
        return at != NOCOORDS && chamber::chunkOf(at) == chunk;
    });
}

/// walks the player's view far away and back: grow and shrink until there is nothing left to do
void settleAround(const std::shared_ptr<chamber> &world, coords cell)
{
    while (worldBuilder::growAround(world, cell) || worldBuilder::shrinkAround(world, cell))
        ;
}
} // namespace

TEST(WorldTests, ASwappedChunkComesBackAsItWasLeft)
{
    auto world = newWorld(2323);
    const coords chunk(2, 1);
    // something the generator would not have put there
    const coords cell = floorIn(world, chunk);
    ASSERT_FALSE(cell == NOCOORDS);
    auto k = elementFactory::generateAnElement<key>(world, 3);
    ASSERT_TRUE(k->stepOnElement(world->getElement(cell)));
    const auto keyId = k->getStats()->getInstanceId();
    k.reset();
    world->setVisible(cell, 0);
    const auto cells = layout(world, chunk);
    const auto seen = fog(world, chunk);
    const auto live = liveIn(world, chunk);
    const auto liveTotal = world->liveElems.size();
    const auto apples = goldenApple::getAppleNumber();
    const auto chunksBefore = world->chunkKeys().size();

    ASSERT_TRUE(gameSerializer::swapOutChunk(world, chunk));
    EXPECT_FALSE(world->hasChunk(chunk));
    EXPECT_TRUE(world->isSwapped(chunk));
    EXPECT_EQ(world->swappedCount(), 1u);
    EXPECT_EQ(world->chunkKeys().size(), chunksBefore - 1);
    EXPECT_FALSE(world->getElement(cell));
    EXPECT_EQ(liveIn(world, chunk), 0u);
    EXPECT_EQ(world->liveElems.size(), liveTotal - live);
    // its apples still count
    EXPECT_EQ(goldenApple::getAppleNumber(), apples);
    // a chunk can go to disk only once
    EXPECT_FALSE(gameSerializer::swapOutChunk(world, chunk));

    ASSERT_TRUE(gameSerializer::swapInChunk(world, chunk));
    EXPECT_TRUE(world->hasChunk(chunk));
    EXPECT_FALSE(world->isSwapped(chunk));
    EXPECT_TRUE(layout(world, chunk) == cells);
    EXPECT_TRUE(fog(world, chunk) == seen);
    EXPECT_EQ(liveIn(world, chunk), live);
    EXPECT_EQ(world->liveElems.size(), liveTotal);
    EXPECT_EQ(goldenApple::getAppleNumber(), apples);
    auto back = world->getElement(cell);
    ASSERT_TRUE(back);
    EXPECT_EQ(back->getType(), bElemTypes::_key);
    EXPECT_EQ(back->getStats()->getInstanceId(), keyId);
    // every element knows where it stands again
    const coords first = chamber::chunkOrigin(chunk);
    for (int x = 0; x < chamber::chunkSize; x++)
        for (int y = 0; y < chamber::chunkSize; y++)
            for (auto e = world->getElement(first + coords(x, y)); e; e = e->getStats()->getSteppingOn()) {
                EXPECT_TRUE(e->getStats()->getMyPosition() == first + coords(x, y));
                EXPECT_TRUE(e->getBoard() == world);
                EXPECT_FALSE(e->getStats()->isDisposed());
            }
}

TEST(WorldTests, TheChunkWhereThePlayerStandsStays)
{
    auto world = newWorld();
    const coords here = chamber::chunkOf(player::getActivePlayer()->getStats()->getMyPosition());
    EXPECT_FALSE(gameSerializer::swapOutChunk(world, here));
    EXPECT_TRUE(world->hasChunk(here));
    // walking far away does not drop it either
    settleAround(world, chamber::chunkOrigin(here + coords(12, 0)));
    EXPECT_TRUE(world->hasChunk(here));
}

TEST(WorldTests, WalkingFarKeepsTheChunksInMemoryBounded)
{
    auto world = newWorld(5555);
    const coords start = world->origin;
    const coords marked = floorIn(world, coords(1, 1));
    auto k = elementFactory::generateAnElement<key>(world, 2);
    ASSERT_TRUE(k->stepOnElement(world->getElement(marked)));
    k.reset();
    const auto cells = layout(world, coords(1, 1));
    const std::size_t window = (2 * worldBuilder::keepRadius + 1) * (2 * worldBuilder::keepRadius + 1);
    std::size_t most = 0;
    for (int step = 1; step <= 15; step++) {
        settleAround(world, start + coords(step * chamber::chunkSize, 0));
        most = std::max(most, world->chunkKeys().size());
    }
    // the window around the view, and the chunk the player stands in
    EXPECT_LE(most, window + 1);
    EXPECT_GT(world->swappedCount(), 0u);
    EXPECT_GT(teleport::parkedCount(), 0u);

    // coming back brings the old chunks back as they were, not new ones
    settleAround(world, start);
    EXPECT_TRUE(layout(world, coords(1, 1)) == cells);
    EXPECT_EQ(world->getElement(marked)->getType(), bElemTypes::_key);
    EXPECT_LE(world->chunkKeys().size(), window + 1);
}

TEST(WorldTests, ATeleporterLinkSurvivesTheOtherEndGoingToDisk)
{
    auto world = newWorld();
    const int subtype = 555; // no generated teleporter has it
    const coords a = floorIn(world, coords(0, 0), 7), b = floorIn(world, coords(2, 2));
    auto here = elementFactory::generateAnElement<teleport>(world, subtype);
    auto there = elementFactory::generateAnElement<teleport>(world, subtype);
    ASSERT_TRUE(here->stepOnElement(world->getElement(a)));
    ASSERT_TRUE(there->stepOnElement(world->getElement(b)));
    there.reset();
    // something to send through, standing next to the teleporter
    auto box = elementFactory::generateAnElement<rubbish>(world, 0);
    ASSERT_TRUE(box->getAttrs()->isMovable());
    const coords boxAt = floorIn(world, coords(0, 0), 20);
    ASSERT_TRUE(box->stepOnElement(world->getElement(boxAt)));

    ASSERT_TRUE(here->interact(box));
    EXPECT_TRUE(chamber::chunkOf(box->getStats()->getMyPosition()) == coords(2, 2));
    // the other end's chunk goes to disk; the link brings it back
    ASSERT_TRUE(gameSerializer::swapOutChunk(world, coords(2, 2)));
    ASSERT_TRUE(box->getStats()->isDisposed()); // it stood there, so it went with the chunk
    auto box2 = elementFactory::generateAnElement<rubbish>(world, 0);
    ASSERT_TRUE(box2->stepOnElement(world->getElement(floorIn(world, coords(0, 0), 30))));
    // let both ends and the interaction cool down
    for (int t = 0; t < 555; t++)
        bElem::tick();
    ASSERT_TRUE(here->interact(box2));
    EXPECT_TRUE(world->hasChunk(coords(2, 2)));
    EXPECT_TRUE(chamber::chunkOf(box2->getStats()->getMyPosition()) == coords(2, 2));
}

TEST(WorldTests, AParkedTeleporterCanStillBeTheOtherEnd)
{
    auto world = newWorld();
    const int subtype = 556;
    auto here = elementFactory::generateAnElement<teleport>(world, subtype);
    auto there = elementFactory::generateAnElement<teleport>(world, subtype);
    ASSERT_TRUE(here->stepOnElement(world->getElement(floorIn(world, coords(0, 0), 7))));
    ASSERT_TRUE(there->stepOnElement(world->getElement(floorIn(world, coords(-2, 2)))));
    there.reset();
    const auto parked = teleport::parkedCount();
    ASSERT_TRUE(gameSerializer::swapOutChunk(world, coords(-2, 2)));
    EXPECT_GT(teleport::parkedCount(), parked);
    auto box = elementFactory::generateAnElement<rubbish>(world, 0);
    ASSERT_TRUE(box->stepOnElement(world->getElement(floorIn(world, coords(0, 0), 20))));
    ASSERT_TRUE(here->interact(box));
    EXPECT_TRUE(world->hasChunk(coords(-2, 2)));
    EXPECT_TRUE(chamber::chunkOf(box->getStats()->getMyPosition()) == coords(-2, 2));
}

TEST(WorldTests, SwappedChunksAreKeptInASave)
{
    auto world = newWorld(777);
    const coords chunk(-2, -1);
    const auto cells = layout(world, chunk);
    const auto apples = goldenApple::getAppleNumber();
    ASSERT_TRUE(gameSerializer::swapOutChunk(world, chunk));
    const auto parked = teleport::parkedCount();
    const std::string file = (std::filesystem::temp_directory_path() / "goe-swapped-chunks.goe").string();
    ASSERT_TRUE(gameSerializer::saveGame(file));
    world.reset();
    ASSERT_TRUE(gameSerializer::loadGame(file));
    std::filesystem::remove(file);
    ASSERT_EQ(chamber::allChambers.size(), 1u);
    auto loaded = chamber::allChambers.front();
    EXPECT_FALSE(loaded->hasChunk(chunk));
    EXPECT_TRUE(loaded->isSwapped(chunk));
    EXPECT_EQ(teleport::parkedCount(), parked);
    EXPECT_EQ(goldenApple::getAppleNumber(), apples);
    ASSERT_TRUE(gameSerializer::swapInChunk(loaded, chunk));
    EXPECT_TRUE(layout(loaded, chunk) == cells);
    EXPECT_EQ(goldenApple::getAppleNumber(), apples);
}
