#include "elements.h"
#include "commons.h"
#include "chamber.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <memory>

/**
 * @brief Behaviour tests for the board elements: doors and keys, explosives, missiles, bunkers,
 * monsters, the static scenery (floor, wall, rubbish, brick clusters) and the kiki/bouba pair.
 */
namespace {

/// earlier test cases leave their player active, and only the active player's chamber ticks
void dropActivePlayers()
{
    inputManager::getInstance(true);
    while (auto old = player::getActivePlayer())
        old->disposeElement();
}

/// a chamber of the given size whose border is made of walls
std::shared_ptr<chamber> walledRoom(int w, int h)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(w, h));
    for (int x = 0; x < w; x++)
        for (auto p : {coords(x, 0), coords(x, h - 1)})
            elementFactory::generateAnElement<wall>(mc, 0)->stepOnElement(mc->getElement(p));
    for (int y = 1; y < h - 1; y++)
        for (auto p : {coords(0, y), coords(w - 1, y)})
            elementFactory::generateAnElement<wall>(mc, 0)->stepOnElement(mc->getElement(p));
    return mc;
}

template <class T>
std::shared_ptr<T> place(const std::shared_ptr<chamber> &mc, int subtype, int x, int y)
{
    auto e = elementFactory::generateAnElement<T>(mc, subtype);
    BOOST_REQUIRE(e->stepOnElement(mc->getElement(x, y)));
    return e;
}

/// the new player becomes the active one, so the chamber it stands in gets its mechanics run
std::shared_ptr<player> activePlayerAt(const std::shared_ptr<chamber> &mc, int x, int y)
{
    dropActivePlayers();
    auto p = place<player>(mc, 0, x, y);
    p->getStats()->setActive(true);
    BOOST_REQUIRE(player::getActivePlayer() == p);
    return p;
}

void run(int n)
{
    for (int c = 0; c < n; c++)
        bElem::runLiveElements();
}

void ticks(int n)
{
    for (int c = 0; c < n; c++)
        bElem::tick();
}

int typeAt(const std::shared_ptr<chamber> &mc, int x, int y)
{
    return mc->getElement(x, y)->getType();
}

bool isAt(const std::shared_ptr<bElem> &e, int x, int y)
{
    return e->getStats()->getMyPosition() == coords(x, y);
}

bool goneOrGoing(const std::shared_ptr<bElem> &e)
{
    return e->getStats()->isDisposed() || e->getStats()->isDestroying() || e->getStats()->isDying();
}

int keysOf(const std::shared_ptr<bElem> &who, int subtype)
{
    return who->getAttrs()->getInventory()->countTokens(bElemTypes::_key, subtype);
}

} // namespace

BOOST_AUTO_TEST_SUITE(ElementTests)

/* ---------------------------------------------------------------- doors and keys */

BOOST_AUTO_TEST_CASE(KeyIsACollectibleToken)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto k = place<key>(mc, 3, 3, 2);
    BOOST_CHECK_EQUAL(k->getType(), bElemTypes::_key);
    BOOST_CHECK(k->getAttrs()->isCollectible());
    BOOST_CHECK(!k->getAttrs()->isSteppable());
    BOOST_CHECK(plr->collect(k));
    BOOST_CHECK(k->getStats()->isCollected());
    BOOST_CHECK_EQUAL(typeAt(mc, 3, 2), bElemTypes::_floorType);
    BOOST_CHECK_EQUAL(keysOf(plr, 3), 1);
    BOOST_CHECK(plr->getAttrs()->getInventory()->getKey(bElemTypes::_key, 3, false) == k);
    BOOST_CHECK(plr->getAttrs()->getInventory()->getKey(bElemTypes::_key, 4, false) == nullptr);
}

BOOST_AUTO_TEST_CASE(NewDoorIsLockedClosedAndNotSteppable)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    for (int st : {0, 1, 2}) {
        auto d = place<door>(mc, st, 3, 2);
        BOOST_CHECK_EQUAL(d->getType(), bElemTypes::_door);
        BOOST_CHECK(!d->getAttrs()->isSteppable());
        BOOST_CHECK(d->getAttrs()->isLocked());
        BOOST_CHECK(!d->getAttrs()->isOpen());
        BOOST_CHECK(!plr->moveInDirection(dir::direction::RIGHT));
        BOOST_CHECK(isAt(plr, 2, 2));
        ticks(GoEConstants::_interactedTime + 1);
        d->disposeElement();
    }
}

