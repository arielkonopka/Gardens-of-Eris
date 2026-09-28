/*
 * Tests for the building blocks: element stats and attributes, inventories, coordinates,
 * chambers, the word generator and the level generator.
 */
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "randomLevelGenerator.h"
#include "randomWordGen.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
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
    BOOST_REQUIRE(plr->collect(e));
    return e;
}

void ticks(int n)
{
    for (int c = 0; c < n; c++)
        bElem::tick();
}
} // namespace

BOOST_AUTO_TEST_SUITE(StatsTests)

BOOST_AUTO_TEST_CASE(InstanceIdsAreUniqueAndGrow)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    unsigned long last = 0;
    std::set<unsigned long> ids;
    for (int c = 0; c < 50; c++) {
        auto id = elementFactory::generateAnElement<wall>(mc, 0)->getStats()->getInstanceId();
        BOOST_CHECK_GT(id, last);
        BOOST_CHECK(ids.insert(id).second);
        last = id;
    }
}

BOOST_AUTO_TEST_CASE(WaitingLastsItsDuration)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    BOOST_CHECK(!w->getStats()->isWaiting());
    w->getStats()->setWaiting(5);
    BOOST_CHECK(w->getStats()->isWaiting());
    ticks(4);
    BOOST_CHECK(w->getStats()->isWaiting());
    ticks(1);
    BOOST_CHECK(!w->getStats()->isWaiting());
    w->getStats()->setWaiting(100);
    w->getStats()->stopWaiting();
    BOOST_CHECK(!w->getStats()->isWaiting());
}

BOOST_AUTO_TEST_CASE(DyingAndDestroyingEndWithTheirTimers)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    w->getStats()->setKilled(10);
    w->getStats()->setDestroyed(20);
    BOOST_CHECK(w->getStats()->isDying());
    BOOST_CHECK(w->getStats()->isDestroying());
    BOOST_CHECK_EQUAL(w->getStats()->getKillTimeReq(), 10u);
    BOOST_CHECK_EQUAL(w->getStats()->getDestTimeReq(), 20u);
    ticks(10);
    BOOST_CHECK(!w->getStats()->isDying());
    BOOST_CHECK(w->getStats()->isDestroying());
    ticks(10);
    BOOST_CHECK(!w->getStats()->isDestroying());
}

BOOST_AUTO_TEST_CASE(PointsStartAtZeroAndAreKeptPerKind)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    BOOST_CHECK_EQUAL(w->getStats()->getPoints(COLLECTS), 0);
    w->getStats()->setPoints(COLLECTS, 3);
    w->getStats()->setPoints(TOTAL, 11);
    BOOST_CHECK_EQUAL(w->getStats()->getPoints(COLLECTS), 3);
    BOOST_CHECK_EQUAL(w->getStats()->getPoints(TOTAL), 11);
}

BOOST_AUTO_TEST_CASE(SteppingOnLinksTheStack)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(4, 4));
    auto floor = mc->getElement(2, 2);
    auto brick = elementFactory::generateAnElement<brickCluster>(mc, 0);
    BOOST_REQUIRE(brick->stepOnElement(floor));
    BOOST_CHECK(mc->getElement(2, 2) == brick);
    BOOST_CHECK(brick->getStats()->getSteppingOn() == floor);
    BOOST_CHECK(floor->getStats()->getStandingOn().lock() == brick);
    BOOST_CHECK(brick->getStats()->getMyPosition() == coords(2, 2));
    BOOST_CHECK(brick->getBoard() == mc);
    // removing it puts the floor back on top
    brick->removeElement();
    BOOST_CHECK(mc->getElement(2, 2) == floor);
    BOOST_CHECK(floor->getStats()->getStandingOn().lock() == nullptr);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(AttributeTests)

BOOST_AUTO_TEST_CASE(DefaultsComeFromTheConfig)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(3, 3));
    auto floor = mc->getElement(0, 0);
    BOOST_CHECK(floor->getAttrs()->isSteppable());
    BOOST_CHECK(!floor->getAttrs()->isCollectible());
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    BOOST_CHECK(!w->getAttrs()->isSteppable());
    BOOST_CHECK(!w->getAttrs()->isMovable());
    auto k = elementFactory::generateAnElement<key>(mc, 3);
    BOOST_CHECK(k->getAttrs()->isCollectible());
    BOOST_CHECK_EQUAL(k->getAttrs()->getSubtype(), 3);
    auto t = elementFactory::generateAnElement<teleport>(mc, 9);
    BOOST_CHECK(t->getAttrs()->isInteractive());
    auto brick = elementFactory::generateAnElement<brickCluster>(mc, 0);
    BOOST_CHECK(brick->getAttrs()->isMovable());
    BOOST_CHECK(brick->getAttrs()->canBePushed());
}

