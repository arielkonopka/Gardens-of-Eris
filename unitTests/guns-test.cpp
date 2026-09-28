#include <list>
// *** END ***
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include <gtest/gtest.h>
#include "testSupport.h"
#include <memory>

/**
 * @brief PlainGun and its children test suite
 * We will perform various tests for the plainGun and its children tests.
 * - Create a gun
 * - destroy it
 * - collect a gun
 * - use it without collecting - it should create a projectible in its own direction
 * - use it collected
 *   - it should create a projectible in the direction that a collector is facing
 *   - it should add the owner to the projectible, so it could count points.
 * - test the projectible propelling movement - separate list of elements, in case of some children using different scheme, like homing missiles.
 */
auto preClean=[]()
{
    inputManager::getInstance(true);
    while (player::getActivePlayer()) player::getActivePlayer()->disposeElement();

};

using basicTestedElements = ::testing::Types<plainGun,bazooka>;
using propellingProjectibles = ::testing::Types<plainGun,bazooka>;

template <class> class GunsTests_createAndDestroyWeapon : public ::testing::Test {};
TYPED_TEST_SUITE(GunsTests_createAndDestroyWeapon, basicTestedElements);
TYPED_TEST(GunsTests_createAndDestroyWeapon, createAndDestroyWeapon)
{
    using T = TypeParam;
    coords csize= {20,20};
    std::shared_ptr<chamber> chmbr=chamber::makeNewChamber(csize);
    std::shared_ptr<T> elem=elementFactory::generateAnElement<T>(chmbr,0);
    EXPECT_TRUE(elem);
    elem->stepOnElement(chmbr->getElement(csize/2));
    EXPECT_TRUE(chmbr->getElement(csize/2)->getStats()->getInstanceId()==elem->getStats()->getInstanceId());
    elem->disposeElement();
    EXPECT_TRUE(chmbr->getElement(csize/2)->getStats()->getInstanceId()!=elem->getStats()->getInstanceId());
    elem=elementFactory::generateAnElement<T>(chmbr,0);
    EXPECT_TRUE(elem);
    elem->stepOnElement(chmbr->getElement(csize/2));
    EXPECT_TRUE(chmbr->getElement(csize/2)->getStats()->getInstanceId()==elem->getStats()->getInstanceId());
    chmbr->getElement(csize/2)->disposeElement();
    EXPECT_TRUE(chmbr->getElement(csize/2)->getStats()->getInstanceId()!=elem->getStats()->getInstanceId());
}

/**
 * @brief Test that the gun can be collected and it is really collected by the collector
 */
template <class> class GunsTests_collectAGun : public ::testing::Test {};
TYPED_TEST_SUITE(GunsTests_collectAGun, basicTestedElements);
TYPED_TEST(GunsTests_collectAGun, collectAGun)
{
    using T = TypeParam;
    coords csize= {20,20};
    coords p1= {2,2};
    coords p2= {2,3};
    std::shared_ptr<chamber> chmbr=chamber::makeNewChamber(csize);
    std::shared_ptr<T> elem=elementFactory::generateAnElement<T>(chmbr,0);
    std::shared_ptr<bElem> collector=elementFactory::generateAnElement<bElem>(chmbr,0);
    std::shared_ptr<inventory> i;
    collector->getAttrs()->setCollect(true);
    i=collector->getAttrs()->getInventory();
    collector->stepOnElement(chmbr->getElement(p1));
    elem->stepOnElement(chmbr->getElement(p2));
    collector->collect(chmbr->getElement(p2));
    EXPECT_TRUE(chmbr->getElement(p2)->getStats()->getInstanceId()!=elem->getStats()->getInstanceId());
    EXPECT_TRUE(i->getActiveWeapon()->getStats()->getInstanceId()==elem->getStats()->getInstanceId());
}

/**
 * @brief Test if the gun shoots in the right direction without being collected, like when it shoots by itself, somehow
 */