BOOST_AUTO_TEST_CASE(DoorStaysLockedWithoutAKey)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 0, 3, 2);
    BOOST_CHECK(!d->interact(plr));
    ticks(GoEConstants::_interactedTime + 1);
    auto notACollector = place<brickCluster>(mc, 0, 3, 3);
    BOOST_CHECK(!d->interact(notACollector));
    BOOST_CHECK(d->getAttrs()->isLocked());
    BOOST_CHECK(!d->getAttrs()->isSteppable());
}

BOOST_AUTO_TEST_CASE(WrongKeySubtypeDoesNotOpenDoor)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 0, 3, 2);
    BOOST_REQUIRE(plr->collect(place<key>(mc, 2, 2, 3)));
    BOOST_CHECK(!d->interact(plr));
    BOOST_CHECK(d->getAttrs()->isLocked());
    BOOST_CHECK(!d->getAttrs()->isOpen());
    BOOST_CHECK_EQUAL(keysOf(plr, 2), 1); // the wrong key is not taken away
}

BOOST_AUTO_TEST_CASE(MatchingKeyOpensEvenDoorAndIsUsedUp)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 0, 3, 2);
    auto k = place<key>(mc, 0, 2, 3);
    BOOST_REQUIRE(plr->collect(k));
    BOOST_CHECK(d->interact(plr));
    BOOST_CHECK(d->getAttrs()->isOpen());
    BOOST_CHECK(!d->getAttrs()->isLocked());
    BOOST_CHECK(d->getAttrs()->isSteppable());
    BOOST_CHECK_EQUAL(keysOf(plr, 0), 0);
    BOOST_CHECK(plr->getAttrs()->getInventory()->getKey(bElemTypes::_key, 0, false) == nullptr);
    BOOST_CHECK(k->getStats()->isDisposed());
    // the way is free now
    BOOST_CHECK(plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK(isAt(plr, 3, 2));
    BOOST_CHECK(!plr->getStats()->isDying());
}

BOOST_AUTO_TEST_CASE(UnlockedDoorTogglesOnInteractionAfterCooldown)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 0, 3, 2);
    BOOST_REQUIRE(plr->collect(place<key>(mc, 0, 2, 3)));
    BOOST_REQUIRE(d->interact(plr));
    BOOST_CHECK(!d->interact(plr)); // still busy with the previous interaction
    BOOST_CHECK(d->getAttrs()->isOpen());
    ticks(GoEConstants::_interactedTime + 1);
    BOOST_CHECK(d->interact(plr)); // no key needed any more
    BOOST_CHECK(!d->getAttrs()->isOpen());
    BOOST_CHECK(!d->getAttrs()->isSteppable());
    BOOST_CHECK(!d->getAttrs()->isLocked());
    ticks(GoEConstants::_interactedTime + 1);
    BOOST_CHECK(d->interact(plr));
    BOOST_CHECK(d->getAttrs()->isOpen());
    BOOST_CHECK(d->getAttrs()->isSteppable());
}

BOOST_AUTO_TEST_CASE(PlayerWalkingIntoLockedDoorWithKeyOpensIt)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 4, 3, 2);
    BOOST_REQUIRE(plr->collect(place<key>(mc, 4, 2, 3)));
    BOOST_CHECK(plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK(d->getAttrs()->isOpen());
    BOOST_CHECK(isAt(plr, 2, 2)); // opening the door takes the move
    BOOST_CHECK_EQUAL(keysOf(plr, 4), 0);
}

BOOST_AUTO_TEST_CASE(OddDoorKeepsKeyWhenOpenedAndTakesItOnPassage)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 1, 3, 2);
    BOOST_REQUIRE(plr->collect(place<key>(mc, 1, 2, 3)));
    BOOST_CHECK(d->interact(plr));
    BOOST_CHECK(d->getAttrs()->isOpen());
    BOOST_CHECK(d->getAttrs()->isSteppable());
    BOOST_CHECK_EQUAL(keysOf(plr, 1), 1);
    BOOST_CHECK(plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK(isAt(plr, 3, 2));
    BOOST_CHECK(!plr->getStats()->isDying());
    BOOST_CHECK_EQUAL(keysOf(plr, 1), 0);
    // leaving the door locks it behind the player
    ticks(GoEConstants::_mov_delay + 1);
    BOOST_CHECK(plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK(isAt(plr, 4, 2));
    BOOST_CHECK(!d->getAttrs()->isOpen());
    BOOST_CHECK(d->getAttrs()->isLocked());
    BOOST_CHECK(!d->getAttrs()->isSteppable());
}

BOOST_AUTO_TEST_CASE(OddDoorKillsIntruderWithoutAKey)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 1, 3, 2);
    auto opener = place<bElem>(mc, 0, 3, 3);
    opener->getAttrs()->setCollect(true);
    BOOST_REQUIRE(opener->collect(place<key>(mc, 1, 4, 4)));
    BOOST_REQUIRE(d->interact(opener));
    BOOST_REQUIRE(d->getAttrs()->isSteppable());
    BOOST_CHECK(plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK(plr->getStats()->isDying());
}

