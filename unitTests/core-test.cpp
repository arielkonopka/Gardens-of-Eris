/*
 * Tests for the building blocks: element stats and attributes, inventories, coordinates,
 * chambers, the word generator and the level generator.
 */
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "randomLevelGenerator.h"
#include "randomWordGen.h"
#include <gtest/gtest.h>
#include "testSupport.h"
#include <cctype>
#include <memory>
#include <set>

namespace {
std::shared_ptr<chamber> roomWithPlayer(coords size, std::shared_ptr<bElem> &plr)
{
    inputManager::getInstance(true);
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    auto mc = chamber::makeNewChamber(size);
    plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(1, 1));
    return mc;
}

/// places an element next to the player and has the player collect it
template <typename T>
std::shared_ptr<bElem> give(const std::shared_ptr<chamber> &mc, const std::shared_ptr<bElem> &plr, int subtype)
{
    auto e = elementFactory::generateAnElement<T>(mc, subtype);
    e->stepOnElement(mc->getElement(2, 1));
    REQUIRE_IN_HELPER(plr->collect(e));
    return e;
}

void ticks(int n)
{
    for (int c = 0; c < n; c++)
        bElem::tick();
}
} // namespace


TEST(StatsTests, InstanceIdsAreUniqueAndGrow)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    unsigned long last = 0;
    std::set<unsigned long> ids;
    for (int c = 0; c < 50; c++) {
        auto id = elementFactory::generateAnElement<wall>(mc, 0)->getStats()->getInstanceId();
        EXPECT_GT(id, last);
        EXPECT_TRUE(ids.insert(id).second);
        last = id;
    }
}

TEST(StatsTests, WaitingLastsItsDuration)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    EXPECT_TRUE(!w->getStats()->isWaiting());
    w->getStats()->setWaiting(5);
    EXPECT_TRUE(w->getStats()->isWaiting());
    ticks(4);
    EXPECT_TRUE(w->getStats()->isWaiting());
    ticks(1);
    EXPECT_TRUE(!w->getStats()->isWaiting());
    w->getStats()->setWaiting(100);
    w->getStats()->stopWaiting();
    EXPECT_TRUE(!w->getStats()->isWaiting());
}

TEST(StatsTests, DyingAndDestroyingEndWithTheirTimers)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    w->getStats()->setKilled(10);
    w->getStats()->setDestroyed(20);
    EXPECT_TRUE(w->getStats()->isDying());
    EXPECT_TRUE(w->getStats()->isDestroying());
    EXPECT_EQ(w->getStats()->getKillTimeReq(), 10u);
    EXPECT_EQ(w->getStats()->getDestTimeReq(), 20u);
    ticks(10);
    EXPECT_TRUE(!w->getStats()->isDying());
    EXPECT_TRUE(w->getStats()->isDestroying());
    ticks(10);
    EXPECT_TRUE(!w->getStats()->isDestroying());
}

TEST(StatsTests, PointsStartAtZeroAndAreKeptPerKind)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    EXPECT_EQ(w->getStats()->getPoints(COLLECTS), 0);
    w->getStats()->setPoints(COLLECTS, 3);
    w->getStats()->setPoints(TOTAL, 11);
    EXPECT_EQ(w->getStats()->getPoints(COLLECTS), 3);
    EXPECT_EQ(w->getStats()->getPoints(TOTAL), 11);
}

TEST(StatsTests, SteppingOnLinksTheStack)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(4, 4));
    auto floor = mc->getElement(2, 2);
    auto brick = elementFactory::generateAnElement<brickCluster>(mc, 0);
    ASSERT_TRUE(brick->stepOnElement(floor));
    EXPECT_TRUE(mc->getElement(2, 2) == brick);
    EXPECT_TRUE(brick->getStats()->getSteppingOn() == floor);
    EXPECT_TRUE(floor->getStats()->getStandingOn().lock() == brick);
    EXPECT_TRUE(brick->getStats()->getMyPosition() == coords(2, 2));
    EXPECT_TRUE(brick->getBoard() == mc);
    // removing it puts the floor back on top
    brick->removeElement();
    EXPECT_TRUE(mc->getElement(2, 2) == floor);
    EXPECT_TRUE(floor->getStats()->getStandingOn().lock() == nullptr);
}



TEST(AttributeTests, DefaultsComeFromTheConfig)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(3, 3));
    auto floor = mc->getElement(0, 0);
    EXPECT_TRUE(floor->getAttrs()->isSteppable());
    EXPECT_TRUE(!floor->getAttrs()->isCollectible());
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    EXPECT_TRUE(!w->getAttrs()->isSteppable());
    EXPECT_TRUE(!w->getAttrs()->isMovable());
    auto k = elementFactory::generateAnElement<key>(mc, 3);
    EXPECT_TRUE(k->getAttrs()->isCollectible());
    EXPECT_EQ(k->getAttrs()->getSubtype(), 3);
    auto t = elementFactory::generateAnElement<teleport>(mc, 9);
    EXPECT_TRUE(t->getAttrs()->isInteractive());
    auto brick = elementFactory::generateAnElement<brickCluster>(mc, 0);
    EXPECT_TRUE(brick->getAttrs()->isMovable());
    EXPECT_TRUE(brick->getAttrs()->canBePushed());
}

