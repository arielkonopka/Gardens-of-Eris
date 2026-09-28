#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include <gtest/gtest.h>
#include "testSupport.h"
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
    REQUIRE_IN_HELPER(e->stepOnElement(mc->getElement(x, y)));
    return e;
}

/// the new player becomes the active one, so the chamber it stands in gets its mechanics run
std::shared_ptr<player> activePlayerAt(const std::shared_ptr<chamber> &mc, int x, int y)
{
    dropActivePlayers();
    auto p = place<player>(mc, 0, x, y);
    p->getStats()->setActive(true);
    REQUIRE_IN_HELPER(player::getActivePlayer() == p);
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


/* ---------------------------------------------------------------- doors and keys */

TEST(ElementTests, KeyIsACollectibleToken)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto k = place<key>(mc, 3, 3, 2);
    EXPECT_EQ(k->getType(), bElemTypes::_key);
    EXPECT_TRUE(k->getAttrs()->isCollectible());
    EXPECT_TRUE(!k->getAttrs()->isSteppable());
    EXPECT_TRUE(plr->collect(k));
    EXPECT_TRUE(k->getStats()->isCollected());
    EXPECT_EQ(typeAt(mc, 3, 2), bElemTypes::_floorType);
    EXPECT_EQ(keysOf(plr, 3), 1);
    EXPECT_TRUE(plr->getAttrs()->getInventory()->getKey(bElemTypes::_key, 3, false) == k);
    EXPECT_TRUE(plr->getAttrs()->getInventory()->getKey(bElemTypes::_key, 4, false) == nullptr);
}

TEST(ElementTests, NewDoorIsLockedClosedAndNotSteppable)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    for (int st : {0, 1, 2}) {
        auto d = place<door>(mc, st, 3, 2);
        EXPECT_EQ(d->getType(), bElemTypes::_door);
        EXPECT_TRUE(!d->getAttrs()->isSteppable());
        EXPECT_TRUE(d->getAttrs()->isLocked());
        EXPECT_TRUE(!d->getAttrs()->isOpen());
        EXPECT_TRUE(!plr->moveInDirection(dir::direction::RIGHT));
        EXPECT_TRUE(isAt(plr, 2, 2));
        ticks(GoEConstants::_interactedTime + 1);
        d->disposeElement();
    }
}

TEST(ElementTests, DoorStaysLockedWithoutAKey)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 0, 3, 2);
    EXPECT_TRUE(!d->interact(plr));
    ticks(GoEConstants::_interactedTime + 1);
    auto notACollector = place<brickCluster>(mc, 0, 3, 3);
    EXPECT_TRUE(!d->interact(notACollector));
    EXPECT_TRUE(d->getAttrs()->isLocked());
    EXPECT_TRUE(!d->getAttrs()->isSteppable());
}

TEST(ElementTests, WrongKeySubtypeDoesNotOpenDoor)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 0, 3, 2);
    ASSERT_TRUE(plr->collect(place<key>(mc, 2, 2, 3)));
    EXPECT_TRUE(!d->interact(plr));
    EXPECT_TRUE(d->getAttrs()->isLocked());
    EXPECT_TRUE(!d->getAttrs()->isOpen());
    EXPECT_EQ(keysOf(plr, 2), 1); // the wrong key is not taken away
}

TEST(ElementTests, MatchingKeyOpensEvenDoorAndIsUsedUp)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 0, 3, 2);
    auto k = place<key>(mc, 0, 2, 3);
    ASSERT_TRUE(plr->collect(k));
    EXPECT_TRUE(d->interact(plr));
    EXPECT_TRUE(d->getAttrs()->isOpen());
    EXPECT_TRUE(!d->getAttrs()->isLocked());
    EXPECT_TRUE(d->getAttrs()->isSteppable());
    EXPECT_EQ(keysOf(plr, 0), 0);
    EXPECT_TRUE(plr->getAttrs()->getInventory()->getKey(bElemTypes::_key, 0, false) == nullptr);
    EXPECT_TRUE(k->getStats()->isDisposed());
    // the way is free now
    EXPECT_TRUE(plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_TRUE(isAt(plr, 3, 2));
    EXPECT_TRUE(!plr->getStats()->isDying());
}

