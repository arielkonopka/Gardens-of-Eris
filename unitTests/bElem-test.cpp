//#ifndef _UNIT_TEST_BUILD_
//#ifndef BELEM_H_INCLUDED
//#define BELEM_H_INCLUDED
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "gCollect.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "floorElement.h"



typedef boost::mpl::list<bElem,killableElements,player,mechanical,collectible,door,explosives,movableElements> base_test_types;

typedef boost::mpl::list<bElem,bunker,floorElement,door,explosives,goldenApple,key,killableElements,mechanical,monster,movableElements,nonSteppable,patrollingDrone,plainGun,plainMissile,player,rubbish,teleport,usable,wall> all_test_types;


int countTheStack(bElem* in)
{
    bElem* f=in->getBoard()->getElement(in->getCoords());
    int cnt=0;
    while(f!=nullptr)
    {
        cnt++;
        f=f->getSteppingOnElement();
    }
    return cnt;
}
int findHowManyTimesObjectIsInStack(bElem* in,int instanceId)
{
    bElem* f=in->getBoard()->getElement(in->getCoords());
    int cnt=0;
    while(f!=nullptr)
    {
        if(f->getInstanceid()==instanceId)
            cnt++;
        f=f->getSteppingOnElement();
    }
    return cnt;
}

/* bElem class unit tests */

BOOST_AUTO_TEST_SUITE( BasicObjectTests )
//Create a bElem object, and destroy it - smoke test
BOOST_AUTO_TEST_CASE( bElemCreateDestroy )
{
    // the simpliest possible test case
    bElem* be=new bElem();
    BOOST_ASSERT( be!=nullptr);
    delete be;

}
//create an object, a whole chamber, and then place an object somewhere, and remove it, very simple case
BOOST_AUTO_TEST_CASE_TEMPLATE( bElemCreateDestroyChamber,T,all_test_types)
{
    chamber* chmbr=new chamber(10,10); // we need only a small chamber
    BOOST_ASSERT( chmbr!=nullptr );

    for(int c=0; c<chmbr->width; c++) // check that all elements are not nullptr
        for(int d=0; d<chmbr->height; d++)
        {
            bElem* beOrig=chmbr->getElement(c,d);
            coords mcoords= {c,d};
            BOOST_ASSERT(beOrig!=nullptr);
            BOOST_CHECK( beOrig->getType()==_belemType);
            BOOST_CHECK(beOrig->getCoords()==mcoords); // just check if the allocation is correct
            BOOST_CHECK(beOrig->getStomper()==nullptr);
            BOOST_ASSERT(beOrig->getSteppingOnElement()==nullptr);
        }

    bElem* beOrig=chmbr->getElement(0,0); // ok, now let's step on something
    bElem* be=new T(chmbr);
    BOOST_ASSERT( be!=nullptr );
    be->stepOnElement(chmbr->getElement(0,0));
    BOOST_CHECK(be->getBoard()==chmbr);
    bElem* be2=chmbr->getElement(0,0); // check if the element is placed
    BOOST_CHECK(be->getInstanceid()==be2->getInstanceid());
    BOOST_ASSERT(be->getSteppingOnElement()!=nullptr); // something is under the new object
    BOOST_CHECK(be->getSteppingOnElement()->getInstanceid()==beOrig->getInstanceid()); // check it is original background
    BOOST_CHECK(beOrig->getStomper()!=nullptr); // check, that the object below, "knows" it is below.
    BOOST_CHECK(beOrig->getStomper()->getInstanceid()==be->getInstanceid());
    be->removeElement(); // remove the object from the board
    BOOST_CHECK(beOrig->getStomper()==nullptr); //check if the original object is being stepped on
    be2=chmbr->getElement(0,0); // fetch the element from the board, and compare it with the original object, there should be a match
    BOOST_CHECK(beOrig->getInstanceid()==be2->getInstanceid());
    BOOST_CHECK(beOrig->getBoard()==chmbr);
    BOOST_CHECK(be->getBoard()==nullptr);
    BOOST_CHECK(be->getCoords()==NOCOORDS);
    delete be;
    delete chmbr;


    /*
     Napisać resztę testu - tutaj t
    */



}

