#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "gCollect.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

BOOST_AUTO_TEST_SUITE( PlayerTests )

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
    BOOST_CHECK(plr->getInventory()->getActiveWeapon()!=NULL);
    BOOST_CHECK(plr->getInventory()->getActiveWeapon()->getInstanceid()==pGun->getInstanceid());
    plr->setDirection(UP);
    for(int c=0; c<50; c++)
    {
        bElem* wep=plr->getInventory()->getActiveWeapon();
        if(wep!=NULL)
        {
            plr->shootGun();
        }
        for(int d=0; d<100; d++)
            bElem::tick();

    }
    BOOST_CHECK(plr->getInventory()->getActiveWeapon()==NULL);
    for(unsigned int c=0; c<gCollect::getInstance()->garbageVector.size(); c++)
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



BOOST_AUTO_TEST_CASE(PlayerStepsIntoExplodingBomb)
{
    chamber* mc=new chamber(10,10);
    player* p=NULL;
    bool pDisposed=false;
    simpleBomb* sb=NULL;
    p=new player(mc);
    bElem* e=mc->getElement(1,2);
    for(int cnt=0; cnt<10000; cnt++)
    {


     //   sb=new simpleBomb(mc);
        p->stepOnElement(mc->getElement(1,2));
        e->disposeElement();
        gCollect::getInstance()->purgeGarbage();
        p->setActive(true);
        p->moveInDirection(RIGHT);
        BOOST_CHECK(mc->getElement(1,2)!=NULL);
        BOOST_CHECK(mc->getElement(1,2)!=p);



      /*

        sb->stepOnElement(mc->getElement(5,2));
        sb->explode();
        for(int c=0; c<300; c++)
        {
           // p->setMoved(0);
            p->moveInDirection(RIGHT);
           // p->setMoved(0);
            bElem::runLiveElements();
            //    std::cout<<"X: "<<p->getCoords().x<<" y: "<<p->getCoords().y<<"\n";
        }
        BOOST_CHECK(player::getActivePlayer()!=NULL);
        */
        int instances=0;
        for(int a=0; a<10; a++)
        {
            for(int b=0; b<10; b++)
            {
                bElem* e=mc->getElement(a,b);
                BOOST_ASSERT(e!=NULL);
                while(e!=NULL)
                {
                    if(e->getInstanceid()==p->getInstanceid())
                        instances++;
                    e=e->getSteppingOnElement();
                }

            }
        }
        BOOST_CHECK(instances==1);
    }
    delete mc;
}

BOOST_AUTO_TEST_SUITE_END()