template <class> class GunsTests_useWithoutCollecting : public ::testing::Test {};
TYPED_TEST_SUITE(GunsTests_useWithoutCollecting, basicTestedElements);
TYPED_TEST(GunsTests_useWithoutCollecting, useWithoutCollecting)
{
    using T = TypeParam;
    coords csize= {20,20};
    coords p1= {5,5},p1_= {6,5};
    coords ploc= {10,10};
    dir::direction dir=dir::direction::DOWN,dir1=dir::direction::UP;
    inputManager::getInstance(true);
    std::shared_ptr<chamber> chmbr=chamber::makeNewChamber(csize);
    std::shared_ptr<T> elem=elementFactory::generateAnElement<T>(chmbr,1);
    std::shared_ptr<bElem> be=elementFactory::generateAnElement<bElem>(chmbr,0);
    preClean();
    std::shared_ptr<bElem> pl=elementFactory::generateAnElement<player>(chmbr,0);
    pl->getStats()->setActive(true);
    pl->stepOnElement(chmbr->getElement(ploc));

    be->stepOnElement(chmbr->getElement(p1_));
    be->getStats()->setFacing(dir1);
    be->getStats()->setMyDirection(dir1);
    elem->stepOnElement(chmbr->getElement(p1));
    elem->getStats()->setMyDirection(dir);
    elem->getStats()->setFacing(dir);
    EXPECT_TRUE(elem->use(be));
    EXPECT_TRUE(!elem->use(be));
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir)->getType()!=bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==bElemTypes::_floorType);
    for(int c=0; c<1000; c++) bElem::runLiveElements();
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()==bElemTypes::_floorType);
    std::cout<<"type: "<<chmbr->getElement(p1)->getElementInDirection(dir)->getType()<<"\n";
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(elem->use(be));
    EXPECT_TRUE(!elem->use(be));
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir)->getType()!=bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==bElemTypes::_floorType);
    elem->getAttrs()->setSubtype(0);
    for(int c=0; c<1000; c++) bElem::tick();
    EXPECT_TRUE(elem->use(be));
    EXPECT_TRUE(!elem->use(be));
}

/**
 * @brief we test the gun shoots in the right direction when collected and used
 * We use the gun multiple times, also simulating 1000 ticks between them, on two ways, to ensure compatibility with the rest of the engine
 */


template <class> class GunsTests_useCollectedGun : public ::testing::Test {};
TYPED_TEST_SUITE(GunsTests_useCollectedGun, basicTestedElements);
TYPED_TEST(GunsTests_useCollectedGun, useCollectedGun)
{
    using T = TypeParam;
    coords csize= {20,20};
    coords p1= {5,5},p1_= {6,5};
    coords ploc={10,10};
    dir::direction dir=dir::direction::DOWN,dir1=dir::direction::UP;
    inputManager::getInstance(true);
    preClean();
    std::shared_ptr<chamber> chmbr=chamber::makeNewChamber(csize);
    std::shared_ptr<T> elem=elementFactory::generateAnElement<T>(chmbr,0);
    std::shared_ptr<bElem> be=elementFactory::generateAnElement<bElem>(chmbr,0);
    std::shared_ptr<bElem> pl=elementFactory::generateAnElement<player>(chmbr,0);
    pl->getStats()->setActive(true);
    pl->stepOnElement(chmbr->getElement(ploc));
    be->stepOnElement(chmbr->getElement(p1_));
    be->getAttrs()->setCollect(true);
    be->getStats()->setFacing(dir1);
    be->getStats()->setMyDirection(dir1);
    elem->stepOnElement(chmbr->getElement(p1));
    elem->getStats()->setMyDirection(dir);
    elem->getStats()->setFacing(dir);
    be->collect(elem);
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(elem->use(be));
    EXPECT_TRUE(!elem->use(be));
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()!=bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir)->getType()==bElemTypes::_floorType);
    EXPECT_TRUE(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==bElemTypes::_floorType);
    for(int c=0; c<1000; c++) bElem::runLiveElements();
    for(int c=0; c<4; c++)
    {
        dir::direction d=(dir::direction)c;
        EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(d)->getType()==bElemTypes::_floorType);
    }
    EXPECT_TRUE(elem->use(be));
    EXPECT_TRUE(!elem->use(be));
    EXPECT_TRUE(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()!=bElemTypes::_floorType);
    for(int c=0; c<1000; c++) bElem::tick();
    EXPECT_TRUE(be->getAttrs()->getInventory()->getActiveWeapon()->use(be));
    EXPECT_TRUE(!be->getAttrs()->getInventory()->getActiveWeapon()->use(be));
}