BOOST_AUTO_TEST_CASE(DestroyedDoorsCrumbleOrTurnIntoWalls)
{
    auto mc = walledRoom(8, 8);
    auto odd = place<door>(mc, 1, 3, 3);
    BOOST_CHECK(odd->destroy());
    BOOST_CHECK_EQUAL(typeAt(mc, 3, 3), bElemTypes::_wallType);
    BOOST_CHECK(odd->getStats()->isDisposed());
    auto even = place<door>(mc, 0, 5, 5);
    BOOST_CHECK(even->destroy());
    BOOST_CHECK(even->getStats()->isDestroying());
    BOOST_CHECK_EQUAL(typeAt(mc, 5, 5), bElemTypes::_door);
}

/* ---------------------------------------------------------------- scenery */

BOOST_AUTO_TEST_CASE(FloorAndWallBasicAttributes)
{
    auto mc = walledRoom(6, 6);
    auto fl = mc->getElement(2, 2);
    BOOST_CHECK_EQUAL(fl->getType(), bElemTypes::_floorType);
    BOOST_CHECK(fl->getAttrs()->isSteppable());
    BOOST_CHECK(!fl->getAttrs()->isCollectible());
    BOOST_CHECK(!fl->getAttrs()->isKillable());
    BOOST_CHECK(!fl->getAttrs()->isDestroyable());
    auto w = mc->getElement(0, 2);
    BOOST_CHECK_EQUAL(w->getType(), bElemTypes::_wallType);
    BOOST_CHECK(!w->getAttrs()->isSteppable());
    BOOST_CHECK(!w->getAttrs()->isDestroyable());
    BOOST_CHECK(!w->getAttrs()->isKillable());
    BOOST_CHECK(!w->getAttrs()->isMovable());
    BOOST_CHECK(!w->destroy());
    BOOST_CHECK(!w->kill());
    BOOST_CHECK(!w->hurt(1000));
    BOOST_CHECK(!w->getStats()->isDestroying());
    BOOST_CHECK(!mc->getElement(1, 1)->isSteppableDirection(dir::direction::UP));
    BOOST_CHECK(mc->getElement(1, 1)->isSteppableDirection(dir::direction::DOWN));
}

BOOST_AUTO_TEST_CASE(PlayerWalkingOverRubbishPicksItUp)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto r = place<rubbish>(mc, 0, 3, 2);
    BOOST_CHECK(r->getAttrs()->isSteppable());
    BOOST_CHECK(r->getAttrs()->isCollectible());
    BOOST_CHECK(r->getAttrs()->isKillable());
    BOOST_CHECK(plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK(isAt(plr, 3, 2));
    BOOST_CHECK(r->getStats()->isCollected());
    BOOST_CHECK(plr->getStats()->getSteppingOn()->getType() == bElemTypes::_floorType);
}

BOOST_AUTO_TEST_CASE(RubbishStashHandsItsInventoryOver)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto r = elementFactory::generateAnElement<rubbish>(mc, 0);
    r->getAttrs()->setCollect(true);
    BOOST_REQUIRE(r->collect(place<key>(mc, 7, 5, 5)));
    BOOST_REQUIRE(r->stepOnElement(mc->getElement(3, 2)));
    BOOST_CHECK(plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK_EQUAL(keysOf(plr, 7), 1);
    BOOST_CHECK(plr->getAttrs()->getInventory()->getKey(bElemTypes::_key, 7, false) != nullptr);
}

BOOST_AUTO_TEST_CASE(BrickClusterAttributes)
{
    auto mc = walledRoom(8, 8);
    auto b = place<brickCluster>(mc, 0, 3, 3);
    BOOST_CHECK_EQUAL(b->getType(), bElemTypes::_brickClusterType);
    BOOST_CHECK(!b->getAttrs()->isSteppable());
    BOOST_CHECK(!b->getAttrs()->isKillable());
    BOOST_CHECK(!b->getAttrs()->isCollectible());
    BOOST_CHECK(b->getAttrs()->isMovable());
    BOOST_CHECK(b->getAttrs()->canBePushed());
    BOOST_CHECK(b->getAttrs()->isDestroyable());
    BOOST_CHECK(!b->kill());
    BOOST_CHECK(!b->hurt(1000));
    BOOST_CHECK(!b->getStats()->isDying());
    BOOST_CHECK(b->destroy());
    BOOST_CHECK(b->getStats()->isDestroying());
}

