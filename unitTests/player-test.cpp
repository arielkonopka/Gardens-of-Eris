#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "gCollect.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>


BOOST_AUTO_TEST_CASE(PlayerShootsGun)
{
    bool gunDisposed=false;
    chamber* mc=new chamber(5,5);
    player* plr=new player(mc);
    plr->stepOnElement(mc->getElement(2,2));
    plainGun* pGun=new plainGun(mc);
    pGun->setSubtype(0);
    pGun->stepOnElement(mc->getElement(3,2));
    plr->setActive(true);
    plr->collect(pGun);
    BOOST_CHECK(plr->myInventory->getActiveWeapon()!=NULL);
    BOOST_CHECK(plr->myInventory->getActiveWeapon()->getInstanceid()==pGun->getInstanceid());
    plr->setDirection(UP);
    for(int c=0;c<50;c++)
    {
        bElem* wep=plr->myInventory->getActiveWeapon();
        if(wep!=NULL)
        {
            plr->shootGun();
        }
        for(int d=0;d<100;d++)
            bElem::tick();

    }
    BOOST_CHECK(plr->myInventory->getActiveWeapon()==NULL);
    for(unsigned int c=0;c<gCollect::getInstance()->garbageVector.size();c++)
    {
        if(gCollect::getInstance()->garbageVector[c]->getInstanceid()==pGun->getInstanceid())
        {
            gunDisposed=true;
        }
    }
    BOOST_ASSERT(gunDisposed==true);
    gCollect::getInstance()->purgeGarbage();
    pGun=new plainGun(mc);
    pGun->setSubtype(0);
    pGun->stepOnElement(mc->getElement(3,2));
    plr->collect(pGun);
    pGun=new plainGun(mc);
    pGun->setSubtype(0);
    pGun->stepOnElement(mc->getElement(3,2));
    plr->collect(pGun);
//    for(int c=0;c<pGun->getAmmo();c++)
//    {

//    }
    plr->disposeElement();

    BOOST_CHECK(mc->getElement(2,2)->getType()==_rubishType);
    delete mc;
}
