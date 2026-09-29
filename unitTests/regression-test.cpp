/*
 * Regression tests: each case pins down a bug that was fixed, so it cannot come back unnoticed.
 * The comment on each case names the fix.
 */
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include <gtest/gtest.h>
#include "testSupport.h"
#include <chrono>
#include <cstdlib>
#include <memory>
#include <set>
#include <thread>

namespace {
/// a fresh chamber with an active player in its corner, so bElem::runLiveElements ticks it
std::shared_ptr<chamber> roomWithPlayer(coords size, std::shared_ptr<bElem> &plr)
{
    inputManager::getInstance(true);
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    auto mc = chamber::makeNewChamber(size);
    plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(0, 0));
    return mc;
}

bool contains(const std::vector<std::shared_ptr<bElem>> &v, const std::shared_ptr<bElem> &e)
{
    return std::find(v.begin(), v.end(), e) != v.end();
}

int manhattan(coords a, coords b)
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}
} // namespace


// PR #261: every element used to deep-copy the whole sprite config, so a 500x500 level took
// 107 s to build. 40,000 elements now take a few milliseconds; the bound leaves a wide margin.
TEST(RegressionTests, CreatingElementsIsCheap)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto start = std::chrono::steady_clock::now();
    for (int c = 0; c < 20000; c++) {
        elementFactory::generateAnElement<wall>(mc, 0);
        elementFactory::generateAnElement<floorElement>(mc, 0);
    }
    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    EXPECT_LT(seconds, 3.0);
}

// PR #262: makeNewChamber(Coords) created the floor twice, stacking a second floor on every cell.
TEST(RegressionTests, NewChamberHasOneFloorPerCell)
{
    inputManager::getInstance(true);
    for (auto mc : {chamber::makeNewChamber(coords(7, 5)), chamber::makeNewChamber(myUtility::Coords(7, 5))}) {
        EXPECT_TRUE(mc->getSize() == coords(7, 5));
        for (int x = 0; x < 7; x++)
            for (int y = 0; y < 5; y++) {
                auto e = mc->getElement(x, y);
                ASSERT_TRUE(e);
                EXPECT_EQ(e->getType(), bElemTypes::_floorType);
                EXPECT_TRUE(e->getStats()->getSteppingOn() == nullptr);
                EXPECT_TRUE(e->getStats()->getMyPosition() == coords(x, y));
            }
    }
}

// PR #262: teleporters of a level still being generated must not be paired with.
TEST(RegressionTests, TeleportersInABatchStayHiddenUntilItEnds)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(30, 10), plr);
    // the very first teleporter ever created becomes an inactive receiver; get it out of the way
    elementFactory::generateAnElement<teleport>(mc, 0)->stepOnElement(mc->getElement(29, 9));
    const int subtype = 5001;
    auto near = elementFactory::generateAnElement<teleport>(mc, subtype);
    near->stepOnElement(mc->getElement(3, 5));
    std::shared_ptr<teleport> hidden;
    {
        teleport::registrationBatch batch;
        hidden = elementFactory::generateAnElement<teleport>(mc, subtype);
        hidden->stepOnElement(mc->getElement(25, 5));
        // while the batch is open, near finds no partner, and sends things around itself
        auto brick = elementFactory::generateAnElement<brickCluster>(mc, 0);
        brick->stepOnElement(mc->getElement(10, 2));
        ASSERT_TRUE(near->interact(brick));
        EXPECT_LE(manhattan(brick->getStats()->getMyPosition(), coords(3, 5)), 1);
    }
    // once the batch ends the hidden teleporter is published, and a new one pairs with it
    for (int c = 0; c < 200; c++)
        bElem::tick();
    auto late = elementFactory::generateAnElement<teleport>(mc, subtype);
    late->stepOnElement(mc->getElement(12, 8));
    auto brick = elementFactory::generateAnElement<brickCluster>(mc, 0);
    brick->stepOnElement(mc->getElement(12, 2));
    ASSERT_TRUE(late->interact(brick));
    auto landed = brick->getStats()->getMyPosition();
    EXPECT_TRUE(manhattan(landed, coords(25, 5)) <= 1 || manhattan(landed, coords(3, 5)) <= 1);
}