BOOST_AUTO_TEST_CASE(PlayerPushesBrickCluster)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto b = place<brickCluster>(mc, 0, 3, 2);
    BOOST_CHECK(plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK(isAt(b, 4, 2));
    BOOST_CHECK(isAt(plr, 3, 2));
    BOOST_CHECK(mc->getElement(4, 2) == b);
}

BOOST_AUTO_TEST_CASE(BrickClusterAgainstWallDoesNotMove)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 5, 2);
    auto b = place<brickCluster>(mc, 0, 6, 2); // (7,2) is the border wall
    BOOST_CHECK(!plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK(isAt(b, 6, 2));
    BOOST_CHECK(isAt(plr, 5, 2));
}

/* ---------------------------------------------------------------- explosives */

BOOST_AUTO_TEST_CASE(SimpleBombTriggersOnlyOnce)
{
    auto mc = walledRoom(8, 8);
    auto bomb = place<simpleBomb>(mc, 0, 3, 3);
    BOOST_CHECK_EQUAL(bomb->getType(), bElemTypes::_simpleBombType);
    BOOST_CHECK(bomb->getAttrs()->isKillable());
    BOOST_CHECK(bomb->getAttrs()->isDestroyable());
    BOOST_CHECK(bomb->getAttrs()->canBePushed());
    BOOST_CHECK(bomb->destroy());
    BOOST_CHECK(!bomb->destroy());
    BOOST_CHECK(!bomb->hurt(5));
    BOOST_CHECK(!bomb->kill());
    BOOST_CHECK(bomb->getStats()->isWaiting()); // the fuse burns first
    BOOST_CHECK(!bomb->getStats()->isDestroying());
}

BOOST_AUTO_TEST_CASE(BombBlastDestroysNeighboursWithinRadiusOnly)
{
    auto mc = walledRoom(12, 12);
    auto plr = activePlayerAt(mc, 1, 1);
    auto bomb = place<simpleBomb>(mc, 0, 5, 5);
    std::vector<std::shared_ptr<bElem>> near;
    for (auto p : {coords(4, 5), coords(6, 5), coords(5, 4), coords(5, 6), coords(6, 6)})
        near.push_back(place<brickCluster>(mc, 0, p.x, p.y));
    auto far = place<brickCluster>(mc, 0, 8, 5);
    auto shield = place<wall>(mc, 0, 4, 4);
    BOOST_REQUIRE(bomb->destroy());
    run(5);
    for (auto &b : near)
        BOOST_CHECK(!goneOrGoing(b)); // the fuse is still burning
    run(GoEConstants::_defaultDestroyTime + 60);
    for (auto &b : near)
        BOOST_CHECK(b->getStats()->isDisposed());
    for (auto p : {coords(4, 5), coords(6, 5), coords(5, 4), coords(5, 6), coords(6, 6), coords(5, 5)})
        BOOST_CHECK_EQUAL(typeAt(mc, p.x, p.y), bElemTypes::_floorType);
    BOOST_CHECK(bomb->getStats()->isDisposed());
    BOOST_CHECK(!goneOrGoing(far));
    BOOST_CHECK(mc->getElement(8, 5) == far);
    BOOST_CHECK(!shield->getStats()->isDestroying());
    BOOST_CHECK(mc->getElement(4, 4) == shield);
    BOOST_CHECK(!goneOrGoing(plr));
}

BOOST_AUTO_TEST_CASE(BombBlastTakesOutNearbyPlayer)
{
    auto mc = walledRoom(10, 10);
    auto plr = activePlayerAt(mc, 5, 4);
    auto bomb = place<simpleBomb>(mc, 0, 5, 5);
    BOOST_REQUIRE(bomb->kill()); // killing a bomb just lights it
    run(40);
    BOOST_CHECK(goneOrGoing(plr));
}

