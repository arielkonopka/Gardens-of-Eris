// *** ADDED BY HEADER FIXUP ***
#include <list>
// *** END ***
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <memory>

BOOST_AUTO_TEST_SUITE(PlayerTests)

BOOST_AUTO_TEST_CASE(GetActivePlayerTest)
{
    coords point={3,3};
    inputManager::getInstance(true);
    std::shared_ptr<chamber> mc = chamber::makeNewChamber({5, 5});
    std::shared_ptr<player> plr = elementFactory::generateAnElement<player>(mc,0);
    plr->stepOnElement(mc->getElement(point));
    plr->getStats()->setActive(true);
    BOOST_CHECK(player::getActivePlayer());
    BOOST_CHECK(plr->getStats()->getInstanceId()==player::getActivePlayer()->getStats()->getInstanceId());
}

/**
 * @brief Unit test o' the `shootGun` function in the `player` class, as if written by a dwarf.
 *
 * This unit test be checkin' whether the player can properly collect a gun, use it for firin',
 * and then if the gun is rightly removed from the player's inventory once all the bullets have been spent.
 * It also checks whether, after disposing the player from the board, the location o' the player is properly replaced with rubbish.
 * Proper management of firearms is crucial for any self-respectin' dwarf, ain't it?
 */
BOOST_AUTO_TEST_CASE(PlayerShootsGun)
{
    //    bool gunDisposed=false;
    inputManager::getInstance(true);
    std::shared_ptr<chamber> mc = chamber::makeNewChamber({5, 5});
    std::shared_ptr<player> plr = elementFactory::generateAnElement<player>(mc,0);
    BOOST_CHECK(plr->getAttrs()->canCollect());
    plr->stepOnElement(mc->getElement(2, 2));
    std::shared_ptr<plainGun> pGun = elementFactory::generateAnElement<plainGun>(mc,0);
    pGun->stepOnElement(mc->getElement(3, 2));
    plr->getStats()->setActive(true);
    plr->collect(pGun);
    BOOST_CHECK(pGun->getAttrs()->isCollectible());
    BOOST_CHECK(plr->getAttrs()->getInventory()->getActiveWeapon() != nullptr);
    BOOST_CHECK(plr->getAttrs()->getInventory()->getActiveWeapon()->getStats()->getInstanceId() == pGun->getStats()->getInstanceId());
    plr->getStats()->setMyDirection(dir::direction::UP);
    for (int c = 0; c < 555; c++)
    {
        std::shared_ptr<bElem> wep = plr->getAttrs()->getInventory()->getActiveWeapon();
        if (wep != nullptr)
        {
            plr->shootGun();
        }
        for (int d = 0; d < 100; d++)
            bElem::tick();
    }
    BOOST_CHECK(plr->getAttrs()->getInventory()->getActiveWeapon() == nullptr);

    pGun = elementFactory::generateAnElement<plainGun>(mc,0);
    pGun->getAttrs()->setSubtype(0);
    pGun->stepOnElement(mc->getElement(3, 2));
    plr->collect(pGun);
    pGun = elementFactory::generateAnElement<plainGun>(mc,0);
    pGun->getAttrs()->setSubtype(0);
    pGun->stepOnElement(mc->getElement(3, 2));
    plr->collect(pGun);
    plr->disposeElement(); // here we should have the player to be removed from the board
    BOOST_CHECK(mc->getElement(2, 2)->getStats()->getInstanceId()!= plr->getStats()->getInstanceId());
    BOOST_CHECK(mc->getElement(2, 2)->getType()!= plr->getType());
    BOOST_CHECK(mc->getElement(2, 2)->getType()==bElemTypes::_rubishType);
}

/**
 * @brief Unit test fer the 'stepOnElement' an' 'disposeElement' functions in the 'player' class, given a touch o' the Welsh Valleys, isn't it?
 *
 * This test makes sure a player's stepping right onto an element, which is then disposed of, but it doesn't blow the player to smithereens, no siree.
 * Tests to see if the player is rightly removed from the original location and then placed in a new location, it does.
 * It also ensures the player ain't appearing in more than one place at the same time, because that'd be more magic than even Merlin could handle!
 */