TEST(AttributeTests, EnergyStaysWithinItsLimits)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto m = elementFactory::generateAnElement<monster>(mc, 0);
    int maxE = m->getAttrs()->getMaxEnergy();
    m->getAttrs()->setEnergy(maxE + 1000);
    EXPECT_EQ(m->getAttrs()->getEnergy(), maxE);
    m->getAttrs()->setEnergy(-5);
    EXPECT_EQ(m->getAttrs()->getEnergy(), 0);
}



TEST(InventoryTests, CollectedThingsLandInTheRightPlace)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    auto gun = give<plainGun>(mc, plr, 0);
    auto k = give<key>(mc, plr, 2);
    auto apple = give<goldenApple>(mc, plr, 0);
    EXPECT_TRUE(inv->getActiveWeapon() == gun);
    EXPECT_TRUE(inv->getKey(bElemTypes::_key, 2, false) == k);
    EXPECT_TRUE(inv->getKey(bElemTypes::_key, 3, false) == nullptr);
    EXPECT_EQ(inv->countTokens(bElemTypes::_key, 2), 1);
    EXPECT_EQ(inv->countTokens(bElemTypes::_goldenAppleType, apple->getAttrs()->getSubtype()), 1);
    for (auto &e : {gun, k, apple}) {
        EXPECT_TRUE(e->getStats()->isCollected());
        EXPECT_TRUE(e->getStats()->getCollector().lock() == plr);
        EXPECT_TRUE(inv->findInInventory(e->getStats()->getInstanceId()));
    }
    // collected elements left the board
    EXPECT_TRUE(mc->getElement(2, 1)->getType() == bElemTypes::_floorType);
}

TEST(InventoryTests, TakingAKeyRemovesIt)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    auto k = give<key>(mc, plr, 4);
    EXPECT_TRUE(inv->getKey(bElemTypes::_key, 4, true) == k);
    EXPECT_TRUE(inv->getKey(bElemTypes::_key, 4, false) == nullptr);
    EXPECT_EQ(inv->countTokens(bElemTypes::_key, 4), 0);
}

TEST(InventoryTests, NextGunCyclesThroughWeapons)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    EXPECT_TRUE(!inv->nextGun());
    auto g1 = give<plainGun>(mc, plr, 0);
    auto g2 = give<bazooka>(mc, plr, 0);
    auto first = inv->getActiveWeapon();
    ASSERT_TRUE(first);
    EXPECT_TRUE(inv->nextGun());
    auto second = inv->getActiveWeapon();
    EXPECT_TRUE(second != first);
    EXPECT_TRUE(second == g1 || second == g2);
    EXPECT_TRUE(inv->nextGun());
    EXPECT_TRUE(inv->getActiveWeapon() == first);
}

TEST(InventoryTests, RequestTokensTakesAtMostWhatIsThere)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    for (int c = 0; c < 3; c++)
        give<goldenApple>(mc, plr, 0);
    int st = -1; // any subtype
    EXPECT_EQ(inv->requestTokens(2, bElemTypes::_goldenAppleType, st), 2);
    EXPECT_EQ(inv->requestTokens(5, bElemTypes::_goldenAppleType, st), 1);
    EXPECT_EQ(inv->requestTokens(1, bElemTypes::_goldenAppleType, st), 0);
}

TEST(InventoryTests, MergingMovesEverythingOver)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(8, 8), plr);
    auto other = elementFactory::generateAnElement<player>(mc, 0);
    other->stepOnElement(mc->getElement(5, 5));
    auto gun = elementFactory::generateAnElement<plainGun>(mc, 0);
    gun->stepOnElement(mc->getElement(6, 5));
    ASSERT_TRUE(other->collect(gun));
    auto inv = plr->getAttrs()->getInventory();
    EXPECT_TRUE(inv->getActiveWeapon() == nullptr);
    ASSERT_TRUE(inv->mergeInventory(other->getAttrs()->getInventory()));
    EXPECT_TRUE(inv->getActiveWeapon() == gun);
    EXPECT_TRUE(gun->getStats()->getCollector().lock() == plr);
    EXPECT_TRUE(!inv->mergeInventory(nullptr));
}