BOOST_AUTO_TEST_CASE(BombBlastSetsOffNeighbouringBomb)
{
    auto mc = walledRoom(12, 12);
    auto plr = activePlayerAt(mc, 1, 1);
    auto first = place<simpleBomb>(mc, 0, 3, 6);
    auto second = place<simpleBomb>(mc, 0, 4, 6);
    auto target = place<brickCluster>(mc, 0, 5, 6); // out of reach of the first bomb
    BOOST_REQUIRE(first->hurt(1));
    run(GoEConstants::_defaultDestroyTime * 2 + 80);
    BOOST_CHECK(first->getStats()->isDisposed());
    BOOST_CHECK(second->getStats()->isDisposed());
    BOOST_CHECK(target->getStats()->isDisposed());
    BOOST_CHECK(!goneOrGoing(plr));
}

/* ---------------------------------------------------------------- missiles and guns */

BOOST_AUTO_TEST_CASE(PlainMissileFliesStraightAndDiesAtWall)
{
    auto mc = walledRoom(16, 7);
    activePlayerAt(mc, 1, 1);
    auto m = elementFactory::generateAnElement<plainMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    BOOST_REQUIRE(m->stepOnElement(mc->getElement(2, 3)));
    run(30);
    BOOST_CHECK(m->getStats()->getMyPosition().x > 2);
    BOOST_CHECK_EQUAL(m->getStats()->getMyPosition().y, 3);
    run(200);
    BOOST_CHECK(m->getStats()->isDisposed());
    BOOST_CHECK_EQUAL(typeAt(mc, 15, 3), bElemTypes::_wallType);
    for (int x = 1; x < 15; x++)
        BOOST_CHECK_EQUAL(typeAt(mc, x, 3), bElemTypes::_floorType);
}

BOOST_AUTO_TEST_CASE(PlainMissileHurtsKillableTargetAndScores)
{
    auto mc = walledRoom(16, 7);
    auto shooter = activePlayerAt(mc, 1, 1);
    auto target = place<player>(mc, 0, 9, 3); // a second, inactive avatar: killable and static
    BOOST_REQUIRE(!target->getStats()->isActive());
    int before = target->getAttrs()->getEnergy();
    auto m = elementFactory::generateAnElement<plainMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    m->getStats()->setStatsOwner(shooter);
    BOOST_REQUIRE(m->stepOnElement(mc->getElement(2, 3)));
    m->getAttrs()->setEnergy(30);
    run(200);
    BOOST_CHECK(m->getStats()->isDisposed());
    BOOST_CHECK_EQUAL(target->getAttrs()->getEnergy(), before - 30);
    BOOST_CHECK(!target->getStats()->isDying());
    BOOST_CHECK(shooter->getStats()->getPoints(SHOOT) >= 1);
}

BOOST_AUTO_TEST_CASE(PlainMissileDoesNotHarmBrickCluster)
{
    auto mc = walledRoom(16, 7);
    activePlayerAt(mc, 1, 1);
    auto b = place<brickCluster>(mc, 0, 8, 3);
    auto m = elementFactory::generateAnElement<plainMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    BOOST_REQUIRE(m->stepOnElement(mc->getElement(2, 3)));
    run(200);
    BOOST_CHECK(m->getStats()->isDisposed());
    BOOST_CHECK(!goneOrGoing(b));
    BOOST_CHECK(mc->getElement(8, 3) == b);
}

BOOST_AUTO_TEST_CASE(BazookaFiresMissileWhereTheShooterFaces)
{
    auto mc = walledRoom(16, 9);
    auto plr = activePlayerAt(mc, 2, 5);
    auto bz = place<bazooka>(mc, 0, 2, 6);
    BOOST_REQUIRE(plr->collect(bz));
    BOOST_CHECK(plr->getAttrs()->getInventory()->getActiveWeapon() == bz);
    plr->getStats()->setFacing(dir::direction::RIGHT);
    BOOST_CHECK(bz->use(plr));
    auto m = mc->getElement(3, 5);
    BOOST_REQUIRE_EQUAL(m->getType(), bElemTypes::_bazookaMissileType);
    BOOST_CHECK(m->getStats()->getMyDirection() == dir::direction::RIGHT);
    BOOST_CHECK(m->getStats()->getStatsOwner().lock() == plr);
    BOOST_CHECK(!bz->use(plr)); // needs to recharge
}

