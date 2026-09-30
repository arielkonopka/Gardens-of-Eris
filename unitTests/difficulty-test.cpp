#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "randomLevelGenerator.h"
#include "difficultyMusic.h"
#include <gtest/gtest.h>
#include <memory>

namespace {
/// an empty walled room with the active player at plrAt
std::shared_ptr<chamber> room(coords size, coords plrAt, std::shared_ptr<bElem> &plr)
{
    inputManager::getInstance(true);
    // earlier tests leave their player active, and only the active player's chamber ticks
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    auto mc = chamber::makeNewChamber(size);
    for (int x = 0; x < size.x; x++)
        for (auto p : {coords(x, 0), coords(x, size.y - 1)})
            elementFactory::generateAnElement<wall>(mc, 0)->stepOnElement(mc->getElement(p));
    for (int y = 0; y < size.y; y++)
        for (auto p : {coords(0, y), coords(size.x - 1, y)})
            elementFactory::generateAnElement<wall>(mc, 0)->stepOnElement(mc->getElement(p));
    plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(plrAt));
    plr->getStats()->setActive(true);
    return mc;
}

int countOfType(const std::shared_ptr<chamber> &mc, int type)
{
    int n = 0;
    for (int x = 0; x < mc->getSize().x; x++)
        for (int y = 0; y < mc->getSize().y; y++)
            for (auto e = mc->getElement(x, y); e; e = e->getStats()->getSteppingOn())
                n += e->getType() == type;
    return n;
}

std::vector<std::shared_ptr<patrollingDrone>> houndsIn(const std::shared_ptr<chamber> &mc)
{
    std::vector<std::shared_ptr<patrollingDrone>> res;
    for (int x = 0; x < mc->getSize().x; x++)
        for (int y = 0; y < mc->getSize().y; y++)
            if (auto d = std::dynamic_pointer_cast<patrollingDrone>(mc->getElement(x, y)))
                if (std::dynamic_pointer_cast<puppetMasterHound>(d->getBrainModule()))
                    res.push_back(d);
    return res;
}
} // namespace

TEST(DifficultyTests, PlayerLevelIsLogFiveOfShots)
{
    EXPECT_EQ(difficulty::playerLevel(0), 0);
    EXPECT_EQ(difficulty::playerLevel(3), 0);
    EXPECT_EQ(difficulty::playerLevel(4), 1);
    EXPECT_EQ(difficulty::playerLevel(23), 1);
    EXPECT_EQ(difficulty::playerLevel(24), 2);
    EXPECT_EQ(difficulty::playerLevel(124), 3);
    EXPECT_EQ(difficulty::playerLevel(623), 3);
    EXPECT_EQ(difficulty::playerLevel(624), 4);
}

TEST(DifficultyTests, DistanceDoublesPerStep)
{
    const int u = difficulty::distanceUnit;
    EXPECT_EQ(difficulty::distanceLevel(coords(5, 5), coords(5, 5)), 0);
    EXPECT_EQ(difficulty::distanceLevel(coords(0, 0), coords(u - 1, 0)), 0);
    EXPECT_EQ(difficulty::distanceLevel(coords(0, 0), coords(0, u)), 1);
    EXPECT_EQ(difficulty::distanceLevel(coords(0, 0), coords(3 * u - 1, 2)), 1);
    EXPECT_EQ(difficulty::distanceLevel(coords(0, 0), coords(3 * u, 0)), 2);
    EXPECT_EQ(difficulty::distanceLevel(coords(0, 0), coords(-7 * u, 0)), 3);
    EXPECT_EQ(difficulty::distanceLevel(NOCOORDS, coords(3 * u, 0)), 0);
}

TEST(DifficultyTests, AreasUseFloorDivision)
{
    const int u = difficulty::distanceUnit;
    EXPECT_TRUE(difficulty::areaOf(coords(0, 0)) == coords(0, 0));
    EXPECT_TRUE(difficulty::areaOf(coords(u - 1, u - 1)) == coords(0, 0));
    EXPECT_TRUE(difficulty::areaOf(coords(u, 2 * u + 3)) == coords(1, 2));
    EXPECT_TRUE(difficulty::areaOf(coords(-1, -u)) == coords(-1, -1));
    EXPECT_TRUE(difficulty::areaOf(coords(-u - 1, 0)) == coords(-2, 0));
}