TEST(ElementTests, UnlockedDoorTogglesOnInteractionAfterCooldown)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 0, 3, 2);
    ASSERT_TRUE(plr->collect(place<key>(mc, 0, 2, 3)));
    ASSERT_TRUE(d->interact(plr));
    EXPECT_TRUE(!d->interact(plr)); // still busy with the previous interaction
    EXPECT_TRUE(d->getAttrs()->isOpen());
    ticks(GoEConstants::_interactedTime + 1);
    EXPECT_TRUE(d->interact(plr)); // no key needed any more
    EXPECT_TRUE(!d->getAttrs()->isOpen());
    EXPECT_TRUE(!d->getAttrs()->isSteppable());
    EXPECT_TRUE(!d->getAttrs()->isLocked());
    ticks(GoEConstants::_interactedTime + 1);
    EXPECT_TRUE(d->interact(plr));
    EXPECT_TRUE(d->getAttrs()->isOpen());
    EXPECT_TRUE(d->getAttrs()->isSteppable());
}

TEST(ElementTests, PlayerWalkingIntoLockedDoorWithKeyOpensIt)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 4, 3, 2);
    ASSERT_TRUE(plr->collect(place<key>(mc, 4, 2, 3)));
    EXPECT_TRUE(plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_TRUE(d->getAttrs()->isOpen());
    EXPECT_TRUE(isAt(plr, 2, 2)); // opening the door takes the move
    EXPECT_EQ(keysOf(plr, 4), 0);
}