BOOST_AUTO_TEST_CASE(BazookaMissileExplodesAtObstacle)
{
    auto mc = walledRoom(16, 9);
    auto plr = activePlayerAt(mc, 2, 5);
    auto bz = place<bazooka>(mc, 0, 2, 6);
    BOOST_REQUIRE(plr->collect(bz));
    auto hit = place<brickCluster>(mc, 0, 9, 5);
    auto side = place<brickCluster>(mc, 0, 8, 6); // diagonal to where the missile stops
    auto spared = place<brickCluster>(mc, 0, 11, 5);
    plr->getStats()->setFacing(dir::direction::RIGHT);
    BOOST_REQUIRE(bz->use(plr));
    auto m = mc->getElement(3, 5);
    BOOST_REQUIRE_EQUAL(m->getType(), bElemTypes::_bazookaMissileType);
    run(20);
    BOOST_CHECK(m->getStats()->getMyPosition().x > 3);
    run(GoEConstants::_defaultDestroyTime + 100);
    BOOST_CHECK(m->getStats()->isDisposed());
    BOOST_CHECK(hit->getStats()->isDisposed());
    BOOST_CHECK(side->getStats()->isDisposed());
    BOOST_CHECK(!goneOrGoing(spared));
    BOOST_CHECK(!goneOrGoing(plr));
}

BOOST_AUTO_TEST_CASE(BazookaMissileSelfDetonatesAfterMaxSteps)
{
    auto mc = walledRoom(60, 7);
    activePlayerAt(mc, 1, 1);
    auto m = elementFactory::generateAnElement<bazookaMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    BOOST_REQUIRE(m->stepOnElement(mc->getElement(2, 3)));
    m->registerLiveElement(m);
    int stop = 2 + GoEConstants::_bazookaMaxSteps - 1;
    auto inBlast = place<brickCluster>(mc, 0, stop + 1, 4);
    auto beyond = place<brickCluster>(mc, 0, stop + 4, 3);
    run(GoEConstants::_bazookaMaxSteps * (GoEConstants::_bazookaMissileSpeed + 2) + 200);
    BOOST_CHECK(m->getStats()->isDisposed());
    BOOST_CHECK(inBlast->getStats()->isDisposed());
    BOOST_CHECK(!goneOrGoing(beyond));
}

BOOST_AUTO_TEST_CASE(PointBlankBazookaMissileHurtsWithoutExploding)
{
    auto mc = walledRoom(12, 8);
    activePlayerAt(mc, 1, 1);
    auto target = place<player>(mc, 0, 4, 3);
    auto bystander = place<brickCluster>(mc, 0, 3, 4);
    int before = target->getAttrs()->getEnergy();
    auto m = elementFactory::generateAnElement<bazookaMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    BOOST_REQUIRE(m->stepOnElement(mc->getElement(3, 3)));
    m->getAttrs()->setEnergy(20);
    m->registerLiveElement(m);
    run(GoEConstants::_defaultDestroyTime + 50);
    BOOST_CHECK(m->getStats()->isDisposed());
    BOOST_CHECK_EQUAL(target->getAttrs()->getEnergy(), before - 20);
    BOOST_CHECK(!goneOrGoing(bystander));
}

/* ---------------------------------------------------------------- bunker */

BOOST_AUTO_TEST_CASE(BunkerAttributesAndInteraction)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 1, 1);
    auto bk = place<bunker>(mc, 0, 4, 4);
    BOOST_CHECK_EQUAL(bk->getType(), bElemTypes::_bunker);
    BOOST_CHECK(!bk->getAttrs()->isKillable());
    BOOST_CHECK(bk->getAttrs()->isDestroyable());
    BOOST_CHECK(!bk->getAttrs()->isSteppable());
    BOOST_CHECK(!bk->kill());
    BOOST_CHECK(bk->interact(plr));
    BOOST_CHECK(!bk->interact(plr)); // cooling down
}

BOOST_AUTO_TEST_CASE(BunkerAlignsTowardsKillableTarget)
{
    auto mc = walledRoom(12, 12);
    auto plr = activePlayerAt(mc, 2, 9);
    auto bk = place<bunker>(mc, 0, 2, 3);
    BOOST_CHECK(bk->findLongestShot() == dir::direction::DOWN);
    BOOST_CHECK(bk->selfAlign());
    BOOST_CHECK(bk->getStats()->getFacing() == dir::direction::DOWN);
}

BOOST_AUTO_TEST_CASE(BunkerShootsPlayerInLineOfSight)
{
    auto mc = walledRoom(12, 12);
    auto plr = activePlayerAt(mc, 8, 5);
    auto bk = place<bunker>(mc, 0, 2, 5);
    int before = plr->getAttrs()->getEnergy();
    bool missileSeen = false;
    for (int c = 0; c < 4000; c++) {
        bElem::runLiveElements();
        for (int x = 3; x < 8; x++)
            missileSeen |= typeAt(mc, x, 5) == bElemTypes::_plainMissile;
        if (plr->getAttrs()->getEnergy() < before || goneOrGoing(plr))
            break;
    }
    BOOST_CHECK(plr->getAttrs()->getEnergy() < before || goneOrGoing(plr));
    BOOST_CHECK(missileSeen);
    BOOST_CHECK(bk->getStats()->getFacing() == dir::direction::RIGHT);
}