BOOST_AUTO_TEST_CASE_TEMPLATE( bElemCreateThenDispose,T,all_test_types)
{
    coords point= {3,3};
    chamber* chmbr=new chamber(10,10); // we need only a small chamber

    BOOST_ASSERT( chmbr!=nullptr );

    bElem* beOrig=chmbr->getElement(point); // ok, now let's step on something
    bElem* be=new T(chmbr);

    BOOST_ASSERT( be!=nullptr );
    be->stepOnElement(chmbr->getElement(point));
    BOOST_CHECK(be->getBoard()==chmbr);
    BOOST_CHECK(be->getCoords()==point); // we check, that the coordinates are set properly
    bElem* be2=chmbr->getElement(point); // check if the element is placed
    BOOST_CHECK(be->getInstanceid()==be2->getInstanceid());
    BOOST_ASSERT(be->getSteppingOnElement()!=nullptr); // something is under the new object
    BOOST_CHECK(be->getSteppingOnElement()->getInstanceid()==beOrig->getInstanceid()); // check it is original background
    BOOST_CHECK(beOrig->getStomper()!=nullptr); // check, that the object below, "knows" it is below.
    BOOST_CHECK(beOrig->getStomper()->getInstanceid()==be->getInstanceid());
    be->disposeElement(); // remove the object from the board
    BOOST_CHECK(beOrig->getStomper()==nullptr); //check if the original object is being stepped on
    be2=chmbr->getElement(point); // fetch the element from the board, and compare it with the original object, there should be a match
    BOOST_CHECK(beOrig->getInstanceid()==be2->getInstanceid());
    BOOST_CHECK(beOrig->getBoard()==chmbr);
    BOOST_CHECK(be->getBoard()==nullptr);
    BOOST_CHECK(be->getCoords()==NOCOORDS);
    gCollect::getInstance()->purgeGarbage();
    delete chmbr;
}



BOOST_AUTO_TEST_CASE(StackAndRemoveFromTheFeet)
{
    coords point={3,3};
    chamber* mc=new chamber(10,10);
    bElem* o=mc->getElement(point);
    bElem* e=new bElem(mc);
    bElem* e1=new bElem(mc);
    BOOST_CHECK(o->getInstanceid()!=e->getInstanceid() && o->getInstanceid()!=e1->getInstanceid());
    e->stepOnElement(mc->getElement(point));
    e1->stepOnElement(mc->getElement(point));
    o->disposeElement();
    gCollect::getInstance()->purgeGarbage();
    e1->disposeElement();
    gCollect::getInstance()->purgeGarbage();
    e->disposeElement();
//    gCollect::getInstance()->purgeGarbage();
    BOOST_CHECK(mc->getElement(point)->getInstanceid()!=e->getInstanceid());

    delete mc;
}







bool searchForIdInSteppers(bElem* el,int id)
{
    while(el!=nullptr)
    {
        if(id==el->getInstanceid())
            return true;
        el=el->getSteppingOnElement();
    }
    return false;
}

int findDepth(bElem* b)
{
    int d=0;
    while(b!=nullptr)
    {
        d++;
        if(b->getSteppingOnElement())
        {

            b=b->getSteppingOnElement();

        }
        else
        {
            return d;
        }

    }
    return d;
}

bElem* findLastStep(bElem* first)
{
    bElem* last=first;
    while(first!=nullptr)
    {
        last=first;
        first=first->getSteppingOnElement();
    }
    return last;
}

