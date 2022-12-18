#include "elements.h"
#include "commons.h"
#include "chamber.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <memory>

BOOST_AUTO_TEST_SUITE( PlayerTests )

BOOST_AUTO_TEST_CASE(PlayerShootsGun)
{
//    bool gunDisposed=false;
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({5,5});
    std::shared_ptr<player> plr=bElem::generateAnElement<player>(mc);
    plr->stepOnElement(mc->getElement(2,2));
    std::shared_ptr<plainGun> pGun=bElem::generateAnElement<plainGun>(mc);
    pGun->setSubtype(0);
    pGun->stepOnElement(mc->getElement(3,2));
    plr->setActive(true);
    plr->collect(pGun);
    BOOST_CHECK(plr->getInventory()->getActiveWeapon()!=nullptr);
    BOOST_CHECK(plr->getInventory()->getActiveWeapon()->getInstanceid()==pGun->getInstanceid());
    plr->setDirection(UP);
    for(int c=0; c<50; c++)
    {
        std::shared_ptr<bElem> wep=plr->getInventory()->getActiveWeapon();
        if(wep!=nullptr)
        {
            plr->shootGun();
        }
        for(int d=0; d<100; d++)
            bElem::tick();

    }
    BOOST_CHECK(plr->getInventory()->getActiveWeapon()==nullptr);

    pGun=bElem::generateAnElement<plainGun>(mc);
    pGun->setSubtype(0);
    pGun->stepOnElement(mc->getElement(3,2));
    plr->collect(pGun);
    pGun=bElem::generateAnElement<plainGun>(mc);
    pGun->setSubtype(0);
    pGun->stepOnElement(mc->getElement(3,2));
    plr->collect(pGun);
//    for(int c=0;c<pGun->getAmmo();c++)
//    {

//    }
    plr->disposeElement();

    BOOST_CHECK(mc->getElement(2,2)->getType()==_rubishType);
}



BOOST_AUTO_TEST_CASE(PlayerStepsIntoExplodingBomb)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({10,10});
    std::shared_ptr<player> p=nullptr;
    p=bElem::generateAnElement<player>(mc);
    std::shared_ptr<bElem> e=nullptr;
    for(int cnt=0; cnt<10000; cnt++)
    {
        e=mc->getElement(1,2);
        BOOST_CHECK(e!=nullptr);
        BOOST_CHECK(e!=p);
        p->stepOnElement(e);
        e->disposeElement();
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
                std::shared_ptr<bElem> e=mc->getElement(a,b);
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

}

BOOST_AUTO_TEST_CASE(PlayerActivationOnPlayerDeath)
{
    std::vector<std::shared_ptr<chamber>> m;
    std::shared_ptr<player> p;
    std::shared_ptr<player> p1;
    std::shared_ptr<player> p0;
    std::shared_ptr<bElem> tp,tp1;
    int iid;
    for(int c=0; c<10; c++)
        m.push_back(chamber::makeNewChamber({105,10}));

    p0=bElem::generateAnElement<player>(m[0]);
    p0->stepOnElement(m[1]->getElement(0,0));
    p0->setActive(true);
    for(int a=0; a<100; a++)
    {
        //std::cout<<a<<"\n";
        p=bElem::generateAnElement<player>();
        BOOST_CHECK(p->stepOnElement(nullptr)==false);
        BOOST_CHECK(p->stepOnElement(m[a%m.size()]->getElement(a,a%10))==true);
        BOOST_CHECK(p->stepOnElement(m[a%m.size()]->getElement(a,a%10))==false);
        BOOST_CHECK(p->interact(nullptr)==false);
        BOOST_CHECK(p->interact(p0)==true);
        BOOST_CHECK(p->interact(p0)==false);
    }
    for(int c=0; c<100; c++)
        bElem::runLiveElements();


    tp=player::getActivePlayer();
    while(tp!=nullptr)
    {
        iid=tp->getInstanceid();
        tp->disposeElement();
        tp1=player::getActivePlayer();
        if(p1!=nullptr)
            BOOST_CHECK(iid!=p1->getInstanceid());
        tp=tp1;
    }
    m.clear();


}