// PR #262: pairing always took the same candidate; now it picks at random.
TEST(RegressionTests, TeleporterPairingIsRandom)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(40, 12), plr);
    elementFactory::generateAnElement<teleport>(mc, 0)->stepOnElement(mc->getElement(39, 11));
    const std::array<coords, 3> spots = {coords(10, 2), coords(20, 2), coords(30, 2)};
    std::set<int> chosen;
    for (int trial = 0; trial < 30 && chosen.size() < 2; trial++) {
        int subtype = 6000 + trial;
        std::vector<std::shared_ptr<teleport>> targets;
        for (auto s : spots) {
            auto t = elementFactory::generateAnElement<teleport>(mc, subtype);
            t->stepOnElement(mc->getElement(s.x, s.y + (trial % 8)));
            targets.push_back(t);
        }
        auto source = elementFactory::generateAnElement<teleport>(mc, subtype);
        source->stepOnElement(mc->getElement(2, 10));
        auto brick = elementFactory::generateAnElement<brickCluster>(mc, 0);
        brick->stepOnElement(mc->getElement(3, 11));
        if (!source->interact(brick))
            continue;
        auto at = brick->getStats()->getMyPosition();
        for (int i = 0; i < 3; i++)
            if (manhattan(at, targets[i]->getStats()->getMyPosition()) <= 1)
                chosen.insert(i);
        brick->disposeElement();
        for (auto &t : targets)
            t->disposeElement();
        source->disposeElement();
        for (int c = 0; c < 100; c++)
            bElem::tick();
    }
    EXPECT_GE(chosen.size(), 2u);
}

// PR #263: a puppet master handed to a drone was collected, had no board, never became live,
// and the drone stood still forever.
TEST(RegressionTests, DroneWithAPuppetMasterMoves)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(15, 15), plr);
    plr->removeElement();
    plr->stepOnElement(mc->getElement(3, 3));
    auto brain = puppetMasterFR::create(mc, puppetMasterFR::patrol);
    brain->stepOnElement(mc->getElement(4, 3));
    ASSERT_TRUE(plr->collect(brain));
    auto drone = elementFactory::generateAnElement<patrollingDrone>(mc, 0);
    drone->stepOnElement(mc->getElement(8, 8));
    ASSERT_TRUE(drone->interact(plr));
    EXPECT_TRUE(drone->getStats()->hasActivatedMechanics());
    EXPECT_TRUE(contains(mc->liveElems, drone));
    auto start = drone->getStats()->getMyPosition();
    bool moved = false;
    for (int c = 0; c < 2000 && !moved; c++) {
        bElem::runLiveElements();
        moved = !(drone->getStats()->getMyPosition() == start);
    }
    EXPECT_TRUE(moved);
}

// PR #264: validate() returned (-65535, 65535) for cells off the board, which never matched NOCOORDS.
TEST(RegressionTests, OffBoardCoordinatesAreNOCOORDS)
{
    EXPECT_TRUE(coords(5, 5).validate(coords(3, 3)) == NOCOORDS);
    EXPECT_TRUE(coords(-1, 0).validate(coords(3, 3)) == NOCOORDS);
    EXPECT_TRUE(coords(0, -1).validate(coords(3, 3)) == NOCOORDS);
    EXPECT_TRUE(coords(2, 2).validate(coords(3, 3)) == coords(2, 2));

    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(3, 3));
    auto corner = mc->getElement(0, 0);
    EXPECT_TRUE(corner->getAbsCoords(dir::direction::LEFT) == NOCOORDS);
    EXPECT_TRUE(corner->getAbsCoords(dir::direction::UP) == NOCOORDS);
    EXPECT_TRUE(corner->getAbsCoords(dir::direction::RIGHT) == coords(1, 0));
    EXPECT_TRUE(corner->getElementInDirection(dir::direction::LEFT) == nullptr);
    EXPECT_TRUE(corner->getElementInDirection(dir::direction::DOWN) == mc->getElement(0, 1));
    EXPECT_TRUE(!corner->isSteppableDirection(dir::direction::UP));
    EXPECT_TRUE(corner->isSteppableDirection(dir::direction::RIGHT));
    // an element that is not on a board has no neighbours
    auto loose = elementFactory::generateAnElement<wall>(mc, 0);
    EXPECT_TRUE(loose->getElementInDirection(dir::direction::RIGHT) == nullptr);
    EXPECT_TRUE(loose->getAbsCoords(dir::direction::RIGHT) == NOCOORDS);
}

// PR #264: runLiveElements was rewritten to compact its lists in one pass; it must still drop
// deregistered and disposed elements, keep the rest, and run elements registered mid-tick.
TEST(RegressionTests, LiveElementBookkeeping)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(20, 20), plr);
    std::vector<std::shared_ptr<bElem>> monsters;
    for (int c = 0; c < 6; c++) {
        auto m = elementFactory::generateAnElement<monster>(mc, 0);
        m->stepOnElement(mc->getElement(3 + 2 * c, 10));
        monsters.push_back(m);
    }
    for (auto &m : monsters)
        ASSERT_TRUE(contains(mc->liveElems, m));
    monsters[1]->deregisterLiveElement(monsters[1]->getStats()->getInstanceId());
    monsters[4]->disposeElement();
    bElem::runLiveElements();
    EXPECT_TRUE(!contains(mc->liveElems, monsters[1]));
    EXPECT_TRUE(!contains(mc->liveElems, monsters[4]));
    for (int i : {0, 2, 3, 5})
        EXPECT_TRUE(contains(mc->liveElems, monsters[i]));
    // the player runs separately and is never kept in the list
    EXPECT_TRUE(!contains(mc->liveElems, plr));
    // no element is listed twice
    std::set<bElem *> seen;
    for (auto &e : mc->liveElems)
        EXPECT_TRUE(seen.insert(e.get()).second);
    // an element registered again after deregistration comes back
    monsters[1]->registerLiveElement(monsters[1]);
    bElem::runLiveElements();
    EXPECT_TRUE(contains(mc->liveElems, monsters[1]));
}