TEST(ElementTests, OddDoorKeepsKeyWhenOpenedAndTakesItOnPassage)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 1, 3, 2);
    ASSERT_TRUE(plr->collect(place<key>(mc, 1, 2, 3)));
    EXPECT_TRUE(d->interact(plr));
    EXPECT_TRUE(d->getAttrs()->isOpen());
    EXPECT_TRUE(d->getAttrs()->isSteppable());
    EXPECT_EQ(keysOf(plr, 1), 1);
    EXPECT_TRUE(plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_TRUE(isAt(plr, 3, 2));
    EXPECT_TRUE(!plr->getStats()->isDying());
    EXPECT_EQ(keysOf(plr, 1), 0);
    // leaving the door locks it behind the player
    ticks(GoEConstants::_mov_delay + 1);
    EXPECT_TRUE(plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_TRUE(isAt(plr, 4, 2));
    EXPECT_TRUE(!d->getAttrs()->isOpen());
    EXPECT_TRUE(d->getAttrs()->isLocked());
    EXPECT_TRUE(!d->getAttrs()->isSteppable());
}

TEST(ElementTests, OddDoorKillsIntruderWithoutAKey)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto d = place<door>(mc, 1, 3, 2);
    auto opener = place<bElem>(mc, 0, 3, 3);
    opener->getAttrs()->setCollect(true);
    ASSERT_TRUE(opener->collect(place<key>(mc, 1, 4, 4)));
    ASSERT_TRUE(d->interact(opener));
    ASSERT_TRUE(d->getAttrs()->isSteppable());
    EXPECT_TRUE(plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_TRUE(plr->getStats()->isDying());
}

TEST(ElementTests, DestroyedDoorsCrumbleOrTurnIntoWalls)
{
    auto mc = walledRoom(8, 8);
    auto odd = place<door>(mc, 1, 3, 3);
    EXPECT_TRUE(odd->destroy());
    EXPECT_EQ(typeAt(mc, 3, 3), bElemTypes::_wallType);
    EXPECT_TRUE(odd->getStats()->isDisposed());
    auto even = place<door>(mc, 0, 5, 5);
    EXPECT_TRUE(even->destroy());
    EXPECT_TRUE(even->getStats()->isDestroying());
    EXPECT_EQ(typeAt(mc, 5, 5), bElemTypes::_door);
}

/* ---------------------------------------------------------------- scenery */

TEST(ElementTests, FloorAndWallBasicAttributes)
{
    auto mc = walledRoom(6, 6);
    auto fl = mc->getElement(2, 2);
    EXPECT_EQ(fl->getType(), bElemTypes::_floorType);
    EXPECT_TRUE(fl->getAttrs()->isSteppable());
    EXPECT_TRUE(!fl->getAttrs()->isCollectible());
    EXPECT_TRUE(!fl->getAttrs()->isKillable());
    EXPECT_TRUE(!fl->getAttrs()->isDestroyable());
    auto w = mc->getElement(0, 2);
    EXPECT_EQ(w->getType(), bElemTypes::_wallType);
    EXPECT_TRUE(!w->getAttrs()->isSteppable());
    EXPECT_TRUE(!w->getAttrs()->isDestroyable());
    EXPECT_TRUE(!w->getAttrs()->isKillable());
    EXPECT_TRUE(!w->getAttrs()->isMovable());
    EXPECT_TRUE(!w->destroy());
    EXPECT_TRUE(!w->kill());
    EXPECT_TRUE(!w->hurt(1000));
    EXPECT_TRUE(!w->getStats()->isDestroying());
    EXPECT_TRUE(!mc->getElement(1, 1)->isSteppableDirection(dir::direction::UP));
    EXPECT_TRUE(mc->getElement(1, 1)->isSteppableDirection(dir::direction::DOWN));
}

TEST(ElementTests, PlayerWalkingOverRubbishPicksItUp)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto r = place<rubbish>(mc, 0, 3, 2);
    EXPECT_TRUE(r->getAttrs()->isSteppable());
    EXPECT_TRUE(r->getAttrs()->isCollectible());
    EXPECT_TRUE(r->getAttrs()->isKillable());
    EXPECT_TRUE(plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_TRUE(isAt(plr, 3, 2));
    EXPECT_TRUE(r->getStats()->isCollected());
    EXPECT_TRUE(plr->getStats()->getSteppingOn()->getType() == bElemTypes::_floorType);
}

TEST(ElementTests, RubbishStashHandsItsInventoryOver)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto r = elementFactory::generateAnElement<rubbish>(mc, 0);
    r->getAttrs()->setCollect(true);
    ASSERT_TRUE(r->collect(place<key>(mc, 7, 5, 5)));
    ASSERT_TRUE(r->stepOnElement(mc->getElement(3, 2)));
    EXPECT_TRUE(plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_EQ(keysOf(plr, 7), 1);
    EXPECT_TRUE(plr->getAttrs()->getInventory()->getKey(bElemTypes::_key, 7, false) != nullptr);
}

TEST(ElementTests, BrickClusterAttributes)
{
    auto mc = walledRoom(8, 8);
    auto b = place<brickCluster>(mc, 0, 3, 3);
    EXPECT_EQ(b->getType(), bElemTypes::_brickClusterType);
    EXPECT_TRUE(!b->getAttrs()->isSteppable());
    EXPECT_TRUE(!b->getAttrs()->isKillable());
    EXPECT_TRUE(!b->getAttrs()->isCollectible());
    EXPECT_TRUE(b->getAttrs()->isMovable());
    EXPECT_TRUE(b->getAttrs()->canBePushed());
    EXPECT_TRUE(b->getAttrs()->isDestroyable());
    EXPECT_TRUE(!b->kill());
    EXPECT_TRUE(!b->hurt(1000));
    EXPECT_TRUE(!b->getStats()->isDying());
    EXPECT_TRUE(b->destroy());
    EXPECT_TRUE(b->getStats()->isDestroying());
}

TEST(ElementTests, PlayerPushesBrickCluster)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 2, 2);
    auto b = place<brickCluster>(mc, 0, 3, 2);
    EXPECT_TRUE(plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_TRUE(isAt(b, 4, 2));
    EXPECT_TRUE(isAt(plr, 3, 2));
    EXPECT_TRUE(mc->getElement(4, 2) == b);
}

TEST(ElementTests, BrickClusterAgainstWallDoesNotMove)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 5, 2);
    auto b = place<brickCluster>(mc, 0, 6, 2); // (7,2) is the border wall
    EXPECT_TRUE(!plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_TRUE(isAt(b, 6, 2));
    EXPECT_TRUE(isAt(plr, 5, 2));
}

/* ---------------------------------------------------------------- explosives */