BOOST_AUTO_TEST_CASE(PlayerStepsIntoExplodingBomb)
{
    inputManager::getInstance(true);
    std::shared_ptr<chamber> mc = chamber::makeNewChamber({10, 10});
    std::shared_ptr<player> p = nullptr;
    p = elementFactory::generateAnElement<player>(mc,0);
    std::shared_ptr<bElem> e = nullptr;
    for (int cnt = 0; cnt < 10000; cnt++)
    {
        e = mc->getElement(1, 2);
        BOOST_CHECK(e != nullptr);
        BOOST_CHECK(e != p);
        p->stepOnElement(e);
        e->disposeElement();
        p->getStats()->setActive(true);
        // p->moveInDirection(RIGHT);
        BOOST_CHECK(p->removeElement() != nullptr);

        BOOST_CHECK(mc->getElement(1, 2) != nullptr);
        BOOST_CHECK(mc->getElement(1, 2) != p);
        p->stepOnElement(mc->getElement(2, 2));

        int instances = 0;
        for (int a = 0; a < 10; a++)
        {
            for (int b = 0; b < 10; b++)
            {
                std::shared_ptr<bElem> e = mc->getElement(a, b);
                BOOST_ASSERT(e != nullptr);
                while (e != nullptr)
                {
                    if (e->getStats()->getInstanceId() == p->getStats()->getInstanceId())
                        instances++;
                    e = e->getStats()->getSteppingOn();
                }
            }
        }
        BOOST_CHECK(instances == 1);
    }
}

/**
 * @brief Bejaysus, 'tis a unit test for the activation and disposal o' player elements, top o' the mornin' to ya!
 *
 * This dandy test ensures a player can only step onto an element once, and cannot be stepping onto nothing at all, by the saints!
 * Interaction between players is also tested, so it is.
 * We be making sure that after a player interacts with another, they can't be doing it again.
 * Then we're checking that only one player can be active at a time, and that when a player meets their end, another steps up to the plate.
 * At the end of the day, when all players have been disposed of, the chamber is emptied and ready for another round, so it is!
 */

BOOST_AUTO_TEST_CASE(PlayerActivationOnPlayerDeath)
{
    inputManager::getInstance(true);
    std::vector<std::shared_ptr<chamber>> m;
    std::shared_ptr<player> p;
    std::shared_ptr<player> p1;
    std::shared_ptr<player> p0;
    std::shared_ptr<bElem> tp, tp1;
    unsigned long int iid;
    for (int c = 0; c < 10; c++)
        m.push_back(chamber::makeNewChamber({105, 10}));

    p0 = elementFactory::generateAnElement<player>(m[0],0);
    p0->stepOnElement(m[1]->getElement(0, 0));
    p0->getStats()->setActive(true);
    for (int a = 0; a < 100; a++)
    {
        // std::cout<<a<<"\n";
        p = elementFactory::generateAnElement<player>(m[0],0);
        BOOST_CHECK(p->stepOnElement(nullptr) == false);
        BOOST_CHECK(p->stepOnElement(m[a % m.size()]->getElement(a, a % 10)) == true);
        BOOST_CHECK(p->stepOnElement(m[a % m.size()]->getElement(a, a % 10)) == false);
        BOOST_CHECK(p->interact(nullptr) == false);
        BOOST_CHECK(p->interact(p0) == true);
        BOOST_CHECK(p->interact(p0) == false);
    }
    for (int c = 0; c < 100; c++)
        bElem::runLiveElements();

    tp = player::getActivePlayer();
    while (tp != nullptr)
    {
        iid = tp->getStats()->getInstanceId();
        tp->disposeElement();
        tp1 = player::getActivePlayer();
        if (p1 != nullptr)
            BOOST_CHECK(iid != p1->getStats()->getInstanceId());
        tp = tp1;
    }
    m.clear();
}