TEST(CoordsTests, Arithmetic)
{
    myUtility::Coords a(3, 4), b(1, 2);
    EXPECT_TRUE(a + b == myUtility::Coords(4, 6));
    EXPECT_TRUE(a - b == myUtility::Coords(2, 2));
    EXPECT_TRUE(a * 2 == myUtility::Coords(6, 8));
    EXPECT_NEAR(myUtility::Coords(0, 0).distance(myUtility::Coords(3, 4)), 5.0, std::abs(5.0) * (1e-9) / 100.0);
    coords c(3, 4);
    EXPECT_TRUE(c + coords(1, 1) == coords(4, 5));
    EXPECT_NEAR(c.distance(coords(0, 0)), 5.0f, std::abs(5.0f) * (1e-4) / 100.0);
}

TEST(CoordsTests, Directions)
{
    for (auto d : {dir::direction::UP, dir::direction::LEFT, dir::direction::DOWN, dir::direction::RIGHT}) {
        auto o = dir::getOppositeDirection(d);
        EXPECT_TRUE(o != d);
        EXPECT_TRUE(dir::getOppositeDirection(o) == d);
        // a step and its opposite cancel out
        EXPECT_TRUE(dir::dirToCoords(d) + dir::dirToCoords(o) == coords(0, 0));
    }
    EXPECT_TRUE(dir::dirToCoords(dir::direction::UP) == coords(0, -1));
    EXPECT_TRUE(dir::dirToCoords(dir::direction::RIGHT) == coords(1, 0));
}



TEST(ChamberTests, OutOfRangeCellsAreEmpty)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(4, 3));
    EXPECT_TRUE(mc->getElement(-1, 0) == nullptr);
    EXPECT_TRUE(mc->getElement(0, -1) == nullptr);
    EXPECT_TRUE(mc->getElement(4, 0) == nullptr);
    EXPECT_TRUE(mc->getElement(0, 3) == nullptr);
    EXPECT_TRUE(mc->getElement(3, 2) != nullptr);
}

TEST(ChamberTests, ChambersAreRegisteredWithUniqueIds)
{
    inputManager::getInstance(true);
    auto a = chamber::makeNewChamber(coords(2, 2));
    auto b = chamber::makeNewChamber(coords(2, 2));
    EXPECT_NE(a->getInstanceId(), b->getInstanceId());
    auto registered = [](const std::shared_ptr<chamber> &c) {
        return std::find(chamber::allChambers.begin(), chamber::allChambers.end(), c) != chamber::allChambers.end();
    };
    EXPECT_TRUE(registered(a));
    EXPECT_TRUE(registered(b));
}

TEST(ChamberTests, VisitedCellsStayVisited)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(5, 5));
    EXPECT_TRUE(mc->visitPosition(coords(2, 2)));
    EXPECT_TRUE(!mc->visitPosition(NOCOORDS));
    mc->setVisible(coords(1, 1), 7);
    EXPECT_EQ(mc->isVisible(coords(1, 1)), 7);
}



TEST(GeneratorTests, WordsAreMadeOfSyllables)
{
    randomWordGen gen;
    std::set<std::string> words;
    for (int c = 0; c < 20; c++) {
        auto w = gen.generateWord(3);
        EXPECT_TRUE(!w.empty());
        for (char ch : w)
            EXPECT_TRUE(std::isalpha((unsigned char) ch));
        words.insert(w);
    }
    EXPECT_GT(words.size(), 1u);
}

TEST(GeneratorTests, GeneratedLevelIsWalledAndConsistent)
{
    inputManager::getInstance(true);
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    ASSERT_TRUE(randomLevelGenerator(64, 64).generateLevel(5));
    auto mc = chamber::allChambers.back();
    ASSERT_TRUE(mc);
    EXPECT_TRUE(mc->getSize() == coords(64, 64));
    int players = 0, floors = 0;
    for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++) {
            auto top = mc->getElement(x, y);
            ASSERT_TRUE(top);
            // the outer border is solid wall
            if (x == 0 || y == 0 || x == 63 || y == 63)
                EXPECT_EQ(top->getType(), bElemTypes::_wallType);
            // every element in the stack knows where it is and on which board
            for (auto e = top; e; e = e->getStats()->getSteppingOn()) {
                EXPECT_TRUE(e->getStats()->getMyPosition() == coords(x, y));
                EXPECT_TRUE(e->getBoard() == mc);
                if (e->getType() == bElemTypes::_player)
                    players++;
                if (e->getType() == bElemTypes::_floorType)
                    floors++;
            }
            // the floor is always the bottom of the stack
            auto bottom = top;
            while (bottom->getStats()->getSteppingOn())
                bottom = bottom->getStats()->getSteppingOn();
            EXPECT_EQ(bottom->getType(), bElemTypes::_floorType);
        }
    EXPECT_GE(players, 1);
    EXPECT_EQ(floors, 64 * 64);
}