BOOST_AUTO_TEST_CASE(EnergyStaysWithinItsLimits)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto m = elementFactory::generateAnElement<monster>(mc, 0);
    int maxE = m->getAttrs()->getMaxEnergy();
    m->getAttrs()->setEnergy(maxE + 1000);
    BOOST_CHECK_EQUAL(m->getAttrs()->getEnergy(), maxE);
    m->getAttrs()->setEnergy(-5);
    BOOST_CHECK_EQUAL(m->getAttrs()->getEnergy(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(InventoryTests)

BOOST_AUTO_TEST_CASE(CollectedThingsLandInTheRightPlace)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    auto gun = give<plainGun>(mc, plr, 0);
    auto k = give<key>(mc, plr, 2);
    auto apple = give<goldenApple>(mc, plr, 0);
    BOOST_CHECK(inv->getActiveWeapon() == gun);
    BOOST_CHECK(inv->getKey(bElemTypes::_key, 2, false) == k);
    BOOST_CHECK(inv->getKey(bElemTypes::_key, 3, false) == nullptr);
    BOOST_CHECK_EQUAL(inv->countTokens(bElemTypes::_key, 2), 1);
    BOOST_CHECK_EQUAL(inv->countTokens(bElemTypes::_goldenAppleType, apple->getAttrs()->getSubtype()), 1);
    for (auto &e : {gun, k, apple}) {
        BOOST_CHECK(e->getStats()->isCollected());
        BOOST_CHECK(e->getStats()->getCollector().lock() == plr);
        BOOST_CHECK(inv->findInInventory(e->getStats()->getInstanceId()));
    }
    // collected elements left the board
    BOOST_CHECK(mc->getElement(2, 1)->getType() == bElemTypes::_floorType);
}

BOOST_AUTO_TEST_CASE(TakingAKeyRemovesIt)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    auto k = give<key>(mc, plr, 4);
    BOOST_CHECK(inv->getKey(bElemTypes::_key, 4, true) == k);
    BOOST_CHECK(inv->getKey(bElemTypes::_key, 4, false) == nullptr);
    BOOST_CHECK_EQUAL(inv->countTokens(bElemTypes::_key, 4), 0);
}

BOOST_AUTO_TEST_CASE(NextGunCyclesThroughWeapons)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    BOOST_CHECK(!inv->nextGun());
    auto g1 = give<plainGun>(mc, plr, 0);
    auto g2 = give<bazooka>(mc, plr, 0);
    auto first = inv->getActiveWeapon();
    BOOST_REQUIRE(first);
    BOOST_CHECK(inv->nextGun());
    auto second = inv->getActiveWeapon();
    BOOST_CHECK(second != first);
    BOOST_CHECK(second == g1 || second == g2);
    BOOST_CHECK(inv->nextGun());
    BOOST_CHECK(inv->getActiveWeapon() == first);
}

BOOST_AUTO_TEST_CASE(RequestTokensTakesAtMostWhatIsThere)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    for (int c = 0; c < 3; c++)
        give<goldenApple>(mc, plr, 0);
    int st = -1; // any subtype
    BOOST_CHECK_EQUAL(inv->requestTokens(2, bElemTypes::_goldenAppleType, st), 2);
    BOOST_CHECK_EQUAL(inv->requestTokens(5, bElemTypes::_goldenAppleType, st), 1);
    BOOST_CHECK_EQUAL(inv->requestTokens(1, bElemTypes::_goldenAppleType, st), 0);
}

BOOST_AUTO_TEST_CASE(MergingMovesEverythingOver)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(8, 8), plr);
    auto other = elementFactory::generateAnElement<player>(mc, 0);
    other->stepOnElement(mc->getElement(5, 5));
    auto gun = elementFactory::generateAnElement<plainGun>(mc, 0);
    gun->stepOnElement(mc->getElement(6, 5));
    BOOST_REQUIRE(other->collect(gun));
    auto inv = plr->getAttrs()->getInventory();
    BOOST_CHECK(inv->getActiveWeapon() == nullptr);
    BOOST_REQUIRE(inv->mergeInventory(other->getAttrs()->getInventory()));
    BOOST_CHECK(inv->getActiveWeapon() == gun);
    BOOST_CHECK(gun->getStats()->getCollector().lock() == plr);
    BOOST_CHECK(!inv->mergeInventory(nullptr));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(CoordsTests)

BOOST_AUTO_TEST_CASE(Arithmetic)
{
    myUtility::Coords a(3, 4), b(1, 2);
    BOOST_CHECK(a + b == myUtility::Coords(4, 6));
    BOOST_CHECK(a - b == myUtility::Coords(2, 2));
    BOOST_CHECK(a * 2 == myUtility::Coords(6, 8));
    BOOST_CHECK_CLOSE(myUtility::Coords(0, 0).distance(myUtility::Coords(3, 4)), 5.0, 1e-9);
    coords c(3, 4);
    BOOST_CHECK(c + coords(1, 1) == coords(4, 5));
    BOOST_CHECK_CLOSE(c.distance(coords(0, 0)), 5.0f, 1e-4);
}