TEST(DifficultyTests, RulesStartAtTheOldValuesAndNeverGetEasier)
{
    EXPECT_EQ(difficulty::bunkerRange(0), 10);
    EXPECT_EQ(difficulty::bunkerRest(100, 0), 100);
    EXPECT_EQ(difficulty::cameraSight(0), 8);
    EXPECT_EQ(difficulty::guardianCount(0), 2);
    EXPECT_EQ(difficulty::beamDamage(0), GoEConstants::_radioActivityPower);
    EXPECT_EQ(difficulty::landmineCopies(0), 0);
    EXPECT_EQ(difficulty::houndPatience(0), 0);
    EXPECT_EQ(difficulty::houndPatience(1), 230 * difficulty::ticksPerSecond);
    EXPECT_EQ(difficulty::bunkerRest(5, 3), 5); // a short rest never gets longer
    for (int d = 1; d < 40; d++) {
        EXPECT_GE(difficulty::bunkerRange(d), difficulty::bunkerRange(d - 1));
        EXPECT_LE(difficulty::bunkerRest(275, d), difficulty::bunkerRest(275, d - 1));
        EXPECT_GE(difficulty::bunkerRest(275, d), difficulty::twentyThree);
        EXPECT_GE(difficulty::cameraSight(d), difficulty::cameraSight(d - 1));
        EXPECT_GE(difficulty::guardianCount(d), difficulty::guardianCount(d - 1));
        EXPECT_GE(difficulty::beamDamage(d), difficulty::beamDamage(d - 1));
        EXPECT_GE(difficulty::landmineCopies(d), difficulty::landmineCopies(d - 1));
        EXPECT_GE(difficulty::houndPatience(d), 55 * difficulty::ticksPerSecond);
        if (d > 1) {
            EXPECT_LE(difficulty::houndPatience(d), difficulty::houndPatience(d - 1));
        }
    }
}

TEST(DifficultyTests, DifficultyAddsPlayerLevelAndDistance)
{
    std::shared_ptr<bElem> plr;
    auto mc = room(coords(150, 10), coords(3, 3), plr);
    EXPECT_EQ(difficulty::current(), 0);
    plr->getStats()->setPoints(SHOOT, 4);
    EXPECT_EQ(difficulty::current(), 1);
    mc->origin = coords(3, 3);
    plr->stepOnElement(mc->getElement(3 + difficulty::distanceUnit, 3));
    EXPECT_EQ(difficulty::current(), 2);
    EXPECT_EQ(difficulty::of(nullptr), 0);
}

TEST(DifficultyTests, LandmineGoesOffWhenSteppedOn)
{
    std::shared_ptr<bElem> plr;
    auto mc = room(coords(20, 10), coords(2, 2), plr);
    auto mine = elementFactory::generateAnElement<landmine>(mc, 0);
    mine->stepOnElement(mc->getElement(12, 5));
    EXPECT_TRUE(mine->getAttrs()->isSteppable());
    auto victim = elementFactory::generateAnElement<monster>(mc, 0);
    victim->stepOnElement(mc->getElement(11, 5));
    for (int c = 0; c < 20; c++)
        bElem::runLiveElements();
    EXPECT_FALSE(mine->getStats()->isDestroying() || mine->getStats()->isDisposed());

    victim->stepOnElement(mine);
    for (int c = 0; c < 100; c++)
        bElem::runLiveElements();
    EXPECT_TRUE(mine->getStats()->isDisposed());
    EXPECT_TRUE(victim->getStats()->isDisposed() || victim->getStats()->isDying());
    EXPECT_FALSE(plr->getStats()->isDisposed());
}