TEST(ElementTests, SimpleBombTriggersOnlyOnce)
{
    auto mc = walledRoom(8, 8);
    auto bomb = place<simpleBomb>(mc, 0, 3, 3);
    EXPECT_EQ(bomb->getType(), bElemTypes::_simpleBombType);
    EXPECT_TRUE(bomb->getAttrs()->isKillable());
    EXPECT_TRUE(bomb->getAttrs()->isDestroyable());
    EXPECT_TRUE(bomb->getAttrs()->canBePushed());
    EXPECT_TRUE(bomb->destroy());
    EXPECT_TRUE(!bomb->destroy());
    EXPECT_TRUE(!bomb->hurt(5));
    EXPECT_TRUE(!bomb->kill());
    EXPECT_TRUE(bomb->getStats()->isWaiting()); // the fuse burns first
    EXPECT_TRUE(!bomb->getStats()->isDestroying());
}

TEST(ElementTests, BombBlastDestroysNeighboursWithinRadiusOnly)
{
    auto mc = walledRoom(12, 12);
    auto plr = activePlayerAt(mc, 1, 1);
    auto bomb = place<simpleBomb>(mc, 0, 5, 5);
    std::vector<std::shared_ptr<bElem>> near;
    for (auto p : {coords(4, 5), coords(6, 5), coords(5, 4), coords(5, 6), coords(6, 6)})
        near.push_back(place<brickCluster>(mc, 0, p.x, p.y));
    auto far = place<brickCluster>(mc, 0, 8, 5);
    auto shield = place<wall>(mc, 0, 4, 4);
    ASSERT_TRUE(bomb->destroy());
    run(5);
    for (auto &b : near)
        EXPECT_TRUE(!goneOrGoing(b)); // the fuse is still burning
    run(GoEConstants::_defaultDestroyTime + 60);
    for (auto &b : near)
        EXPECT_TRUE(b->getStats()->isDisposed());
    for (auto p : {coords(4, 5), coords(6, 5), coords(5, 4), coords(5, 6), coords(6, 6), coords(5, 5)})
        EXPECT_EQ(typeAt(mc, p.x, p.y), bElemTypes::_floorType);
    EXPECT_TRUE(bomb->getStats()->isDisposed());
    EXPECT_TRUE(!goneOrGoing(far));
    EXPECT_TRUE(mc->getElement(8, 5) == far);
    EXPECT_TRUE(!shield->getStats()->isDestroying());
    EXPECT_TRUE(mc->getElement(4, 4) == shield);
    EXPECT_TRUE(!goneOrGoing(plr));
}

TEST(ElementTests, BombBlastTakesOutNearbyPlayer)
{
    auto mc = walledRoom(10, 10);
    auto plr = activePlayerAt(mc, 5, 4);
    auto bomb = place<simpleBomb>(mc, 0, 5, 5);
    ASSERT_TRUE(bomb->kill()); // killing a bomb just lights it
    run(40);
    EXPECT_TRUE(goneOrGoing(plr));
}

TEST(ElementTests, BombBlastSetsOffNeighbouringBomb)
{
    auto mc = walledRoom(12, 12);
    auto plr = activePlayerAt(mc, 1, 1);
    auto first = place<simpleBomb>(mc, 0, 3, 6);
    auto second = place<simpleBomb>(mc, 0, 4, 6);
    auto target = place<brickCluster>(mc, 0, 5, 6); // out of reach of the first bomb
    ASSERT_TRUE(first->hurt(1));
    run(GoEConstants::_defaultDestroyTime * 2 + 80);
    EXPECT_TRUE(first->getStats()->isDisposed());
    EXPECT_TRUE(second->getStats()->isDisposed());
    EXPECT_TRUE(target->getStats()->isDisposed());
    EXPECT_TRUE(!goneOrGoing(plr));
}

/* ---------------------------------------------------------------- missiles and guns */

TEST(ElementTests, PlainMissileFliesStraightAndDiesAtWall)
{
    auto mc = walledRoom(16, 7);
    activePlayerAt(mc, 1, 1);
    auto m = elementFactory::generateAnElement<plainMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    ASSERT_TRUE(m->stepOnElement(mc->getElement(2, 3)));
    run(30);
    EXPECT_TRUE(m->getStats()->getMyPosition().x > 2);
    EXPECT_EQ(m->getStats()->getMyPosition().y, 3);
    run(200);
    EXPECT_TRUE(m->getStats()->isDisposed());
    EXPECT_EQ(typeAt(mc, 15, 3), bElemTypes::_wallType);
    for (int x = 1; x < 15; x++)
        EXPECT_EQ(typeAt(mc, x, 3), bElemTypes::_floorType);
}

