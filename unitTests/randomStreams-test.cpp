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
#include "testSupport.h"
#include "elements.h"
#include "inputManager.h"
#include "randomLevelGenerator.h"
#include "randomStreams.h"
#include <thread>

namespace {
/// what a level looks like, cell by cell: the top element's type, subtype and energy
struct levelPrint
{
    std::string name;
    int r, g, b;
    std::vector<int> cells;
    bool operator==(const levelPrint &) const = default;
};

levelPrint buildLevel(goe::rng::seed levelSeed, int size = 64)
{
    inputManager::getInstance(true);
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    randomLevelGenerator gen(size, size, levelSeed);
    REQUIRE_IN_HELPER(gen.generateLevel(5));
    auto mc = gen.mychamber;
    levelPrint p{mc->getName(), mc->getChColour().r, mc->getChColour().g, mc->getChColour().b, {}};
    for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++) {
            auto e = mc->getElement(x, y);
            REQUIRE_IN_HELPER(e);
            p.cells.push_back(e->getType());
            // a local teleporter's subtype is its chamber's id, which differs from one chamber to the next
            p.cells.push_back(e->getType() == bElemTypes::_teleporter ? 0 : e->getAttrs()->getSubtype());
            p.cells.push_back(e->getAttrs()->getEnergy());
        }
    return p;
}
} // namespace

TEST(RandomStreams, SameSeedBuildsTheSameLevel)
{
    EXPECT_TRUE(buildLevel(5555) == buildLevel(5555));
}

TEST(RandomStreams, DifferentSeedsBuildDifferentLevels)
{
    EXPECT_FALSE(buildLevel(5555) == buildLevel(23));
}

TEST(RandomStreams, BuildingALevelLeavesGameplayRandomnessAlone)
{
    goe::rng::engine before = goe::rng::saved();
    buildLevel(5);
    EXPECT_TRUE(before == goe::rng::saved());
}

TEST(RandomStreams, BuildingALevelOnAnotherThreadLeavesGameplayRandomnessAlone)
{
    goe::rng::engine before = goe::rng::saved();
    levelPrint there;
    std::thread builder([&there]() { there = buildLevel(55); });
    builder.join();
    EXPECT_TRUE(before == goe::rng::saved());
    EXPECT_TRUE(there == buildLevel(55));
}

TEST(RandomStreams, AudioAndEffectsLeaveGameplayRandomnessAlone)
{
    goe::rng::engine before = goe::rng::saved();
    for (int c = 0; c < 5; c++) {
        goe::rng::audio();
        goe::rng::cosmetic()();
    }
    EXPECT_TRUE(before == goe::rng::saved());
}

TEST(RandomStreams, GameplayDrawsFromTheInnermostScope)
{
    EXPECT_EQ(&goe::rng::gameplay(), &goe::rng::saved());
    goe::rng::engine outer(1), inner(2);
    {
        goe::rng::generationScope a(outer);
        EXPECT_EQ(&goe::rng::gameplay(), &outer);
        {
            goe::rng::generationScope b(inner);
            EXPECT_EQ(&goe::rng::gameplay(), &inner);
        }
        EXPECT_EQ(&goe::rng::gameplay(), &outer);
    }
    EXPECT_EQ(&goe::rng::gameplay(), &goe::rng::saved());
}

TEST(RandomStreams, WorldSeedGivesEachLevelItsOwnRepeatableSeed)
{
    goe::rng::setWorldSeed(5);
    auto first = goe::rng::nextLevelSeed();
    auto second = goe::rng::nextLevelSeed();
    EXPECT_NE(first, second);
    goe::rng::setWorldSeed(5);
    EXPECT_EQ(first, goe::rng::nextLevelSeed());
    EXPECT_EQ(second, goe::rng::nextLevelSeed());
    goe::rng::setWorldSeed(6);
    EXPECT_NE(first, goe::rng::nextLevelSeed());
}