BOOST_AUTO_TEST_CASE(BunkerDoesNotHitPlayerBehindWall)
{
    auto mc = walledRoom(12, 12);
    auto plr = activePlayerAt(mc, 8, 5);
    place<bunker>(mc, 0, 2, 5);
    place<wall>(mc, 0, 5, 5);
    int before = plr->getAttrs()->getEnergy();
    run(1500);
    BOOST_CHECK_EQUAL(plr->getAttrs()->getEnergy(), before);
    BOOST_CHECK(!goneOrGoing(plr));
}

/* ---------------------------------------------------------------- monster */

BOOST_AUTO_TEST_CASE(MonsterWandersAround)
{
    auto mc = walledRoom(14, 14);
    // the player hides in a walled pocket, out of the monster's sight
    place<wall>(mc, 0, 2, 1);
    place<wall>(mc, 0, 1, 2);
    auto plr = activePlayerAt(mc, 1, 1);
    auto mon = place<monster>(mc, 0, 7, 7);
    BOOST_CHECK(mon->getAttrs()->isKillable());
    BOOST_CHECK(mon->getAttrs()->isMovable());
    int moves = 0;
    coords last = mon->getStats()->getMyPosition();
    for (int c = 0; c < 600; c++) {
        bElem::runLiveElements();
        coords now = mon->getStats()->getMyPosition();
        if (now != last)
            moves++;
        last = now;
        BOOST_REQUIRE(mc->getElement(now) == mon);
    }
    BOOST_CHECK(moves >= 5);
    BOOST_CHECK(!goneOrGoing(plr));
}

BOOST_AUTO_TEST_CASE(MonsterHurtsAdjacentPlayer)
{
    auto mc = walledRoom(10, 10);
    auto plr = activePlayerAt(mc, 6, 5);
    place<monster>(mc, 0, 5, 5);
    int before = plr->getAttrs()->getEnergy();
    for (int c = 0; c < 100 && plr->getAttrs()->getEnergy() == before && !goneOrGoing(plr); c++)
        bElem::runLiveElements();
    BOOST_CHECK(plr->getAttrs()->getEnergy() < before || goneOrGoing(plr));
}

BOOST_AUTO_TEST_CASE(MonsterTurnsWhenBlocked)
{
    auto mc = walledRoom(10, 10);
    place<wall>(mc, 0, 2, 1);
    place<wall>(mc, 0, 1, 2);
    activePlayerAt(mc, 1, 1);
    auto mon = place<monster>(mc, 0, 6, 1); // facing up, straight into the border wall
    BOOST_REQUIRE(mon->getStats()->getMyDirection() == dir::direction::UP);
    run(1);
    BOOST_CHECK(isAt(mon, 6, 1));
    BOOST_CHECK(mon->getStats()->getMyDirection() == dir::direction::LEFT);
    run(GoEConstants::_mov_delay * 4);
    BOOST_CHECK(mon->getStats()->getMyPosition().x < 6);
    BOOST_CHECK_EQUAL(mon->getStats()->getMyPosition().y, 1);
}

/* ---------------------------------------------------------------- kiki and bouba */

namespace {
/// a one tile high corridor (y=1, x=1..8) with a kiki at its west end, and a separate row (y=3) for the player
std::shared_ptr<chamber> kikiCorridor(std::shared_ptr<kiki> &k)
{
    auto mc = walledRoom(10, 5);
    for (int x = 1; x < 9; x++)
        place<wall>(mc, 0, x, 2);
    k = place<kiki>(mc, 0, 1, 1);
    return mc;
}
} // namespace

BOOST_AUTO_TEST_CASE(KikiLaysBoubaRayUpToItsTerminator)
{
    std::shared_ptr<kiki> k;
    auto mc = kikiCorridor(k);
    BOOST_CHECK(k->getStats()->getMyDirection() == dir::direction::RIGHT);
    BOOST_CHECK_EQUAL(k->getAttrs()->getSubtype(), 0);
    auto term = mc->getElement(8, 1);
    BOOST_REQUIRE_EQUAL(term->getType(), bElemTypes::_kikiType);
    BOOST_CHECK_EQUAL(term->getAttrs()->getSubtype(), bElemTypes::_kikiType + 1);
    BOOST_CHECK(term->getStats()->getMyDirection() == dir::direction::LEFT);
    for (int x = 2; x < 8; x++)
        BOOST_CHECK_EQUAL(typeAt(mc, x, 1), bElemTypes::_boubaType);
    BOOST_CHECK(!k->getAttrs()->isSteppable());
    BOOST_CHECK(!k->getAttrs()->isKillable());
}