TEST(ElementTests, PlainMissileHurtsKillableTargetAndScores)
{
    auto mc = walledRoom(16, 7);
    auto shooter = activePlayerAt(mc, 1, 1);
    auto target = place<player>(mc, 0, 9, 3); // a second, inactive avatar: killable and static
    ASSERT_TRUE(!target->getStats()->isActive());
    int before = target->getAttrs()->getEnergy();
    auto m = elementFactory::generateAnElement<plainMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    m->getStats()->setStatsOwner(shooter);
    ASSERT_TRUE(m->stepOnElement(mc->getElement(2, 3)));
    m->getAttrs()->setEnergy(30);
    run(200);
    EXPECT_TRUE(m->getStats()->isDisposed());
    EXPECT_EQ(target->getAttrs()->getEnergy(), before - 30);
    EXPECT_TRUE(!target->getStats()->isDying());
    EXPECT_TRUE(shooter->getStats()->getPoints(SHOOT) >= 1);
}

TEST(ElementTests, PlainMissileDoesNotHarmBrickCluster)
{
    auto mc = walledRoom(16, 7);
    activePlayerAt(mc, 1, 1);
    auto b = place<brickCluster>(mc, 0, 8, 3);
    auto m = elementFactory::generateAnElement<plainMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    ASSERT_TRUE(m->stepOnElement(mc->getElement(2, 3)));
    run(200);
    EXPECT_TRUE(m->getStats()->isDisposed());
    EXPECT_TRUE(!goneOrGoing(b));
    EXPECT_TRUE(mc->getElement(8, 3) == b);
}

TEST(ElementTests, BazookaFiresMissileWhereTheShooterFaces)
{
    auto mc = walledRoom(16, 9);
    auto plr = activePlayerAt(mc, 2, 5);
    auto bz = place<bazooka>(mc, 0, 2, 6);
    ASSERT_TRUE(plr->collect(bz));
    EXPECT_TRUE(plr->getAttrs()->getInventory()->getActiveWeapon() == bz);
    plr->getStats()->setFacing(dir::direction::RIGHT);
    EXPECT_TRUE(bz->use(plr));
    auto m = mc->getElement(3, 5);
    ASSERT_EQ(m->getType(), bElemTypes::_bazookaMissileType);
    EXPECT_TRUE(m->getStats()->getMyDirection() == dir::direction::RIGHT);
    EXPECT_TRUE(m->getStats()->getStatsOwner().lock() == plr);
    EXPECT_TRUE(!bz->use(plr)); // needs to recharge
}

TEST(ElementTests, BazookaMissileExplodesAtObstacle)
{
    auto mc = walledRoom(16, 9);
    auto plr = activePlayerAt(mc, 2, 5);
    auto bz = place<bazooka>(mc, 0, 2, 6);
    ASSERT_TRUE(plr->collect(bz));
    auto hit = place<brickCluster>(mc, 0, 9, 5);
    auto side = place<brickCluster>(mc, 0, 8, 6); // diagonal to where the missile stops
    auto spared = place<brickCluster>(mc, 0, 11, 5);
    plr->getStats()->setFacing(dir::direction::RIGHT);
    ASSERT_TRUE(bz->use(plr));
    auto m = mc->getElement(3, 5);
    ASSERT_EQ(m->getType(), bElemTypes::_bazookaMissileType);
    run(20);
    EXPECT_TRUE(m->getStats()->getMyPosition().x > 3);
    run(GoEConstants::_defaultDestroyTime + 100);
    EXPECT_TRUE(m->getStats()->isDisposed());
    EXPECT_TRUE(hit->getStats()->isDisposed());
    EXPECT_TRUE(side->getStats()->isDisposed());
    EXPECT_TRUE(!goneOrGoing(spared));
    EXPECT_TRUE(!goneOrGoing(plr));
}

