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
    BOOST_CHECK(plr->getInventory()->getActiveWeapon()!=nullptr);
    BOOST_CHECK(plr->getInventory()->getActiveWeapon()->getInstanceid()==pGun->getInstanceid());
    plr->setDirection(UP);
    for(int c=0; c<50; c++)
    {
        bElem* wep=plr->getInventory()->getActiveWeapon();
        if(wep!=nullptr)
        {
            plr->shootGun();
        }
        for(int d=0; d<100; d++)
            bElem::tick();

    }
    BOOST_CHECK(plr->getInventory()->getActiveWeapon()==nullptr);
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
    player* p=nullptr;
    p=new player(mc);
    bElem* e=nullptr;
    for(int cnt=0; cnt<10000; cnt++)
    {
        e=mc->getElement(1,2);
        BOOST_CHECK(e!=nullptr);
        BOOST_CHECK(e!=p);
        p->stepOnElement(e);
        e->disposeElement();
        gCollect::getInstance()->purgeGarbage();
        p->setActive(true);
        //p->moveInDirection(RIGHT);
        BOOST_CHECK(p->removeElement()!=nullptr);

        BOOST_CHECK(mc->getElement(1,2)!=nullptr);
        BOOST_CHECK(mc->getElement(1,2)!=p);
        p->stepOnElement(mc->getElement(2,2));

        int instances=0;
        for(int a=0; a<10; a++)
        {
            for(int b=0; b<10; b++)
            {
                bElem* e=mc->getElement(a,b);
                BOOST_ASSERT(e!=nullptr);
                while(e!=nullptr)
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

BOOST_AUTO_TEST_CASE(PlayerActivationOnPlayerDeath)
{
    std::vector<chamber*> m;
    player* p;
    player* p1;
    player* p0;
    int iid;
    for(int c=0;c<10;c++)
        m.push_back(new chamber(105,10));

    p0=new player(m[0]);
    p0->stepOnElement(m[1]->getElement(0,0));
    p0->setActive(true);
    for(int a=0;a<100;a++)
    {
        //std::cout<<a<<"\n";
        p=new player(nullptr);
        BOOST_CHECK(p->stepOnElement(nullptr)==false);
        BOOST_CHECK(p->stepOnElement(m[a%m.size()]->getElement(a,a%10))==true);
        BOOST_CHECK(p->stepOnElement(m[a%m.size()]->getElement(a,a%10))==false);
        BOOST_CHECK(p->interact(nullptr)==false);
        BOOST_CHECK(p->interact(p0)==true);
        BOOST_CHECK(p->interact(p0)==false);
    }
    for(int c=0;c<100;c++)
        bElem::runLiveElements();


    p=player::getActivePlayer();
    while(p!=nullptr)
    {
        iid=p->getInstanceid();
        p->disposeElement();
        p1=player::getActivePlayer();
        if(p1!=nullptr)
            BOOST_CHECK(iid!=p1->getInstanceid());
        gCollect::getInstance()->purgeGarbage();
        p=p1;
    }
    for(int c=0;c<10;c++)
        delete m[c];
    m.clear();


}

BOOST_AUTO_TEST_CASE(PlayerCollectApplesThenDestroyedByBombAndThenTheStashDestroyedWithBomb)
{
    chamber* mc=new chamber(10,10);
    goldenApple* gc=new goldenApple(mc);
    player* p=new player(mc);
    simpleBomb* sb=new simpleBomb(mc);
    p->stepOnElement(mc->getElement(2,1));
    sb->stepOnElement(mc->getElement(2,2));
    gc->stepOnElement(mc->getElement(1,1));
    gc=new goldenApple(mc);
    gc->stepOnElement(mc->getElement(1,2));
    p->collect(mc->getElement(1,1));
    p->collect(mc->getElement(1,2));
    BOOST_CHECK(goldenApple::getAppleNumber()>=2);
    sb->kill();
    // We take time for the exploded bomb to finish
    for(int c=0;c<100;c++)
        bElem::runLiveElements();
    BOOST_CHECK(goldenApple::getAppleNumber()>=2);

    gCollect::getInstance()->purgeGarbage();
    sb=new simpleBomb(mc);
    sb->stepOnElement(mc->getElement(2,2));
    sb->kill();
    // We take time for the exploded bomb to finish
    for(int c=0;c<100;c++)
        bElem::runLiveElements();
    gCollect::getInstance()->purgeGarbage();
    BOOST_CHECK(goldenApple::getAppleNumber()==0);

    delete mc;

}
BOOST_AUTO_TEST_CASE(MovePlayer)
{
    chamber* mc=new chamber(100,100);
    controlItem cntrlItm;
    coords c0,c1;
    while(player::getActivePlayer()!=nullptr)
    {
            player::getActivePlayer();
            player::getActivePlayer()->disposeElement();
            gCollect::getInstance()->purgeGarbage();
    }
    player* p=new player(mc);
    p->stepOnElement(mc->getElement(10,10));
    p->setActive(true);
    p=player::getActivePlayer();
    for(int c=0;c<100;c++)
        bElem::runLiveElements();
    c0=p->getCoords();
    mc->cntrlItm.type=0;
    mc->cntrlItm.dir=RIGHT;
    for(int c=0;c<100;c++)
        bElem::runLiveElements();
    mc->cntrlItm.type=0;
    mc->cntrlItm.dir=NODIRECTION;
    for(int c=0;c<100;c++)
        bElem::runLiveElements();
    c1=p->getCoords();
    BOOST_CHECK(c0!=c1);

    c0=p->getCoords();
    mc->cntrlItm.type=0;
    mc->cntrlItm.dir=DOWN;
    bElem::runLiveElements();
    mc->cntrlItm.type=0;
    mc->cntrlItm.dir=NODIRECTION;
    for(int c=0;c<100;c++)
        bElem::runLiveElements();
    c1=p->getCoords();
    BOOST_CHECK(c0!=c1);

    c0=p->getCoords();
    mc->cntrlItm.type=0;
    mc->cntrlItm.dir=LEFT;
    bElem::runLiveElements();
    mc->cntrlItm.type=0;
    mc->cntrlItm.dir=NODIRECTION;
    for(int c=0;c<100;c++)
        bElem::runLiveElements();
    c1=p->getCoords();
    BOOST_CHECK(c0!=c1);


    c0=p->getCoords();
    mc->cntrlItm.type=0;
    mc->cntrlItm.dir=UP;
    bElem::runLiveElements();
    mc->cntrlItm.type=0;
    mc->cntrlItm.dir=NODIRECTION;
    for(int c=0;c<100;c++)
        bElem::runLiveElements();
    c1=p->getCoords();
    BOOST_CHECK(c0!=c1);

}

BOOST_AUTO_TEST_SUITE_END()