//Place few objects on each other, then remove some from the top, bottom, middle
BOOST_AUTO_TEST_CASE_TEMPLATE(StackingAndRemovingTest,T,all_test_types)
{
    chamber* mc=new chamber(5,5);
    bElem* te;
    bElem* te2;
    int ccc=0;
    for(int x=0; x<10; x++)
    {
        bElem* be=new bElem(mc);
        be->stepOnElement(mc->getElement(3,3));
        BOOST_ASSERT(mc->getElement(3,3)->getInstanceid()==be->getInstanceid());
        BOOST_ASSERT(mc->getElement(3,3)->getSteppingOnElement()!=nullptr);
    }
    bElem* last=new T(mc);
    last->stepOnElement(mc->getElement(3,3));

    //we at first take the last element, at the bottom, because it usually causes issues
    te=findLastStep(mc->getElement(3,3));
    te2=te->removeElement();
    if(mc->getElement(3,3)!=nullptr) //check if element is really removed
        BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),te2->getInstanceid())==false);
    delete te2;
    te=mc->getElement(3,3);
    while(findDepth(mc->getElement(3,3))>2)
    {
        if(te==nullptr)
        {
            te=mc->getElement(3,3);
            ccc=0;
        }
        //     std::cout<<"depth="<<ccc<<" "<<findDepth(mc->getElement(3,3))<<" "<<(std::string)((te->getSteppingOnElement()!=nullptr)?"Middle ":"Edge ")<<te->getInstanceid()<<"\n";
        if(bElem::randomNumberGenerator()%2==0)
        {

            //          std::cout<<"Delete\n";
            bElem* te3=te->getSteppingOnElement();
            te2=te->removeElement();
            te=te3;
            BOOST_ASSERT(te2!=nullptr);
            if(mc->getElement(3,3)!=nullptr)
                BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),te2->getInstanceid())==false);
            delete te2;
        }
        else
        {
            te=te->getSteppingOnElement();
            ccc++;
        }

    }
    delete mc;
    gCollect::getInstance()->purgeGarbage();
}


bool findInstanceInGarbage(int instance)
{
    for(unsigned int cnt=0; cnt<gCollect::getInstance()->garbageVector.size(); cnt++)
    {
        if (instance==gCollect::getInstance()->garbageVector[cnt]->getInstanceid())
            return true;
    }
    return false;

}

//Place few objects on each other, then remove some from the top, bottom, middle
BOOST_AUTO_TEST_CASE_TEMPLATE(StackingAndDisposingTest,T,all_test_types)
{
    chamber* mc=new chamber(5,5);
    bElem* te;
    int myId;
    for(int x=0; x<100; x++)
    {
        bElem* be=new bElem(mc);
        be->stepOnElement(mc->getElement(3,3));
        BOOST_ASSERT(mc->getElement(3,3)->getInstanceid()==be->getInstanceid());
        BOOST_ASSERT(mc->getElement(3,3)->getSteppingOnElement()!=nullptr);
    }
    bElem* last=new T(mc);
    last->stepOnElement(mc->getElement(3,3));
    te=findLastStep(mc->getElement(3,3));
    BOOST_CHECK(te->getInstanceid()!=mc->getElement(3,3)->getInstanceid());
    myId=te->getInstanceid();
    te->disposeElement();
    if(mc->getElement(3,3)!=nullptr)
        BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),myId)==false);

    te=mc->getElement(3,3);
    while(mc->getElement(3,3)!=nullptr && mc->getElement(3,3)->getSteppingOnElement()!=nullptr)
    {
        if(bElem::randomNumberGenerator()%2==0)
        {

            bElem* te3=te->getSteppingOnElement();
            myId=te->getInstanceid();
            te->disposeElement();
            te=te3;
            if(mc->getElement(3,3)!=nullptr)
                BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),myId)==false);
            BOOST_CHECK(findInstanceInGarbage(myId)==true);

        }
        else
        {
            te=te->getSteppingOnElement();
        }
        if(te==nullptr)
            te=mc->getElement(3,3);
    }
    delete mc;
    gCollect::getInstance()->purgeGarbage();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(StackingAndDestroyingTheWholeChamber,T,all_test_types)
{
    chamber* mc=new chamber(11,11);
    bElem* be=nullptr;
    bElem* be1=new bElem(mc);
    BOOST_CHECK(be1!=nullptr);
    BOOST_CHECK(be1->stepOnElement(mc->getElement(10,10))==true);
    for(int x=0; x<10; x++)
    {
        for(int y=0; y<10; y++)
        {
            be=new T(mc);
            be->stepOnElement(mc->getElement(x,y));
            BOOST_CHECK(be->getInstanceid()==mc->getElement(x,y)->getInstanceid());
            be->setActive(true);
            be->interact(be1);
            //be->mechanics(false);
            // be->use(be1);
        }
        //     std::cout<<"\n";
    }
    for(int x=0; x<11; x++)
    {
        for(int y=0; y<11; y++)
        {
            mc->getElement(x,y)->mechanics();
        }

    }



    delete mc;

}

/*
 This test is supposed to check if stepOnElement can place an object between two instances of bElem placed on each other
 It works by first build 100 object high "pile"
 then it iterates downwards and places new elements on each iterated object. This way we can check that:
 first object would be covered (that we know from previous tests)
 second object can be covered with new one
 */
