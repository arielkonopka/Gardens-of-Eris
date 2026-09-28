/*
 * Regression tests: each case pins down a bug that was fixed, so it cannot come back unnoticed.
 * The comment on each case names the fix.
 */
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <set>

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

BOOST_AUTO_TEST_SUITE(RegressionTests)

// PR #261: every element used to deep-copy the whole sprite config, so a 500x500 level took
// 107 s to build. 40,000 elements now take a few milliseconds; the bound leaves a wide margin.
BOOST_AUTO_TEST_CASE(CreatingElementsIsCheap)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(2, 2));
    auto start = std::chrono::steady_clock::now();
    for (int c = 0; c < 20000; c++) {
        elementFactory::generateAnElement<wall>(mc, 0);
        elementFactory::generateAnElement<floorElement>(mc, 0);
    }
    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    BOOST_CHECK_LT(seconds, 3.0);
}

// PR #262: makeNewChamber(Coords) created the floor twice, stacking a second floor on every cell.
BOOST_AUTO_TEST_CASE(NewChamberHasOneFloorPerCell)
{
    inputManager::getInstance(true);
    for (auto mc : {chamber::makeNewChamber(coords(7, 5)), chamber::makeNewChamber(myUtility::Coords(7, 5))}) {
        BOOST_CHECK(mc->getSize() == coords(7, 5));
        for (int x = 0; x < 7; x++)
            for (int y = 0; y < 5; y++) {
                auto e = mc->getElement(x, y);
                BOOST_REQUIRE(e);
                BOOST_CHECK_EQUAL(e->getType(), bElemTypes::_floorType);
                BOOST_CHECK(e->getStats()->getSteppingOn() == nullptr);
                BOOST_CHECK(e->getStats()->getMyPosition() == coords(x, y));
            }
    }
}

// PR #262: teleporters of a level still being generated must not be paired with.
BOOST_AUTO_TEST_CASE(TeleportersInABatchStayHiddenUntilItEnds)
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
        BOOST_REQUIRE(near->interact(brick));
        BOOST_CHECK_LE(manhattan(brick->getStats()->getMyPosition(), coords(3, 5)), 1);
    }
    // once the batch ends the hidden teleporter is published, and a new one pairs with it
    for (int c = 0; c < 200; c++)
        bElem::tick();
    auto late = elementFactory::generateAnElement<teleport>(mc, subtype);
    late->stepOnElement(mc->getElement(12, 8));
    auto brick = elementFactory::generateAnElement<brickCluster>(mc, 0);
    brick->stepOnElement(mc->getElement(12, 2));
    BOOST_REQUIRE(late->interact(brick));
    auto landed = brick->getStats()->getMyPosition();
    BOOST_CHECK(manhattan(landed, coords(25, 5)) <= 1 || manhattan(landed, coords(3, 5)) <= 1);
}

// PR #262: pairing always took the same candidate; now it picks at random.
BOOST_AUTO_TEST_CASE(TeleporterPairingIsRandom)
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
    BOOST_CHECK_GE(chosen.size(), 2u);
}

// PR #263: a puppet master handed to a drone was collected, had no board, never became live,
// and the drone stood still forever.
BOOST_AUTO_TEST_CASE(DroneWithAPuppetMasterMoves)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(15, 15), plr);
    plr->removeElement();
    plr->stepOnElement(mc->getElement(3, 3));
    auto brain = puppetMasterFR::create(mc, puppetMasterFR::patrol);
    brain->stepOnElement(mc->getElement(4, 3));
    BOOST_REQUIRE(plr->collect(brain));
    auto drone = elementFactory::generateAnElement<patrollingDrone>(mc, 0);
    drone->stepOnElement(mc->getElement(8, 8));
    BOOST_REQUIRE(drone->interact(plr));
    BOOST_CHECK(drone->getStats()->hasActivatedMechanics());
    BOOST_CHECK(contains(mc->liveElems, drone));
    auto start = drone->getStats()->getMyPosition();
    bool moved = false;
    for (int c = 0; c < 2000 && !moved; c++) {
        bElem::runLiveElements();
        moved = !(drone->getStats()->getMyPosition() == start);
    }
    BOOST_CHECK(moved);
}

