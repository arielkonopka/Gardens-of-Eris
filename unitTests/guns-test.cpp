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
BOOST_AUTO_TEST_SUITE(GunsTests)
auto preClean=[]()
{
    while (player::getActivePlayer()) player::getActivePlayer()->disposeElement();

};

typedef boost::mpl::list<plainGun,bazooka> basicTestedElements;
typedef boost::mpl::list<plainGun,bazooka> propellingProjectibles;

BOOST_AUTO_TEST_CASE_TEMPLATE( createAndDestroyWeapon, T, basicTestedElements)
{
    coords csize= {20,20};
    std::shared_ptr<chamber> chmbr=chamber::makeNewChamber(csize);
    std::shared_ptr<T> elem=elementFactory::generateAnElement<T>(chmbr,0);
    BOOST_CHECK(elem);
    elem->stepOnElement(chmbr->getElement(csize/2));
    BOOST_CHECK(chmbr->getElement(csize/2)->getStats()->getInstanceId()==elem->getStats()->getInstanceId());
    elem->disposeElement();
    BOOST_CHECK(chmbr->getElement(csize/2)->getStats()->getInstanceId()!=elem->getStats()->getInstanceId());
    elem=elementFactory::generateAnElement<T>(chmbr,0);
    BOOST_CHECK(elem);
    elem->stepOnElement(chmbr->getElement(csize/2));
    BOOST_CHECK(chmbr->getElement(csize/2)->getStats()->getInstanceId()==elem->getStats()->getInstanceId());
    chmbr->getElement(csize/2)->disposeElement();
    BOOST_CHECK(chmbr->getElement(csize/2)->getStats()->getInstanceId()!=elem->getStats()->getInstanceId());
}

/**
 * @brief Test that the gun can be collected and it is really collected by the collector
 */
BOOST_AUTO_TEST_CASE_TEMPLATE( collectAGun, T, basicTestedElements)
{
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
    BOOST_CHECK(chmbr->getElement(p2)->getStats()->getInstanceId()!=elem->getStats()->getInstanceId());
    BOOST_CHECK(i->getActiveWeapon()->getStats()->getInstanceId()==elem->getStats()->getInstanceId());
}

/**
 * @brief Test if the gun shoots in the right direction without being collected, like when it shoots by itself, somehow
 */
BOOST_AUTO_TEST_CASE_TEMPLATE( useWithoutCollecting, T, basicTestedElements)
{
    coords csize= {20,20};
    coords p1= {5,5},p1_= {6,5};
    coords ploc= {10,10};
    direction dir=DOWN,dir1=UP;
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
    BOOST_CHECK(elem->use(be));
    BOOST_CHECK(!elem->use(be));
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir)->getType()!=_floorType);
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==_floorType);
    for(int c=0; c<1000; c++) bElem::runLiveElements();
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()==_floorType);
    std::cout<<"type: "<<chmbr->getElement(p1)->getElementInDirection(dir)->getType()<<"\n";
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==_floorType);
    BOOST_CHECK(elem->use(be));
    BOOST_CHECK(!elem->use(be));
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir)->getType()!=_floorType);
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==_floorType);
    elem->getAttrs()->setSubtype(0);
    for(int c=0; c<1000; c++) bElem::tick();
    BOOST_CHECK(elem->use(be));
    BOOST_CHECK(!elem->use(be));
}

/**
 * @brief we test the gun shoots in the right direction when collected and used
 * We use the gun multiple times, also simulating 1000 ticks between them, on two ways, to ensure compatibility with the rest of the engine
 */


BOOST_AUTO_TEST_CASE_TEMPLATE( useCollectedGun, T, basicTestedElements)
{
    coords csize= {20,20};
    coords p1= {5,5},p1_= {6,5};
    coords ploc={10,10};
    direction dir=DOWN,dir1=UP;
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
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==_floorType);
    BOOST_CHECK(elem->use(be));
    BOOST_CHECK(!elem->use(be));
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()!=_floorType);
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir)->getType()==_floorType);
    BOOST_CHECK(chmbr->getElement(p1)->getElementInDirection(dir1)->getType()==_floorType);
    for(int c=0; c<1000; c++) bElem::runLiveElements();
    for(int c=0; c<4; c++)
    {
        direction d=(direction)c;
        BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(d)->getType()==_floorType);
    }
    BOOST_CHECK(elem->use(be));
    BOOST_CHECK(!elem->use(be));
    BOOST_CHECK(chmbr->getElement(p1_)->getElementInDirection(dir1)->getType()!=_floorType);
    for(int c=0; c<1000; c++) bElem::tick();
    BOOST_CHECK(be->getAttrs()->getInventory()->getActiveWeapon()->use(be));
    BOOST_CHECK(!be->getAttrs()->getInventory()->getActiveWeapon()->use(be));
}




BOOST_AUTO_TEST_SUITE_END()