BOOST_AUTO_TEST_CASE( StepOverElementTests)
{
    coords point= {2,2};

    chamber* mc=new chamber(10,10);
    int stackSize=countTheStack(mc->getElement(point));
    bElem* nElement=nullptr;
    for(int c=0; c<100; c++)
    {
        nElement=new bElem(mc);
        BOOST_CHECK(nElement->stepOnElement(mc->getElement(point))==true);
        BOOST_CHECK(nElement->getInstanceid()==mc->getElement(point)->getInstanceid());
        BOOST_CHECK(nElement->getCoords()==point);
        BOOST_CHECK(stackSize<countTheStack(mc->getElement(point)));
        stackSize=countTheStack(mc->getElement(point));
    }
    nElement=mc->getElement(point);
    int elementCnt=0;
    while(nElement!=nullptr)
    {
        int origId=0;
        int nEInstanceId=nElement->getInstanceid();
        origId=mc->getElement(point)->getInstanceid();
        bElem* nE2=new bElem(mc);
        bElem *stmp,*steppOn;
        stmp=nElement->getStomper();
        steppOn=nElement->getSteppingOnElement();
        BOOST_CHECK(nE2->stepOnElement(nElement)==true);
        BOOST_CHECK(nE2->getCoords()==point);
        BOOST_CHECK(findHowManyTimesObjectIsInStack(nElement,nEInstanceId)==1);
        BOOST_CHECK(nElement->getStomper()->getInstanceid()==nE2->getInstanceid());
        BOOST_CHECK(nE2->getSteppingOnElement()->getInstanceid()==nElement->getInstanceid());
        BOOST_CHECK(mc->getElement(point)->getInstanceid()!=nElement->getInstanceid());
        if(steppOn!=nullptr)
        {
            BOOST_CHECK(steppOn->getInstanceid()==nElement->getSteppingOnElement()->getInstanceid());
            BOOST_CHECK(steppOn->getStomper()->getInstanceid()==nElement->getInstanceid());

        }
        if(stmp!=nullptr)
        {
            BOOST_CHECK(origId==mc->getElement(point)->getInstanceid());
            BOOST_CHECK(stmp->getInstanceid()==nE2->getStomper()->getInstanceid());
            BOOST_CHECK(stmp->getSteppingOnElement()->getStomper()->getInstanceid()==stmp->getInstanceid());
        }
        BOOST_CHECK(stackSize<countTheStack(mc->getElement(point)));
        stackSize=countTheStack(mc->getElement(point));

        nElement=nElement->getSteppingOnElement();
        elementCnt++;
    }
    BOOST_CHECK(countTheStack(mc->getElement(point))>200); // it is because when we create the chamber, there is already one element placed in the chamber
    delete mc;

}



BOOST_AUTO_TEST_CASE_TEMPLATE(SubTypeChecker,T,all_test_types)
{
    chamber* mc=new chamber(10,10);
    bElem* myobj=new T(mc);
    for(int x=0; x<10; x++)
    {
        myobj->setSubtype(x);
        BOOST_CHECK(myobj->getSubtype()==x);
    }
    delete myobj;
    delete mc;

}


BOOST_AUTO_TEST_CASE_TEMPLATE(WaitMechanismTest,T,base_test_types)
{
    chamber* mc=new chamber(5,5);
    bElem* testObj=new T(mc);
    testObj->stepOnElement(mc->getElement(3,3));
    //  testObj->setActive(true);
    for(int d=1; d<100; d++)
    {
        int c=0;
        //std::cout<<"d="<<d<<"\n";
        testObj->setWait(d);
        while(testObj->isWaiting())
        {
//            BOOST_ASSERT(testObj->isWaiting()==true); //mechanics is blocked during waiting time
            c++;
            bElem::tick();
            if(c>_maxWaitingTtime+10) break;
        }
       // std::cout<<"c="<<c<<" d: "<<d<<"\n";
        BOOST_ASSERT(testObj->isWaiting()==false); // mechanics is unblocked after the waiting time
        if(d<_maxWaitingTtime+1)
        {
            BOOST_ASSERT(c==d);
        }
        else
        {
            BOOST_ASSERT(c==0);
        }

    }
    delete mc;
    gCollect::getInstance()->purgeGarbage();
}

