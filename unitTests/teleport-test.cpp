#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "gCollect.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>


BOOST_AUTO_TEST_SUITE( TeleportObjectTests )

BOOST_AUTO_TEST_CASE( TeleportAnObjectWithOneTeleport)
{
    chamber* mc=new chamber(5,5);
    teleport* tel1=new teleport(mc,0);
    //teleport* tel2=new teleport(mc,0);
    bElem* transportedE=new bElem(mc);
    transportedE->stepOnElement(mc->getElement(2,3));
    //  transportedE->setActive(true);
    coords crds;
    transportedE->setDirection(RIGHT);
    bElem::tick();
    bElem::tick();
    tel1->stepOnElement(mc->getElement(3,3));
    BOOST_CHECK(tel1->interact(transportedE)==true);
    // bElem::tick();
    std::cout<<"teleporting1:"<<mc->getElement(2,3)->isTeleporting();
    BOOST_CHECK(mc->getElement(2,3)->getInstanceid()!=transportedE->getInstanceid());
    BOOST_CHECK(mc->getElement(2,3)->isTeleporting()==true);
    crds=transportedE->getCoords();
    //  std::cout<<"x:"<<crds.x<<" y:"<<crds.y<<"\n";
    BOOST_CHECK(mc->getElement(crds.x,crds.y)->getInstanceid()==transportedE->getInstanceid());
    BOOST_CHECK(mc->getElement(crds.x,crds.y)->isTeleporting()==true);
    for(int a=0; a<_teleportationTime; a++)
    {
        BOOST_CHECK(mc->getElement(2,3)->isTeleporting()==true);
        BOOST_CHECK(mc->getElement(crds.x,crds.y)->isTeleporting()==true);
        bElem::tick();
    }
    BOOST_CHECK(mc->getElement(crds.x,crds.y)->isTeleporting()==false);
    BOOST_CHECK(mc->getElement(2,3)->isTeleporting()==false);
    delete mc;
    gCollect::getInstance()->purgeGarbage();
}
//We have two teleports of the same type
BOOST_AUTO_TEST_CASE(TeleportAnObjectWithTwoTeleportsOneChamber)
{
    coords crds;
    coords nc= {5,4};
    chamber* mc=new chamber(6,6);
    teleport* tel1=new teleport(mc,0);
    teleport* tel2=new teleport(mc,0);
    bElem* transportEl=new bElem(mc);
    transportEl->stepOnElement(mc->getElement(1,2));
    bElem::tick();
    bElem::tick();
    transportEl->setDirection(RIGHT);
    tel1->stepOnElement(mc->getElement(2,2));
    tel2->stepOnElement(mc->getElement(4,4));
    tel1->interact(transportEl);
    crds=transportEl->getCoords();
    BOOST_CHECK( crds==nc );
    BOOST_CHECK(transportEl->getInstanceid()==mc->getElement(5,4)->getInstanceid());
    BOOST_CHECK(transportEl->isTeleporting()==true);
    for(int c=0; c<_teleportationTime+1; c++)
        bElem::tick();
    BOOST_CHECK(transportEl->isTeleporting()==false);
    transportEl->setDirection(LEFT);
    BOOST_CHECK(tel2->interact(transportEl)==true);
    BOOST_CHECK(transportEl->getInstanceid()==mc->getElement(1,2)->getInstanceid());
    BOOST_CHECK(transportEl->isTeleporting()==true);
    delete mc;
    gCollect::getInstance()->purgeGarbage();
}

BOOST_AUTO_TEST_CASE(TeleportAnObjectWithTwoTeleportsDifferentType)
{
    chamber* mc=new chamber(8,8);
    coords ncrds;
    coords t1crds={2,3};
    coords t2crds={3,4};
    teleport* tel1=new teleport(mc,0);
    teleport* tel2=new teleport(mc,1);
    bElem* _tr1=new bElem(mc);
    bElem* _tr2=new bElem(mc);
    _tr1->stepOnElement(mc->getElement(2,1));
    _tr2->stepOnElement(mc->getElement(5,6));

    tel1->stepOnElement(mc->getElement(2,2));
    tel2->stepOnElement(mc->getElement(4,4));
    _tr1->setDirection(DOWN);
    _tr2->setDirection(LEFT);
    tel1->interact(_tr1);
    tel2->interact(_tr2);
    coords tr1c=_tr1->getCoords();
    BOOST_CHECK(tr1c==t1crds);
    coords tr2c=_tr2->getCoords();
    BOOST_CHECK(tr2c==t2crds);
    delete mc;
    gCollect::getInstance()->purgeGarbage();
}

BOOST_AUTO_TEST_CASE(WalkInTeleportTests)
{
    chamber* mc=new chamber(10,10);
    coords pointA={3,5};
    coords pointAt={2,5};
    coords pointB={5,5};

    bElem *tel1,*tel2,*transported;
    bElem::tick();

    transported=new bElem(mc);
    transported->stepOnElement(mc->getElement(pointAt));
    transported->setDirection(RIGHT);
    BOOST_CHECK(transported->isTeleporting()==false);
    tel1=new teleport(mc,1);
    tel2=new teleport(mc,1);
    tel1->stepOnElement(mc->getElement(pointA));
    tel2->stepOnElement(mc->getElement(pointB));
    //ok now step on that teleport
    transported->stepOnElement(tel1);
    BOOST_CHECK(tel1->isLiveElement()==true);
    BOOST_CHECK(transported->getCoords()==pointA);
    BOOST_CHECK(mc->getElement(pointA)->getInstanceid()==transported->getInstanceid());
    for(int c=0;c<_teleportStandTime;c++)
    {
        BOOST_CHECK(transported->isTeleporting()==false);
        bElem::runLiveElements();
    }
    BOOST_CHECK(transported->isTeleporting()==true);

    delete mc;
}



BOOST_AUTO_TEST_SUITE_END()
