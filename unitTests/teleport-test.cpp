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


auto preClean=[](std::shared_ptr<chamber> ch,coords point)
{
    inputManager::getInstance(true);
    while (player::getActivePlayer()) player::getActivePlayer()->disposeElement();
    std::shared_ptr<bElem> pl=elementFactory::generateAnElement<player>(ch,0);
    pl->getStats()->setActive(true);
    pl->stepOnElement(ch->getElement(point));

};
BOOST_AUTO_TEST_SUITE( TeleportObjectTests )

BOOST_AUTO_TEST_CASE( TeleportAnObjectWithOneTeleport)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({5,5});
    std::shared_ptr<teleport> tel1=elementFactory::generateAnElement<teleport>(mc,0);
    std::shared_ptr<bElem> transportedE=elementFactory::generateAnElement<bElem>(mc,0);
    transportedE->stepOnElement(mc->getElement(2,3));
    //  transportedE->setActive(true);
    coords crds;
    transportedE->getStats()->setMyDirection(RIGHT);
    bElem::tick();
    bElem::tick();
    tel1->stepOnElement(mc->getElement(3,3));
    BOOST_CHECK(tel1->interact(transportedE)==true);
    std::cout<<"teleporting1:"<<mc->getElement(2,3)->getStats()->isTeleporting();
    BOOST_CHECK(mc->getElement(2,3)->getStats()->getInstanceId()!=transportedE->getStats()->getInstanceId());
    BOOST_CHECK(mc->getElement(2,3)->getStats()->isTeleporting()==true);
    crds=transportedE->getStats()->getMyPosition();
    //  std::cout<<"x:"<<crds.x<<" y:"<<crds.y<<"\n";
    BOOST_CHECK(mc->getElement(crds.x,crds.y)->getStats()->getInstanceId()==transportedE->getStats()->getInstanceId());
    BOOST_CHECK(mc->getElement(crds.x,crds.y)->getStats()->isTeleporting()==true);
    for(int a=0; a<_teleportationTime; a++)
    {
        BOOST_CHECK(mc->getElement(2,3)->getStats()->isTeleporting()==true);
        BOOST_CHECK(mc->getElement(crds.x,crds.y)->getStats()->isTeleporting()==true);
        bElem::tick();
    }
    BOOST_CHECK(mc->getElement(crds.x,crds.y)->getStats()->isTeleporting()==false);
    BOOST_CHECK(mc->getElement(2,3)->getStats()->isTeleporting()==false);
    tel1->disposeElement();

}
//We have two teleports of the same type
BOOST_AUTO_TEST_CASE(TeleportAnObjectWithTwoTeleportsOneChamber)
{
    coords crds;
    coords csize={10,10};
    coords ppoint={9,9};
    coords nc= {5,4};
    std::shared_ptr<chamber> mc=chamber::makeNewChamber(csize);
    preClean(mc,ppoint);
    std::shared_ptr<teleport> tel1=elementFactory::generateAnElement<teleport>(mc,0);
    std::shared_ptr<teleport>  tel2=elementFactory::generateAnElement<teleport>(mc,0);
    std::shared_ptr<bElem> transportEl=elementFactory::generateAnElement<bElem>(mc,0);
    transportEl->stepOnElement(mc->getElement(1,2));
    bElem::tick();
    bElem::tick();
    transportEl->getStats()->setMyDirection(RIGHT);
    tel1->stepOnElement(mc->getElement(2,2));
    tel2->stepOnElement(mc->getElement(4,4));
    tel1->interact(transportEl);
    crds=transportEl->getStats()->getMyPosition();
    BOOST_CHECK( crds==nc );
    BOOST_CHECK(transportEl->getStats()->getInstanceId()==mc->getElement(5,4)->getStats()->getInstanceId());
    BOOST_CHECK(transportEl->getStats()->isTeleporting()==true);
    for(int c=0; c<1000; c++)
        bElem::runLiveElements();
    BOOST_CHECK(transportEl->getStats()->isTeleporting()==false);
    transportEl->getStats()->setMyDirection(LEFT);
    BOOST_CHECK(tel2->interact(transportEl)==true);
    bElem::runLiveElements();
    BOOST_CHECK(transportEl->getStats()->getInstanceId()==mc->getElement(1,2)->getStats()->getInstanceId());
    BOOST_CHECK(transportEl->getStats()->isTeleporting()==true);
    tel1->disposeElement();
    tel2->disposeElement();
}

BOOST_AUTO_TEST_CASE(TeleportAnObjectWithTwoTeleportsDifferentType)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({8,8});
    coords ncrds;
    coords t1crds= {2,3};
    coords t2crds= {3,4};
    std::shared_ptr<teleport>  tel1=elementFactory::generateAnElement<teleport>(mc,0);
    std::shared_ptr<teleport>  tel2=elementFactory::generateAnElement<teleport>(mc,1);
    std::shared_ptr<bElem> _tr1=elementFactory::generateAnElement<bElem>(mc,0);
    std::shared_ptr<bElem> _tr2=elementFactory::generateAnElement<bElem>(mc,0);
    _tr1->stepOnElement(mc->getElement(2,1));
    _tr2->stepOnElement(mc->getElement(5,6));

    tel1->stepOnElement(mc->getElement(2,2));
    tel2->stepOnElement(mc->getElement(4,4));
    _tr1->getStats()->setMyDirection(DOWN);
    _tr2->getStats()->setMyDirection(LEFT);
    tel1->interact(_tr1);
    tel2->interact(_tr2);
    coords tr1c=_tr1->getStats()->getMyPosition();
    BOOST_CHECK(tr1c==t1crds);
    coords tr2c=_tr2->getStats()->getMyPosition();
    BOOST_CHECK(tr2c==t2crds);
    tel1->disposeElement();
    tel2->disposeElement();
    mc.reset();
}

BOOST_AUTO_TEST_CASE(WalkInTeleportTests)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({10,10});
    coords pointA= {3,5};
    coords pointAt= {2,5};
    coords pointB= {5,5};
    preClean(mc,{9,9});
    std::shared_ptr<bElem> tel1,tel2,transported;
    bElem::tick();
    transported=elementFactory::generateAnElement<bElem>(mc,0);
    transported->stepOnElement(mc->getElement(pointAt));
    transported->getStats()->setMyDirection(RIGHT);
    BOOST_CHECK(transported->getStats()->isTeleporting()==false);
    tel1=elementFactory::generateAnElement<teleport>(mc,1);
    tel2=elementFactory::generateAnElement<teleport>(mc,1);
    tel1->stepOnElement(mc->getElement(pointA));
    tel2->stepOnElement(mc->getElement(pointB));
    //ok now step on that teleport
    transported->stepOnElement(tel1);
    BOOST_CHECK(tel1->getStats()->hasActivatedMechanics()==true);
    BOOST_CHECK(transported->getStats()->getMyPosition()==pointA);
    BOOST_CHECK(mc->getElement(pointA)->getStats()->getInstanceId()==transported->getStats()->getInstanceId());
    for(int c=0; c<_teleportStandTime; c++)
    {
        BOOST_CHECK(transported->getStats()->isTeleporting()==false);
        bElem::runLiveElements();
    }
    BOOST_CHECK(transported->getStats()->isTeleporting()==true);
    tel1->disposeElement();
    tel2->disposeElement();
    mc.reset();
}



BOOST_AUTO_TEST_SUITE_END()