// Check destroying mechanism. Remember, that for bElem types, the object is not disposed
// We do not check the disposal process itself, as it should be tested with other tests
BOOST_AUTO_TEST_CASE_TEMPLATE(DestroyObjectOnBoard,T,base_test_types)
{
    chamber* mc=new chamber(5,5);

    bElem* myObj=new T(mc);
    bool canBeDestroyed=myObj->canBeDestroyed();
    bool isSteppable=myObj->isSteppable();
  //  int origType=myObj->getType();
    // std::cout<<"type:"<<myObj->getType()<<" "<<_belemType<<"\n";
    int instance=myObj->getInstanceid();
    bElem::tick();
    bElem::tick();

    myObj->stepOnElement(mc->getElement(3,3));
    myObj->destroy();
    for(int c=0; c<_defaultDestroyTime+1; c++)
    {
        BOOST_CHECK(mc->getElement(3,3)->isDestroyed()==true);
        bElem::runLiveElements();
    }
    myObj=mc->getElement(3,3);
    BOOST_CHECK(mc->getElement(3,3)->isDestroyed()==false);
    if(!canBeDestroyed || isSteppable)
    {
        BOOST_CHECK(mc->getElement(3,3)->getInstanceid()==instance);
    }
    else
    {

        BOOST_CHECK(mc->getElement(3,3)->getInstanceid()!=instance);
    }
}


BOOST_AUTO_TEST_CASE_TEMPLATE(ElementsLockUnlockFeature,T,all_test_types)
{
    chamber* mc=new chamber(5,5);
    bElem* myElement=new T(mc);
    bElem* blocker=new T(mc);
    bElem* blocker2=new T(mc);
    myElement->stepOnElement(mc->getElement(2,2));
    blocker->stepOnElement(mc->getElement(2,3));
    blocker2->stepOnElement(mc->getElement(3,2));
    BOOST_CHECK(myElement->isLocked()==false);
    myElement->lockThisObject(blocker);
    BOOST_CHECK(myElement->isLocked()==true);
    myElement->lockThisObject(blocker);
    BOOST_CHECK(myElement->isLocked()==true);

    myElement->lockThisObject(blocker2);
    BOOST_CHECK(myElement->isLocked()==true);
    myElement->unlockThisObject(blocker);
    BOOST_CHECK(myElement->isLocked()==true);
    myElement->unlockThisObject(blocker2);
    BOOST_CHECK(myElement->isLocked()==false);
    myElement->unlockThisObject(blocker2);
    BOOST_CHECK(myElement->isLocked()==false);
//another round
    /*
        myElement->lockThisObject(blocker);
        BOOST_CHECK(myElement->isLocked()==true);
        myElement->lockThisObject(blocker2);
        BOOST_CHECK(myElement->isLocked()==true);
        blocker2->disposeElement();
        BOOST_CHECK(myElement->isLocked()==true);
        blocker->disposeElement();
        BOOST_CHECK(myElement->isLocked()==false);
        gCollect::getInstance()->purgeGarbage();
        BOOST_CHECK(myElement->isLocked()==false);
    */
    delete mc;

}

BOOST_AUTO_TEST_CASE_TEMPLATE(InteractTimerMechanismChecker,T,all_test_types)
{

    chamber* mc=new chamber(5,5);
    bElem* tElem=new T(mc);

    tElem->stepOnElement(mc->getElement(3,3));
    tElem=new T(mc);
    tElem->stepOnElement(mc->getElement(2,2));

    for(int c=0; c<1000; c++) bElem::tick();
    tElem->interact(mc->getElement(1,1));
    BOOST_CHECK(tElem->canInteract()==false);
    for(int c=0; c<_interactedTime+1; c++) bElem::tick();
    for(int c=0; c<1000; c++)
    {
        bElem::tick();

        BOOST_CHECK(tElem->canInteract()==true);
    }
    tElem->disposeElement();
    delete mc;
    gCollect::getInstance()->purgeGarbage();

}