BOOST_AUTO_TEST_CASE(BoxedInKikiBecomesInert)
{
    auto mc = walledRoom(5, 5);
    auto k = place<kiki>(mc, 0, 2, 2);
    BOOST_CHECK_EQUAL(k->getAttrs()->getSubtype(), bElemTypes::_kikiType + 1);
    BOOST_CHECK(k->getStats()->getMyDirection() == dir::direction::NODIRECTION);
    for (int x = 1; x < 4; x++)
        for (int y = 1; y < 4; y++)
            if (x != 2 || y != 2)
                BOOST_CHECK_EQUAL(typeAt(mc, x, y), bElemTypes::_floorType);
}

BOOST_AUTO_TEST_CASE(BoubaIsHarmlessScenery)
{
    auto mc = walledRoom(6, 6);
    auto b = place<bouba>(mc, 0, 2, 2);
    BOOST_CHECK_EQUAL(b->getType(), bElemTypes::_boubaType);
    BOOST_CHECK(b->getAttrs()->isSteppable());
    BOOST_CHECK(!b->getAttrs()->isKillable());
    BOOST_CHECK(!b->getAttrs()->isDestroyable());
    BOOST_CHECK(!b->getStats()->hasActivatedMechanics());
}

BOOST_AUTO_TEST_CASE(BoubaIrradiatesWhoeverStandsOnIt)
{
    std::shared_ptr<kiki> k;
    auto mc = kikiCorridor(k);
    auto plr = activePlayerAt(mc, 4, 1);
    auto b = plr->getStats()->getSteppingOn();
    BOOST_REQUIRE_EQUAL(b->getType(), bElemTypes::_boubaType);
    BOOST_CHECK(b->getStats()->hasActivatedMechanics());
    int before = plr->getAttrs()->getEnergy();
    run(GoEConstants::_radioActivitySpeed * 4);
    int after = plr->getAttrs()->getEnergy();
    BOOST_CHECK(after < before);
    BOOST_CHECK_EQUAL((before - after) % GoEConstants::_radioActivityPower, 0);
    run(400);
    BOOST_CHECK(goneOrGoing(plr));
}

BOOST_AUTO_TEST_CASE(BoubaVanishesWhenLeftAndKikiRegrowsIt)
{
    std::shared_ptr<kiki> k;
    auto mc = kikiCorridor(k);
    auto plr = activePlayerAt(mc, 4, 1);
    auto oldBouba = plr->getStats()->getSteppingOn();
    BOOST_REQUIRE(plr->moveInDirection(dir::direction::RIGHT));
    BOOST_CHECK(isAt(plr, 5, 1));
    BOOST_CHECK(oldBouba->getStats()->isDisposed());
    BOOST_CHECK_EQUAL(typeAt(mc, 4, 1), bElemTypes::_floorType);
    run(kikiSpace::kikiWaitTime * 3);
    BOOST_CHECK_EQUAL(typeAt(mc, 4, 1), bElemTypes::_boubaType);
}

BOOST_AUTO_TEST_CASE(BlockingTheKikiRayClearsItUntilUnblocked)
{
    std::shared_ptr<kiki> k;
    auto mc = kikiCorridor(k);
    activePlayerAt(mc, 1, 3);
    auto block = place<brickCluster>(mc, 0, 5, 1);
    run(kikiSpace::kikiWaitTime * 3);
    for (int x = 2; x < 8; x++)
        if (x != 5)
            BOOST_CHECK_EQUAL(typeAt(mc, x, 1), bElemTypes::_floorType);
    BOOST_CHECK(mc->getElement(5, 1) == block);
    BOOST_CHECK_EQUAL(typeAt(mc, 8, 1), bElemTypes::_kikiType);
    block->disposeElement();
    run(kikiSpace::kikiWaitTime * 3);
    for (int x = 2; x < 8; x++)
        BOOST_CHECK_EQUAL(typeAt(mc, x, 1), bElemTypes::_boubaType);
}

BOOST_AUTO_TEST_SUITE_END()