TEST(DifficultyTests, DeeperLevelsGetLandminesAndADifficultyOrigin)
{
    inputManager::getInstance(true);
    randomLevelGenerator easy(120, 120);
    ASSERT_TRUE(easy.generateLevel(5));
    EXPECT_EQ(countOfType(easy.mychamber, bElemTypes::_landmineType), 0);
    EXPECT_FALSE(easy.mychamber->origin == NOCOORDS);

    randomLevelGenerator hard(120, 120);
    ASSERT_TRUE(hard.generateLevel(1));
    EXPECT_GT(countOfType(hard.mychamber, bElemTypes::_landmineType), 0);
    EXPECT_FALSE(hard.mychamber->origin == NOCOORDS);
}

TEST(DifficultyTests, HoundComesForACampingPlayerAndGivesUpWhenTheyLeave)
{
    const int u = difficulty::distanceUnit;
    std::shared_ptr<bElem> plr;
    auto mc = room(coords(2 * u + 10, 40), coords(20, 20), plr);
    // difficulty 0 never sends a hound
    for (int c = 0; c < 2000; c++)
        bElem::runLiveElements();
    EXPECT_TRUE(houndsIn(mc).empty());

    // at player level 1 the player may stay in one area for houndPatience(1) ticks
    plr->getStats()->setPoints(SHOOT, 4);
    ASSERT_EQ(difficulty::current(), 1);
    plr->stepOnElement(mc->getElement(u + 20, 20)); // a new area starts the clock again
    for (int c = 0; c < difficulty::houndPatience(1) - 50; c++)
        bElem::runLiveElements();
    EXPECT_TRUE(houndsIn(mc).empty());
    for (int c = 0; c < 100 && houndsIn(mc).empty(); c++)
        bElem::runLiveElements();
    auto hounds = houndsIn(mc);
    ASSERT_EQ(hounds.size(), 1u);
    auto hound = hounds.front();
    EXPECT_EQ(hound->getAttrs()->getSubtype(), 1); // the red hound look in the skin

    // it hunts the player down and bites
    const int energy = plr->getAttrs()->getEnergy();
    for (int c = 0; c < 250; c++)
        bElem::runLiveElements();
    auto h = hound->getStats()->getMyPosition(), p = plr->getStats()->getMyPosition();
    EXPECT_EQ(std::abs(h.x - p.x) + std::abs(h.y - p.y), 1);
    EXPECT_TRUE(p == coords(u + 20, 20)) << "the hound must bite, not push the player away";
    EXPECT_LT(plr->getAttrs()->getEnergy(), energy);

    // the player moves to another area, and the hound gives up
    plr->stepOnElement(mc->getElement(20, 20));
    for (int c = 0; c < 200; c++)
        bElem::runLiveElements();
    EXPECT_TRUE(hound->getStats()->isDisposed() || hound->getStats()->isDying());
}

TEST(DifficultyTests, MusicFollowsTheDifficultyWithAHoldAndACrossfade)
{
    using namespace std::chrono_literals;
    EXPECT_EQ(difficulty::songFor(0, 9), 0);
    EXPECT_EQ(difficulty::songFor(4, 9), 4);
    EXPECT_EQ(difficulty::songFor(23, 9), 8); // the last song plays on
    EXPECT_EQ(difficulty::songFor(3, 0), -1);

    goe::music::byDifficulty music;
    const auto t0 = goe::music::byDifficulty::clock::now();
    EXPECT_EQ(music.choose(0, 9, t0), 0); // the first song starts at once, fading in
    EXPECT_FLOAT_EQ(music.mix(t0), 0.0f);
    EXPECT_NEAR(music.mix(t0 + 2500ms), 0.5f, 0.01f);
    EXPECT_FLOAT_EQ(music.mix(t0 + 5s), 1.0f);
    // D goes up, but the song has not played long enough yet
    EXPECT_EQ(music.choose(1, 9, t0 + 10s), 0);
    EXPECT_EQ(music.choose(1, 9, t0 + 23s), 1);
    EXPECT_FLOAT_EQ(music.mix(t0 + 23s), 0.0f);
    // walking back over the step does not flip it straight back
    EXPECT_EQ(music.choose(0, 9, t0 + 30s), 1);
    EXPECT_EQ(music.choose(0, 9, t0 + 46s), 0);
}