BOOST_AUTO_TEST_CASE(Directions)
{
    for (auto d : {dir::direction::UP, dir::direction::LEFT, dir::direction::DOWN, dir::direction::RIGHT}) {
        auto o = dir::getOppositeDirection(d);
        BOOST_CHECK(o != d);
        BOOST_CHECK(dir::getOppositeDirection(o) == d);
        // a step and its opposite cancel out
        BOOST_CHECK(dir::dirToCoords(d) + dir::dirToCoords(o) == coords(0, 0));
    }
    BOOST_CHECK(dir::dirToCoords(dir::direction::UP) == coords(0, -1));
    BOOST_CHECK(dir::dirToCoords(dir::direction::RIGHT) == coords(1, 0));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ChamberTests)

BOOST_AUTO_TEST_CASE(OutOfRangeCellsAreEmpty)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(4, 3));
    BOOST_CHECK(mc->getElement(-1, 0) == nullptr);
    BOOST_CHECK(mc->getElement(0, -1) == nullptr);
    BOOST_CHECK(mc->getElement(4, 0) == nullptr);
    BOOST_CHECK(mc->getElement(0, 3) == nullptr);
    BOOST_CHECK(mc->getElement(3, 2) != nullptr);
}

BOOST_AUTO_TEST_CASE(ChambersAreRegisteredWithUniqueIds)
{
    inputManager::getInstance(true);
    auto a = chamber::makeNewChamber(coords(2, 2));
    auto b = chamber::makeNewChamber(coords(2, 2));
    BOOST_CHECK_NE(a->getInstanceId(), b->getInstanceId());
    auto registered = [](const std::shared_ptr<chamber> &c) {
        return std::find(chamber::allChambers.begin(), chamber::allChambers.end(), c) != chamber::allChambers.end();
    };
    BOOST_CHECK(registered(a));
    BOOST_CHECK(registered(b));
}

BOOST_AUTO_TEST_CASE(VisitedCellsStayVisited)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(5, 5));
    BOOST_CHECK(mc->visitPosition(coords(2, 2)));
    BOOST_CHECK(!mc->visitPosition(NOCOORDS));
    mc->setVisible(coords(1, 1), 7);
    BOOST_CHECK_EQUAL(mc->isVisible(coords(1, 1)), 7);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(GeneratorTests)

BOOST_AUTO_TEST_CASE(WordsAreMadeOfSyllables)
{
    randomWordGen gen;
    std::set<std::string> words;
    for (int c = 0; c < 20; c++) {
        auto w = gen.generateWord(3);
        BOOST_CHECK(!w.empty());
        for (char ch : w)
            BOOST_CHECK(std::isalpha((unsigned char) ch));
        words.insert(w);
    }
    BOOST_CHECK_GT(words.size(), 1u);
}

BOOST_AUTO_TEST_CASE(GeneratedLevelIsWalledAndConsistent)
{
    inputManager::getInstance(true);
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    auto rl = new randomLevelGenerator(64, 64);
    BOOST_REQUIRE(rl->generateLevel(5));
    delete rl;
    auto mc = chamber::allChambers.back();
    BOOST_REQUIRE(mc);
    BOOST_CHECK(mc->getSize() == coords(64, 64));
    int players = 0, floors = 0;
    for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++) {
            auto top = mc->getElement(x, y);
            BOOST_REQUIRE(top);
            // the outer border is solid wall
            if (x == 0 || y == 0 || x == 63 || y == 63)
                BOOST_CHECK_EQUAL(top->getType(), bElemTypes::_wallType);
            // every element in the stack knows where it is and on which board
            for (auto e = top; e; e = e->getStats()->getSteppingOn()) {
                BOOST_CHECK(e->getStats()->getMyPosition() == coords(x, y));
                BOOST_CHECK(e->getBoard() == mc);
                if (e->getType() == bElemTypes::_player)
                    players++;
                if (e->getType() == bElemTypes::_floorType)
                    floors++;
            }
            // the floor is always the bottom of the stack
            auto bottom = top;
            while (bottom->getStats()->getSteppingOn())
                bottom = bottom->getStats()->getSteppingOn();
            BOOST_CHECK_EQUAL(bottom->getType(), bElemTypes::_floorType);
        }
    BOOST_CHECK_GE(players, 1);
    BOOST_CHECK_EQUAL(floors, 64 * 64);
}

BOOST_AUTO_TEST_SUITE_END()