/**
 * @brief Test case to check the interaction of a player with apples and bombs.
 *
 * In this test, a chamber is created and a player, along with golden apples and simple bombs, are generated. The player
 * collects the apples and then gets destroyed by a bomb. After the player is destroyed, the stash where the apples
 * were collected is also destroyed by a bomb. The test verifies the correct number of apples at different stages and
 * ensures the objects' types are updated correctly as they interact and get destroyed.
 */
BOOST_AUTO_TEST_CASE(PlayerCollectApplesThenDestroyedByBombAndThenTheStashDestroyedWithBomb)
{
    inputManager::getInstance(true);
    std::shared_ptr<chamber> mc = chamber::makeNewChamber({10, 10});
    std::shared_ptr<goldenApple> gc = elementFactory::generateAnElement<goldenApple>(mc,0);
    std::shared_ptr<player> p = elementFactory::generateAnElement<player>(mc,0);
    std::shared_ptr<simpleBomb> sb = elementFactory::generateAnElement<simpleBomb>(mc,0);

    p->stepOnElement(mc->getElement(2, 1));
    sb->stepOnElement(mc->getElement(2, 0));
    gc->stepOnElement(mc->getElement(1, 1));
    gc = elementFactory::generateAnElement<goldenApple>(mc,0);
    gc->stepOnElement(mc->getElement(1, 2));
    p->collect(mc->getElement(1, 1));
    p->collect(mc->getElement(1, 2));
    p.reset();
    gc.reset();
    BOOST_CHECK(goldenApple::getAppleNumber() == 2);
    std::cout << goldenApple::getAppleNumber() << "\n";
    sb->hurt(5);
    // We take time for the exploded bomb to finish
    BOOST_CHECK(mc->getElement(2, 1)->getType() == bElemTypes::_player);
    for (int c = 0; c < 1000; c++)
    {
        bElem::runLiveElements();
    }
    BOOST_CHECK(goldenApple::getAppleNumber() == 2);
    BOOST_CHECK(mc->getElement(2, 1)->getType() == bElemTypes::_rubishType);
    sb = elementFactory::generateAnElement<simpleBomb>(mc,0);
    sb->stepOnElement(mc->getElement(2, 2));
    sb->kill();
    // We take time for the exploded bomb to finish
    for (int c = 0; c < 100; c++)
        bElem::runLiveElements();
    // we should have the rubbish with the apples contained, so no harm done, still two apples
    BOOST_CHECK(goldenApple::getAppleNumber() == 2);
    std::cout << "Ano: " << goldenApple::getAppleNumber() << "\n";
    mc->getElement(2, 1)->disposeElement();
    bElem::runLiveElements();
    bElem::runLiveElements();
    bElem::runLiveElements();
    BOOST_CHECK(mc->getElement(2, 1)->getType() == bElemTypes::_floorType);
    std::cout << "Ano: " << goldenApple::getAppleNumber() << "\n";
    BOOST_CHECK(goldenApple::getAppleNumber() == 2);
}

/**
 * @brief Here we are with the function 'controlPlayer'. Sounds pretty straightforward, isn't it? Let's dive in.
 *
 * Our function here takes a chamber 'mc' and a Control Item 'cntrlItm' as its inputs. Now, what's happening in the function?
 *
 * Firstly, we are getting the active player using the 'getActivePlayer' function, and then we're making sure it's not a nullptr, right?
 *
 * If it's not, then we proceed to animate all the elements for 100 clock ticks, and then we check the player's position once it's stabilised.
 *
 * We then set the control command that the player will intercept. Next, we animate the game for another 100 clock ticks, giving the player a chance to move, isn't it?
 *
 * After this, we set the control item type to -1, indicating no movement, and the direction to NODIRECTION, meaning it's undetermined.
 *
 * We then animate for another 100 clock ticks, and then check the player's final position.
 *
 * If the control item type is either 0 or 4, we check to make sure the player's initial and final positions are not the same. Otherwise, they should be the same.
 *
 * Let's move on to the control item structure, which has an integer 'type' and a 'direction'.
 * The 'type' can be anything from -1 to 7, each indicating a specific command for the player, such as move, shoot, interact, etc.
 * The 'direction' indicates the direction of the player's movement.
 */