// PR #264: validate() returned (-65535, 65535) for cells off the board, which never matched NOCOORDS.
BOOST_AUTO_TEST_CASE(OffBoardCoordinatesAreNOCOORDS)
{
    BOOST_CHECK(coords(5, 5).validate(coords(3, 3)) == NOCOORDS);
    BOOST_CHECK(coords(-1, 0).validate(coords(3, 3)) == NOCOORDS);
    BOOST_CHECK(coords(0, -1).validate(coords(3, 3)) == NOCOORDS);
    BOOST_CHECK(coords(2, 2).validate(coords(3, 3)) == coords(2, 2));

    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(3, 3));
    auto corner = mc->getElement(0, 0);
    BOOST_CHECK(corner->getAbsCoords(dir::direction::LEFT) == NOCOORDS);
    BOOST_CHECK(corner->getAbsCoords(dir::direction::UP) == NOCOORDS);
    BOOST_CHECK(corner->getAbsCoords(dir::direction::RIGHT) == coords(1, 0));
    BOOST_CHECK(corner->getElementInDirection(dir::direction::LEFT) == nullptr);
    BOOST_CHECK(corner->getElementInDirection(dir::direction::DOWN) == mc->getElement(0, 1));
    BOOST_CHECK(!corner->isSteppableDirection(dir::direction::UP));
    BOOST_CHECK(corner->isSteppableDirection(dir::direction::RIGHT));
    // an element that is not on a board has no neighbours
    auto loose = elementFactory::generateAnElement<wall>(mc, 0);
    BOOST_CHECK(loose->getElementInDirection(dir::direction::RIGHT) == nullptr);
    BOOST_CHECK(loose->getAbsCoords(dir::direction::RIGHT) == NOCOORDS);
}

// PR #264: runLiveElements was rewritten to compact its lists in one pass; it must still drop
// deregistered and disposed elements, keep the rest, and run elements registered mid-tick.
BOOST_AUTO_TEST_CASE(LiveElementBookkeeping)
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
        BOOST_REQUIRE(contains(mc->liveElems, m));
    monsters[1]->deregisterLiveElement(monsters[1]->getStats()->getInstanceId());
    monsters[4]->disposeElement();
    bElem::runLiveElements();
    BOOST_CHECK(!contains(mc->liveElems, monsters[1]));
    BOOST_CHECK(!contains(mc->liveElems, monsters[4]));
    for (int i : {0, 2, 3, 5})
        BOOST_CHECK(contains(mc->liveElems, monsters[i]));
    // the player runs separately and is never kept in the list
    BOOST_CHECK(!contains(mc->liveElems, plr));
    // no element is listed twice
    std::set<bElem *> seen;
    for (auto &e : mc->liveElems)
        BOOST_CHECK(seen.insert(e.get()).second);
    // an element registered again after deregistration comes back
    monsters[1]->registerLiveElement(monsters[1]);
    bElem::runLiveElements();
    BOOST_CHECK(contains(mc->liveElems, monsters[1]));
}

// PR #264: getActivePlayer used a throw-away mutex; the behaviour to keep is that a disposed
// active player is replaced, and that an existing active player is returned as is.
BOOST_AUTO_TEST_CASE(ActivePlayerIsStable)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(5, 5), plr);
    BOOST_CHECK(player::getActivePlayer() == plr);
    BOOST_CHECK(player::getActivePlayer() == player::getActivePlayer());
    BOOST_CHECK(plr->getStats()->isActive());
    plr->disposeElement();
    BOOST_CHECK(player::getActivePlayer() != plr);
}

// Found while writing these tests: disposing a collected element removed it from its collector's
// inventory, which could drop the last reference and free it in the middle of the call
// (std::bad_weak_ptr). requestTokens also erased the wrong token afterwards.
BOOST_AUTO_TEST_CASE(DisposingACollectedElementIsSafe)
{
    std::shared_ptr<bElem> plr;
    auto mc = roomWithPlayer(coords(6, 6), plr);
    auto inv = plr->getAttrs()->getInventory();
    {
        auto gun = elementFactory::generateAnElement<plainGun>(mc, 0);
        gun->stepOnElement(mc->getElement(1, 0));
        BOOST_REQUIRE(plr->collect(gun));
    }
    // the inventory now holds the only reference
    BOOST_CHECK_NO_THROW(inv->getActiveWeapon()->disposeElement());
    BOOST_CHECK(inv->getActiveWeapon() == nullptr);

    std::vector<unsigned long> ids;
    for (int c = 0; c < 3; c++) {
        auto apple = elementFactory::generateAnElement<goldenApple>(mc, 0);
        apple->stepOnElement(mc->getElement(1, 0));
        BOOST_REQUIRE(plr->collect(apple));
        ids.push_back(apple->getStats()->getInstanceId());
    }
    int taken = 0;
    BOOST_CHECK_NO_THROW(taken = inv->requestTokens(2, bElemTypes::_goldenAppleType, -1));
    BOOST_CHECK_EQUAL(taken, 2);
    // exactly one of the three apples is left
    int left = 0;
    for (auto id : ids)
        left += inv->findInInventory(id) ? 1 : 0;
    BOOST_CHECK_EQUAL(left, 1);
}

BOOST_AUTO_TEST_SUITE_END()