TEST(ElementTests, BazookaMissileSelfDetonatesAfterMaxSteps)
{
    auto mc = walledRoom(60, 7);
    activePlayerAt(mc, 1, 1);
    auto m = elementFactory::generateAnElement<bazookaMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    ASSERT_TRUE(m->stepOnElement(mc->getElement(2, 3)));
    m->registerLiveElement(m);
    int stop = 2 + GoEConstants::_bazookaMaxSteps - 1;
    auto inBlast = place<brickCluster>(mc, 0, stop + 1, 4);
    auto beyond = place<brickCluster>(mc, 0, stop + 4, 3);
    run(GoEConstants::_bazookaMaxSteps * (GoEConstants::_bazookaMissileSpeed + 2) + 200);
    EXPECT_TRUE(m->getStats()->isDisposed());
    EXPECT_TRUE(inBlast->getStats()->isDisposed());
    EXPECT_TRUE(!goneOrGoing(beyond));
}

TEST(ElementTests, PointBlankBazookaMissileHurtsWithoutExploding)
{
    auto mc = walledRoom(12, 8);
    activePlayerAt(mc, 1, 1);
    auto target = place<player>(mc, 0, 4, 3);
    auto bystander = place<brickCluster>(mc, 0, 3, 4);
    int before = target->getAttrs()->getEnergy();
    auto m = elementFactory::generateAnElement<bazookaMissile>(mc, 0);
    m->getStats()->setMyDirection(dir::direction::RIGHT);
    ASSERT_TRUE(m->stepOnElement(mc->getElement(3, 3)));
    m->getAttrs()->setEnergy(20);
    m->registerLiveElement(m);
    run(GoEConstants::_defaultDestroyTime + 50);
    EXPECT_TRUE(m->getStats()->isDisposed());
    EXPECT_EQ(target->getAttrs()->getEnergy(), before - 20);
    EXPECT_TRUE(!goneOrGoing(bystander));
}

/* ---------------------------------------------------------------- bunker */

TEST(ElementTests, BunkerAttributesAndInteraction)
{
    auto mc = walledRoom(8, 8);
    auto plr = activePlayerAt(mc, 1, 1);
    auto bk = place<bunker>(mc, 0, 4, 4);
    EXPECT_EQ(bk->getType(), bElemTypes::_bunker);
    EXPECT_TRUE(!bk->getAttrs()->isKillable());
    EXPECT_TRUE(bk->getAttrs()->isDestroyable());
    EXPECT_TRUE(!bk->getAttrs()->isSteppable());
    EXPECT_TRUE(!bk->kill());
    EXPECT_TRUE(bk->interact(plr));
    EXPECT_TRUE(!bk->interact(plr)); // cooling down
}

TEST(ElementTests, BunkerAlignsTowardsKillableTarget)
{
    auto mc = walledRoom(12, 12);
    auto plr = activePlayerAt(mc, 2, 9);
    auto bk = place<bunker>(mc, 0, 2, 3);
    EXPECT_TRUE(bk->findLongestShot() == dir::direction::DOWN);
    EXPECT_TRUE(bk->selfAlign());
    EXPECT_TRUE(bk->getStats()->getFacing() == dir::direction::DOWN);
}

TEST(ElementTests, BunkerShootsPlayerInLineOfSight)
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
    EXPECT_TRUE(plr->getAttrs()->getEnergy() < before || goneOrGoing(plr));
    EXPECT_TRUE(missileSeen);
    EXPECT_TRUE(bk->getStats()->getFacing() == dir::direction::RIGHT);
}

TEST(ElementTests, BunkerDoesNotHitPlayerBehindWall)
{
    auto mc = walledRoom(12, 12);
    auto plr = activePlayerAt(mc, 8, 5);
    place<bunker>(mc, 0, 2, 5);
    place<wall>(mc, 0, 5, 5);
    int before = plr->getAttrs()->getEnergy();
    run(1500);
    EXPECT_EQ(plr->getAttrs()->getEnergy(), before);
    EXPECT_TRUE(!goneOrGoing(plr));
}

/* ---------------------------------------------------------------- monster */