/*
Check the collect feature, especially, when we:
1. dispose something from the inventory
2. destroy an object with an inventory
3. destroy the whole board

*/
BOOST_AUTO_TEST_CASE_TEMPLATE(TryToCollectAnObjectAndDisposeIt,T,all_test_types)
{
    chamber* mc=new chamber(5,5);
    bElem* mO=new T(mc);
    bElem* mC=new T(mc);
    inventory* nInv=new inventory(mO);
    mO->setInventory(nInv);
    mO->collect(mc->getElement(2,3));
    mO->stepOnElement(mc->getElement(2,2));
    mC->stepOnElement(mc->getElement(2,3));
    //Check if the collect method works well with its limits
    BOOST_CHECK(mO->collect(mc->getElement(2,3))==(mO->canCollect()==true && mC->isCollectible()==true));
    if(mO->canCollect()==true && mC->isCollectible()==true)
    {
        BOOST_CHECK(mc->getElement(2,3)->getInstanceid()!=mC->getInstanceid());
    }
    else
    {
        BOOST_CHECK(mc->getElement(2,3)->getInstanceid()==mC->getInstanceid());
    }

    mC=new T(mc);
    mC->stepOnElement(mc->getElement(3,2));
    mO->collect(mc->getElement(3,2));
    BOOST_CHECK(mC->isDisposed()==false);
    mC->disposeElement();
    BOOST_CHECK(mC->isDisposed()==true);
    // check the disposed element not present in the board or any inventory
    for(int a=0; a<mc->width; a++)
    {
        for(int b=0; b<mc->height; b++)
        {
            BOOST_CHECK(mc->getElement(a,b)->getInstanceid()!=mC->getInstanceid());
            if(mc->getElement(a,b)->canCollect()==true)
            {
                BOOST_CHECK(mc->getElement(a,b)->getInventory()->findInInventory(mC->getInstanceid())==false);
            }
            bElem* el=mc->getElement(a,b)->getSteppingOnElement();

            while(el!=nullptr)
            {
                BOOST_CHECK(el->getInstanceid()!=mC->getInstanceid());
                if(el->canCollect()==true)
                {
                    BOOST_CHECK(el->getInventory()->findInInventory(mC->getInstanceid())==false);
                }
                el=el->getSteppingOnElement();
            }
        }
    }
    //Find it in the vector of disposed elements
    bool disFound=false;
    for(auto c:gCollect::getInstance()->garbageVector)
    {
        if(c->getInstanceid()==mC->getInstanceid())
        {
            disFound=true;
        }
    }
    BOOST_ASSERT(disFound==true);
    gCollect::getInstance()->purgeGarbage();
    disFound=false;
    for(auto c:gCollect::getInstance()->garbageVector)
    {
        if(c->getInstanceid()==mC->getInstanceid())
        {
            disFound=true;
        }
    }
    BOOST_ASSERT(disFound==false);

    delete mc;
}


BOOST_AUTO_TEST_CASE_TEMPLATE(TryToRemoveElementMoreThanNeeded,T,all_test_types)
{
    chamber* mc=new chamber(3,3);
    bElem* myObj=new T(mc);
    bElem* relic;
    myObj->stepOnElement(mc->getElement(1,1));
    myObj=new T(mc);
    myObj->stepOnElement(mc->getElement(2,2));

    for(int c=0;c<100;c++)
    {
#ifdef _VerbousMode_
   std::cout<<" * [1] removing "<<c<<"\n";
#endif

        relic=mc->getElement(1,1)->removeElement();
        BOOST_CHECK(relic!=nullptr);
        delete relic;
#ifdef _VerbousMode_
   std::cout<<" * [2] removing "<<c<<"\n";
#endif

        BOOST_CHECK(mc->getElement(2,2)->disposeElement()==DISPOSED);
        BOOST_CHECK(gCollect::getInstance()->purgeGarbage()==true);
    }
    delete mc;
}

BOOST_AUTO_TEST_CASE(rwg)
{
    randomLevelGenerator* rwg=new randomLevelGenerator(400,400);
    BOOST_CHECK(rwg!=nullptr);
    BOOST_CHECK(rwg->mychamber!=nullptr);

    delete rwg;
    rwg=new randomLevelGenerator(40,40);
    BOOST_CHECK(rwg!=nullptr);
    BOOST_CHECK(rwg->mychamber!=nullptr);

    delete rwg;
    rwg=new randomLevelGenerator(10,10);
    BOOST_CHECK(rwg!=nullptr);
    BOOST_CHECK(rwg->mychamber!=nullptr);

    delete rwg;


}

// BOOST_AUTO_TEST_CASE()

BOOST_AUTO_TEST_SUITE_END()