BOOST_AUTO_TEST_CASE(PlayerCollectApplesThenDestroyedByBombAndThenTheStashDestroyedWithBomb)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({10,10});
    std::shared_ptr<goldenApple> gc=bElem::generateAnElement<goldenApple>(mc);
    std::shared_ptr<player> p=bElem::generateAnElement<player>(mc);
    std::shared_ptr<simpleBomb> sb=bElem::generateAnElement<simpleBomb>(mc);
    p->stepOnElement(mc->getElement(2,1));
    sb->stepOnElement(mc->getElement(2,2));
    gc->stepOnElement(mc->getElement(1,1));
    gc=bElem::generateAnElement<goldenApple>(mc);
    gc->stepOnElement(mc->getElement(1,2));
    p->collect(mc->getElement(1,1));
    p->collect(mc->getElement(1,2));
    BOOST_CHECK(goldenApple::getAppleNumber()>=2);
    p->disposeElement();

    p=nullptr;
    std::cout<<goldenApple::getAppleNumber()<<"\n";
    sb->kill();
    gc.reset();
    // We take time for the exploded bomb to finish
    for(int c=0; c<100; c++)
        bElem::runLiveElements();
    BOOST_CHECK(goldenApple::getAppleNumber()>=2);

    sb=bElem::generateAnElement<simpleBomb>(mc);
    sb->stepOnElement(mc->getElement(2,2));
    sb->kill();
    // We take time for the exploded bomb to finish
    for(int c=0; c<100; c++)
        bElem::runLiveElements();
    gc.reset();
    BOOST_CHECK(goldenApple::getAppleNumber()==0);
}


void controlPlayer(std::shared_ptr<chamber> mc,controlItem cntrlItm)
{
    std::shared_ptr<bElem> p=player::getActivePlayer();
    BOOST_CHECK(p.get()!=nullptr);
    if(p.get()==nullptr)
        return;
    coords c0,c1;
    for(int c=0; c<100; c++)
        bElem::runLiveElements();
    c0=p->getCoords();
    mc->cntrlItm=cntrlItm;
    for(int c=0; c<100; c++)
        bElem::runLiveElements();
    mc->cntrlItm.type=-1;
    mc->cntrlItm.dir=NODIRECTION;
    for(int c=0; c<100; c++)
        bElem::runLiveElements();
    c1=p->getCoords();
    if(cntrlItm.type==0 || cntrlItm.type==4)
        BOOST_CHECK(c0!=c1);
    else
        BOOST_CHECK(c0==c1);

}

void checkplayerKilled()
{
    std::shared_ptr<bElem> p=player::getActivePlayer();
    int iid1=p->getInstanceid();
    for(int c=0; c<100; c++)
        bElem::runLiveElements();
    p=player::getActivePlayer();
    if(p!=nullptr)
    {
        std::cout<<"Player żyje: "<<iid1<<" "<<p->getInstanceid();
    }
    BOOST_CHECK(p==nullptr || iid1!=p->getInstanceid());

}

BOOST_AUTO_TEST_CASE(MovePlayer)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({100,100});
    coords c0,c1;
    controlItem ci;
    while(player::getActivePlayer()!=nullptr)
    {
        player::getActivePlayer();
        player::getActivePlayer()->disposeElement();
    }
    std::shared_ptr<player> p=bElem::generateAnElement<player>(mc);
    std::shared_ptr<plainGun> pg=bElem::generateAnElement<plainGun>(mc);
    p->stepOnElement(mc->getElement(50,50));
    p->collect(pg);
    p->setActive(true);
    for(int c=0; c<8; c++)
    {
        if(c==6)
        {
            mc->cntrlItm.type=6;
            checkplayerKilled();
            p=bElem::generateAnElement<player>(mc);
            pg=bElem::generateAnElement<plainGun>(mc);
            p->collect(pg);
            p->stepOnElement(mc->getElement(50,50));
            p->setActive(true);
            continue;
        }
        ci.type=c;
        ci.dir=UP;
        controlPlayer(mc,ci);
        ci.dir=DOWN;
        controlPlayer(mc,ci);
        ci.dir=LEFT;
        controlPlayer(mc,ci);
        ci.dir=RIGHT;
        controlPlayer(mc,ci);

    }


}

BOOST_AUTO_TEST_SUITE_END()