TEST(ElementTests, MonsterWandersAround)
{
    auto mc = walledRoom(14, 14);
    // the player hides in a walled pocket, out of the monster's sight
    place<wall>(mc, 0, 2, 1);
    place<wall>(mc, 0, 1, 2);
    auto plr = activePlayerAt(mc, 1, 1);
    auto mon = place<monster>(mc, 0, 7, 7);
    EXPECT_TRUE(mon->getAttrs()->isKillable());
    EXPECT_TRUE(mon->getAttrs()->isMovable());
    int moves = 0;
    coords last = mon->getStats()->getMyPosition();
    for (int c = 0; c < 600; c++) {
        bElem::runLiveElements();
        coords now = mon->getStats()->getMyPosition();
        if (now != last)
            moves++;
        last = now;
        ASSERT_TRUE(mc->getElement(now) == mon);
    }
    EXPECT_TRUE(moves >= 5);
    EXPECT_TRUE(!goneOrGoing(plr));
}

TEST(ElementTests, MonsterHurtsAdjacentPlayer)
{
    auto mc = walledRoom(10, 10);
    auto plr = activePlayerAt(mc, 6, 5);
    place<monster>(mc, 0, 5, 5);
    int before = plr->getAttrs()->getEnergy();
    for (int c = 0; c < 100 && plr->getAttrs()->getEnergy() == before && !goneOrGoing(plr); c++)
        bElem::runLiveElements();
    EXPECT_TRUE(plr->getAttrs()->getEnergy() < before || goneOrGoing(plr));
}

TEST(ElementTests, MonsterTurnsWhenBlocked)
{
    auto mc = walledRoom(10, 10);
    place<wall>(mc, 0, 2, 1);
    place<wall>(mc, 0, 1, 2);
    activePlayerAt(mc, 1, 1);
    auto mon = place<monster>(mc, 0, 6, 1); // facing up, straight into the border wall
    ASSERT_TRUE(mon->getStats()->getMyDirection() == dir::direction::UP);
    run(1);
    EXPECT_TRUE(isAt(mon, 6, 1));
    EXPECT_TRUE(mon->getStats()->getMyDirection() == dir::direction::LEFT);
    run(GoEConstants::_mov_delay * 4);
    EXPECT_TRUE(mon->getStats()->getMyPosition().x < 6);
    EXPECT_EQ(mon->getStats()->getMyPosition().y, 1);
}

TEST(ElementTests, CollectingIsReadFromTheConfig)
{
    // skins.json marks players, monsters, drones and bunkers with canCollect
    auto mc = walledRoom(10, 10);
    activePlayerAt(mc, 1, 1);
    EXPECT_TRUE(place<monster>(mc, 0, 5, 5)->getAttrs()->canCollect());
    EXPECT_TRUE(place<bunker>(mc, 0, 7, 7)->getAttrs()->canCollect());
    EXPECT_TRUE(!place<brickCluster>(mc, 0, 3, 3)->getAttrs()->canCollect());
}