void controlPlayer(std::shared_ptr<chamber> mc, controlItem cntrlItm)
{
    std::shared_ptr<bElem> p = player::getActivePlayer();
    BOOST_CHECK(p.get() != nullptr);
    if (!p)
        return;
    controlItem ctItem=cntrlItm;
    coords c0, c1;
    for (int c = 0; c < 100; c++)
        bElem::runLiveElements();
    c0 = p->getStats()->getMyPosition();
    inputManager::getInstance()->setControlItem( cntrlItm);
    for (int c = 0; c < 100; c++)
        bElem::runLiveElements();

    ctItem.type = -1;
    ctItem.dir = dir::direction::NODIRECTION;
    inputManager::getInstance()->setControlItem( ctItem);

    for (int c = 0; c < 100; c++)
        bElem::runLiveElements();
    c1 = p->getStats()->getMyPosition();
    if (cntrlItm.type == 0 || cntrlItm.type == 4)
        BOOST_CHECK(c0 != c1);
    else
        BOOST_CHECK(c0 == c1);
}

/**
 * @brief Check to see if our player has taken a spill, eh?
 *
 * So, this function here is all about checking if our player has, you know, kicked the bucket.
 * First thing we do is get a hold of the active player and stash their instance ID. Then, we
 * let things tick along for about 100 cycles. Afterwards, we take another gander at our active player.
 * If they're still hanging in there, we print a friendly little note to show they're still with us.
 * Finally, we make sure either the player is now null (they've croaked) or their instance ID has
 * changed (they've respawned, good on them!).
 */
void checkplayerKilled()
{
    std::shared_ptr<bElem> p = player::getActivePlayer();
    unsigned long int iid1 = p->getStats()->getInstanceId();
    for (int c = 0; c < 100; c++)
        bElem::runLiveElements();
    p = player::getActivePlayer();
    BOOST_CHECK(p == nullptr || iid1 != p->getStats()->getInstanceId());
}

/**
 * @brief Y'all, we're gonna be testing the movement of our player in all four directions: up, down, left, and right.
 *
 * This test case starts with creating a new chamber, and in case there are any active players, we'll be disposing of 'em.
 * We then create a player and a plainGun. The player steps onto a specific element and collects the plainGun. The player's
 * status is then set to active. The player is controlled to move in all four directions. If the loop counter hits 6, we're
 * gonna check if our player is killed. If so, a new player is generated, collects the plainGun, steps onto a specific
 * element, and then its status is set to active.
 */
BOOST_AUTO_TEST_CASE(MovePlayer)
{
    inputManager::getInstance(true);
    std::shared_ptr<chamber> mc = chamber::makeNewChamber({100, 100});
    coords c0, c1;
    controlItem ci;

    while (player::getActivePlayer())
    {
        player::getActivePlayer()->disposeElement();
    }
    std::shared_ptr<player> p = elementFactory::generateAnElement<player>(mc,0);
    std::shared_ptr<plainGun> pg = elementFactory::generateAnElement<plainGun>(mc,0);
    p->stepOnElement(mc->getElement(50, 50));
    p->collect(pg);
    p->getStats()->setActive(true);
    for (int c = 0; c < 7; c++)
    {
        if (c == 6)
        {
            controlItem ci2;
            ci2.type = 6;
            ci2.dir=dir::direction::NODIRECTION;
            inputManager::getInstance()->setControlItem( ci2);
            checkplayerKilled();
            p = elementFactory::generateAnElement<player>(mc,0);
            pg = elementFactory::generateAnElement<plainGun>(mc,0);
            p->collect(pg);
            p->stepOnElement(mc->getElement(61, 61));
            p->getStats()->setActive(true);
            continue;
        }
        ci.type = c;
        ci.dir = dir::direction::UP;
        controlPlayer(mc, ci);
        ci.dir = dir::direction::DOWN;
        controlPlayer(mc, ci);
        ci.dir = dir::direction::LEFT;
        controlPlayer(mc, ci);
        ci.dir = dir::direction::RIGHT;
        controlPlayer(mc, ci);
    }
}

BOOST_AUTO_TEST_SUITE_END()