// PR #264: getActivePlayer used a throw-away mutex; the behaviour to keep is that a disposed
// active player is replaced, and that an existing active player is returned as is.
TEST(RegressionTests, ActivePlayerIsStable)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(5, 5), plr);
    EXPECT_TRUE(player::getActivePlayer() == plr);
    EXPECT_TRUE(player::getActivePlayer() == player::getActivePlayer());
    EXPECT_TRUE(plr->getStats()->isActive());
    plr->disposeElement();
    EXPECT_TRUE(player::getActivePlayer() != plr);
}

// Found while writing these tests: disposing a collected element removed it from its collector's
// inventory, which could drop the last reference and free it in the middle of the call
// (std::bad_weak_ptr). requestTokens also erased the wrong token afterwards.
TEST(RegressionTests, DisposingACollectedElementIsSafe)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    {
        auto gun = elementFactory::generateAnElement<plainGun>(mc, 0);
        gun->stepOnElement(mc->getElement(1, 0));
        ASSERT_TRUE(plr->collect(gun));
    }
    // the inventory now holds the only reference
    EXPECT_NO_THROW(inv->getActiveWeapon()->disposeElement());
    EXPECT_TRUE(inv->getActiveWeapon() == nullptr);

    std::vector<unsigned long> ids;
    for (int c = 0; c < 3; c++) {
        auto apple = elementFactory::generateAnElement<goldenApple>(mc, 0);
        apple->stepOnElement(mc->getElement(1, 0));
        ASSERT_TRUE(plr->collect(apple));
        ids.push_back(apple->getStats()->getInstanceId());
    }
    int taken = 0;
    EXPECT_NO_THROW(taken = inv->requestTokens(2, bElemTypes::_goldenAppleType, -1));
    EXPECT_EQ(taken, 2);
    // exactly one of the three apples is left
    int left = 0;
    for (auto id : ids)
        left += inv->findInInventory(id) ? 1 : 0;
    EXPECT_EQ(left, 1);
}


// Crash report of 2026-09-29: a player made while a level was built in the background could
// become the active player once the last avatar died, dropping the game into a half-built level.
TEST(RegressionTests, PlayersBuiltInTheBackgroundNeverTakeOverTheGame)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    plr->disposeElement();
    ASSERT_EQ(player::getActivePlayer(), nullptr);

    std::shared_ptr<bElem> built;
    std::thread([&]() {
        player::backgroundScope background;
        built = elementFactory::generateAnElement<player>(mc, 0);
        built->stepOnElement(mc->getElement(3, 3));
    }).join();
    ASSERT_TRUE(built);
    EXPECT_EQ(player::getActivePlayer(), nullptr);
    EXPECT_FALSE(built->getStats()->isActive());

    // on the game thread a new player still takes over, as the first level's player does
    auto next = elementFactory::generateAnElement<player>(mc, 0);
    EXPECT_EQ(player::getActivePlayer(), next);
}

// Crash report of 2026-09-29: levels built in the background add golden apples while the game
// thread removes the ones that get shot or blown up; the apple list had no lock.
TEST(RegressionTests, ApplesCanBeAddedAndRemovedFromTwoThreads)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    const int before = goldenApple::getAppleNumber();
    constexpr int count = 555;
    std::vector<std::shared_ptr<bElem>> mine;
    for (int c = 0; c < count; c++)
        mine.push_back(elementFactory::generateAnElement<goldenApple>(mc, 0));
    std::vector<std::shared_ptr<bElem>> theirs;
    std::thread builder([&]() {
        for (int c = 0; c < count; c++)
            theirs.push_back(elementFactory::generateAnElement<goldenApple>(mc, 0));
    });
    for (auto &apple : mine)
        apple->disposeElement();
    builder.join();
    EXPECT_EQ(goldenApple::getAppleNumber(), before + count);
    for (auto &apple : theirs)
        apple->disposeElement();
    EXPECT_EQ(goldenApple::getAppleNumber(), before);
}

// The HUD draws goldenApple::getApple(0), which threw std::out_of_range once no apple was left out in
// the world, ending the game.
TEST(RegressionTests, AskingForAnAppleWhenNoneIsLeftIsSafe)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto apple = elementFactory::generateAnElement<goldenApple>(mc, 0);
    EXPECT_TRUE(goldenApple::getApple(goldenApple::getAppleNumber() - 1) == apple);
    EXPECT_NO_THROW(EXPECT_TRUE(goldenApple::getApple(goldenApple::getAppleNumber()) == nullptr));
    EXPECT_TRUE(goldenApple::getApple(-1) == nullptr);
}
