#include "elements.h"
#include "commons.h"
#include "chamber.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <memory>

BOOST_AUTO_TEST_SUITE( TeleportObjectTests )

BOOST_AUTO_TEST_CASE( TeleportAnObjectWithOneTeleport)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({5,5});
    std::shared_ptr<teleport> tel1=elementFactory::generateAnElement<teleport>(mc,0);
    std::shared_ptr<bElem> transportedE=elementFactory::generateAnElement<bElem>(mc,0);
    transportedE->stepOnElement(mc->getElement(2,3));
    //  transportedE->setActive(true);
    coords crds;
    transportedE->status->setMyDirection(RIGHT);
    bElem::tick();
    bElem::tick();
    tel1->stepOnElement(mc->getElement(3,3));
    BOOST_CHECK(tel1->interact(transportedE)==true);
    // bElem::tick();
    std::cout<<"teleporting1:"<<mc->getElement(2,3)->status->isTeleporting();
    BOOST_CHECK(mc->getElement(2,3)->status->getInstanceId()!=transportedE->status->getInstanceId());
    BOOST_CHECK(mc->getElement(2,3)->status->isTeleporting()==true);
    crds=transportedE->status->getMyPosition();
    //  std::cout<<"x:"<<crds.x<<" y:"<<crds.y<<"\n";
    BOOST_CHECK(mc->getElement(crds.x,crds.y)->status->getInstanceId()==transportedE->status->getInstanceId());
    BOOST_CHECK(mc->getElement(crds.x,crds.y)->status->isTeleporting()==true);
    for(int a=0; a<_teleportationTime; a++)
    {
        BOOST_CHECK(mc->getElement(2,3)->status->isTeleporting()==true);
        BOOST_CHECK(mc->getElement(crds.x,crds.y)->status->isTeleporting()==true);
        bElem::tick();
    }
    BOOST_CHECK(mc->getElement(crds.x,crds.y)->status->isTeleporting()==false);
    BOOST_CHECK(mc->getElement(2,3)->status->isTeleporting()==false);
}
//We have two teleports of the same type
BOOST_AUTO_TEST_CASE(TeleportAnObjectWithTwoTeleportsOneChamber)
{
    coords crds;
    coords nc= {5,4};
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({6,6});
    std::shared_ptr<teleport> tel1=elementFactory::generateAnElement<teleport>(mc,0);
    std::shared_ptr<teleport>  tel2=elementFactory::generateAnElement<teleport>(mc,0);
    std::shared_ptr<bElem> transportEl=elementFactory::generateAnElement<bElem>(mc,0);
    transportEl->stepOnElement(mc->getElement(1,2));
    bElem::tick();
    bElem::tick();
    transportEl->status->setMyDirection(RIGHT);
    tel1->stepOnElement(mc->getElement(2,2));
    tel2->stepOnElement(mc->getElement(4,4));
    tel1->interact(transportEl);
    crds=transportEl->status->getMyPosition();
    BOOST_CHECK( crds==nc );
    BOOST_CHECK(transportEl->status->getInstanceId()==mc->getElement(5,4)->status->getInstanceId());
    BOOST_CHECK(transportEl->status->isTeleporting()==true);
    for(int c=0; c<_teleportationTime+1; c++)
        bElem::tick();
    BOOST_CHECK(transportEl->status->isTeleporting()==false);
    transportEl->status->setMyDirection(LEFT);
    BOOST_CHECK(tel2->interact(transportEl)==true);
    BOOST_CHECK(transportEl->status->getInstanceId()==mc->getElement(1,2)->status->getInstanceId());
    BOOST_CHECK(transportEl->status->isTeleporting()==true);
}

BOOST_AUTO_TEST_CASE(TeleportAnObjectWithTwoTeleportsDifferentType)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({8,8});
    coords ncrds;
    coords t1crds={2,3};
    coords t2crds={3,4};
    std::shared_ptr<teleport>  tel1=elementFactory::generateAnElement<teleport>(mc,0);
    std::shared_ptr<teleport>  tel2=elementFactory::generateAnElement<teleport>(mc,1);
    std::shared_ptr<bElem> _tr1=elementFactory::generateAnElement<bElem>(mc,0);
    std::shared_ptr<bElem> _tr2=elementFactory::generateAnElement<bElem>(mc,0);
    _tr1->stepOnElement(mc->getElement(2,1));
    _tr2->stepOnElement(mc->getElement(5,6));

    tel1->stepOnElement(mc->getElement(2,2));
    tel2->stepOnElement(mc->getElement(4,4));
    _tr1->status->setMyDirection(DOWN);
    _tr2->status->setMyDirection(LEFT);
    tel1->interact(_tr1);
    tel2->interact(_tr2);
    coords tr1c=_tr1->status->getMyPosition();
    BOOST_CHECK(tr1c==t1crds);
    coords tr2c=_tr2->status->getMyPosition();
    BOOST_CHECK(tr2c==t2crds);
}

BOOST_AUTO_TEST_CASE(WalkInTeleportTests)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({10,10});
    coords pointA={3,5};
    coords pointAt={2,5};
    coords pointB={5,5};

    std::shared_ptr<bElem> tel1,tel2,transported;
    bElem::tick();

    transported=elementFactory::generateAnElement<bElem>(mc,0);
    transported->stepOnElement(mc->getElement(pointAt));
    transported->status->setMyDirection(RIGHT);
    BOOST_CHECK(transported->status->isTeleporting()==false);
    tel1=elementFactory::generateAnElement<teleport>(mc,1);
    tel2=elementFactory::generateAnElement<teleport>(mc,1);
    tel1->stepOnElement(mc->getElement(pointA));
    tel2->stepOnElement(mc->getElement(pointB));
    //ok now step on that teleport
    transported->stepOnElement(tel1);
    BOOST_CHECK(tel1->status->hasActivatedMechanics()==true);
    BOOST_CHECK(transported->status->getMyPosition()==pointA);
    BOOST_CHECK(mc->getElement(pointA)->status->getInstanceId()==transported->status->getInstanceId());
    for(int c=0;c<_teleportStandTime;c++)
    {
        BOOST_CHECK(transported->status->isTeleporting()==false);
        bElem::runLiveElements();
    }
    BOOST_CHECK(transported->status->isTeleporting()==true);
}



BOOST_AUTO_TEST_SUITE_END()