TEST(ElementTests, MonsterPicksUpKeyInItsWay)
{
    auto mc = walledRoom(12, 5);
    // a dead-end corridor along y == 2, the player sealed off in a corner
    place<wall>(mc, 0, 3, 2);
    for (int x = 3; x < 11; x++) {
        place<wall>(mc, 0, x, 1);
        place<wall>(mc, 0, x, 3);
    }
    place<wall>(mc, 0, 2, 1);
    place<wall>(mc, 0, 1, 2);
    activePlayerAt(mc, 1, 1);
    auto mon = place<monster>(mc, 0, 4, 2);
    auto k = place<key>(mc, 3, 7, 2);
    for (int c = 0; c < 2000 && !k->getStats()->isCollected(); c++)
        bElem::runLiveElements();
    ASSERT_TRUE(k->getStats()->isCollected());
    EXPECT_TRUE(k->getStats()->getCollector().lock() == mon);
    EXPECT_EQ(mon->getAttrs()->getInventory()->countTokens(bElemTypes::_key, 3), 1);
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

TEST(ElementTests, KikiLaysBoubaRayUpToItsTerminator)
{
    std::shared_ptr<kiki> k;
    auto mc = kikiCorridor(k);
    EXPECT_TRUE(k->getStats()->getMyDirection() == dir::direction::RIGHT);
    EXPECT_EQ(k->getAttrs()->getSubtype(), 0);
    auto term = mc->getElement(8, 1);
    ASSERT_EQ(term->getType(), bElemTypes::_kikiType);
    EXPECT_EQ(term->getAttrs()->getSubtype(), bElemTypes::_kikiType + 1);
    EXPECT_TRUE(term->getStats()->getMyDirection() == dir::direction::LEFT);
    for (int x = 2; x < 8; x++)
        EXPECT_EQ(typeAt(mc, x, 1), bElemTypes::_boubaType);
    EXPECT_TRUE(!k->getAttrs()->isSteppable());
    EXPECT_TRUE(!k->getAttrs()->isKillable());
}

TEST(ElementTests, BoxedInKikiBecomesInert)
{
    auto mc = walledRoom(5, 5);
    auto k = place<kiki>(mc, 0, 2, 2);
    EXPECT_EQ(k->getAttrs()->getSubtype(), bElemTypes::_kikiType + 1);
    EXPECT_TRUE(k->getStats()->getMyDirection() == dir::direction::NODIRECTION);
    for (int x = 1; x < 4; x++)
        for (int y = 1; y < 4; y++)
            if (x != 2 || y != 2)
                EXPECT_EQ(typeAt(mc, x, y), bElemTypes::_floorType);
}

TEST(ElementTests, BoubaIsHarmlessScenery)
{
    auto mc = walledRoom(6, 6);
    auto b = place<bouba>(mc, 0, 2, 2);
    EXPECT_EQ(b->getType(), bElemTypes::_boubaType);
    EXPECT_TRUE(b->getAttrs()->isSteppable());
    EXPECT_TRUE(!b->getAttrs()->isKillable());
    EXPECT_TRUE(!b->getAttrs()->isDestroyable());
    EXPECT_TRUE(!b->getStats()->hasActivatedMechanics());
}

TEST(ElementTests, BoubaIrradiatesWhoeverStandsOnIt)
{
    std::shared_ptr<kiki> k;
    auto mc = kikiCorridor(k);
    auto plr = activePlayerAt(mc, 4, 1);
    auto b = plr->getStats()->getSteppingOn();
    ASSERT_EQ(b->getType(), bElemTypes::_boubaType);
    EXPECT_TRUE(b->getStats()->hasActivatedMechanics());
    int before = plr->getAttrs()->getEnergy();
    run(GoEConstants::_radioActivitySpeed * 4);
    int after = plr->getAttrs()->getEnergy();
    EXPECT_TRUE(after < before);
    EXPECT_EQ((before - after) % GoEConstants::_radioActivityPower, 0);
    run(400);
    EXPECT_TRUE(goneOrGoing(plr));
}

TEST(ElementTests, BoubaVanishesWhenLeftAndKikiRegrowsIt)
{
    std::shared_ptr<kiki> k;
    auto mc = kikiCorridor(k);
    auto plr = activePlayerAt(mc, 4, 1);
    auto oldBouba = plr->getStats()->getSteppingOn();
    ASSERT_TRUE(plr->moveInDirection(dir::direction::RIGHT));
    EXPECT_TRUE(isAt(plr, 5, 1));
    EXPECT_TRUE(oldBouba->getStats()->isDisposed());
    EXPECT_EQ(typeAt(mc, 4, 1), bElemTypes::_floorType);
    run(kikiSpace::kikiWaitTime * 3);
    EXPECT_EQ(typeAt(mc, 4, 1), bElemTypes::_boubaType);
}

TEST(ElementTests, BlockingTheKikiRayClearsItUntilUnblocked)
{
    std::shared_ptr<kiki> k;
    auto mc = kikiCorridor(k);
    activePlayerAt(mc, 1, 3);
    auto block = place<brickCluster>(mc, 0, 5, 1);
    run(kikiSpace::kikiWaitTime * 3);
    for (int x = 2; x < 8; x++)
        if (x != 5)
            EXPECT_EQ(typeAt(mc, x, 1), bElemTypes::_floorType);
    EXPECT_TRUE(mc->getElement(5, 1) == block);
    EXPECT_EQ(typeAt(mc, 8, 1), bElemTypes::_kikiType);
    block->disposeElement();
    run(kikiSpace::kikiWaitTime * 3);
    for (int x = 2; x < 8; x++)
        EXPECT_EQ(typeAt(mc, x, 1), bElemTypes::_boubaType);
}

