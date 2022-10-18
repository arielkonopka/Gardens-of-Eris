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




typedef boost::mpl::list<bElem,killableElements,player,mechanical,collectible,door,explosives,movableElements> base_test_types;

typedef boost::mpl::list<bElem,bunker,door,explosives,goldenApple,key,killableElements,mechanical,monster,movableElements,nonSteppable,patrollingDrone,plainGun,plainMissile,player,rubbish,stillElem,teleport,usable,wall> all_test_types;


/* bElem class unit tests */

BOOST_AUTO_TEST_SUITE( BasicObjectTests )
//Create a bElem object, and destroy it - smoke test
BOOST_AUTO_TEST_CASE( bElemCreateDestroy )
{
    // the simpliest possible test case
    bElem* be=new bElem();
    BOOST_ASSERT( be!=NULL);
    delete be;

}
//create an object, a whole chamber, and then place an object somewhere, and remove it, very simple case
BOOST_AUTO_TEST_CASE_TEMPLATE( bElemCreateDestroyChamber,T,all_test_types)
{
    chamber* chmbr=new chamber(10,10); // we need only a small chamber
    BOOST_ASSERT( chmbr!=NULL );

    for(int c=0; c<chmbr->width; c++) // check that all elements are not null
        for(int d=0; d<chmbr->height; d++)
        {
            bElem* beOrig=chmbr->getElement(c,d);
            coords mcoords= {c,d};
            BOOST_ASSERT(beOrig!=NULL);
            BOOST_CHECK( beOrig->getType()==_belemType);
            BOOST_CHECK(beOrig->getCoords()==mcoords); // just check if the allocation is correct
            coords crds=beOrig->getCoords();
            BOOST_ASSERT(crds.x==c);
            BOOST_ASSERT(crds.y==d);
            BOOST_CHECK(beOrig->getStomper()==NULL);
            BOOST_ASSERT(beOrig->getSteppingOnElement()==NULL);
        }

    bElem* beOrig=chmbr->getElement(0,0); // ok, now let's step on something
    bElem* be=new T(chmbr);
    BOOST_ASSERT( be!=NULL );
    be->stepOnElement(chmbr->getElement(0,0));
    BOOST_CHECK(be->getBoard()==chmbr);
    bElem* be2=chmbr->getElement(0,0); // check if the element is placed
    BOOST_CHECK(be->getInstanceid()==be2->getInstanceid());
    BOOST_ASSERT(be->getSteppingOnElement()!=NULL); // something is under the new object
    BOOST_CHECK(be->getSteppingOnElement()->getInstanceid()==beOrig->getInstanceid()); // check it is original background
    BOOST_CHECK(beOrig->getStomper()!=NULL); // check, that the object below, "knows" it is below.
    BOOST_CHECK(beOrig->getStomper()->getInstanceid()==be->getInstanceid());
    be->removeElement(); // remove the object from the board
    BOOST_CHECK(beOrig->getStomper()==NULL); //check if the original object is being stepped on
    be2=chmbr->getElement(0,0); // fetch the element from the board, and compare it with the original object, there should be a match
    BOOST_CHECK(beOrig->getInstanceid()==be2->getInstanceid());
    BOOST_CHECK(beOrig->getBoard()==chmbr);
    BOOST_CHECK(be->getBoard()==NULL);
    BOOST_CHECK(be->getCoords()==NOCOORDS);
    delete be;
    delete chmbr;


    /*
     Napisać resztę testu - tutaj t
    */



}

BOOST_AUTO_TEST_CASE_TEMPLATE( bElemCreateThenDispose,T,all_test_types)
{
    chamber* chmbr=new chamber(10,10); // we need only a small chamber
    BOOST_ASSERT( chmbr!=NULL );
    bElem* beOrig=chmbr->getElement(0,0); // ok, now let's step on something
    bElem* be=new T(chmbr);
    BOOST_ASSERT( be!=NULL );
    be->stepOnElement(chmbr->getElement(0,0));
    BOOST_CHECK(be->getBoard()==chmbr);
    bElem* be2=chmbr->getElement(0,0); // check if the element is placed
    BOOST_CHECK(be->getInstanceid()==be2->getInstanceid());
    BOOST_ASSERT(be->getSteppingOnElement()!=NULL); // something is under the new object
    BOOST_CHECK(be->getSteppingOnElement()->getInstanceid()==beOrig->getInstanceid()); // check it is original background
    BOOST_CHECK(beOrig->getStomper()!=NULL); // check, that the object below, "knows" it is below.
    BOOST_CHECK(beOrig->getStomper()->getInstanceid()==be->getInstanceid());
    be->disposeElement(); // remove the object from the board
    BOOST_CHECK(beOrig->getStomper()==NULL); //check if the original object is being stepped on
    be2=chmbr->getElement(0,0); // fetch the element from the board, and compare it with the original object, there should be a match
    BOOST_CHECK(beOrig->getInstanceid()==be2->getInstanceid());
    BOOST_CHECK(beOrig->getBoard()==chmbr);
    BOOST_CHECK(be->getBoard()==NULL);
    BOOST_CHECK(be->getCoords()==NOCOORDS);
    gCollect::getInstance()->purgeGarbage();
    delete chmbr;



}











bool searchForIdInSteppers(bElem* el,int id)
{
    while(el!=NULL)
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
    while(b!=NULL)
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
    while(first!=NULL)
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
        bElem* be=new bElem(mc,3,3);
        BOOST_ASSERT(mc->getElement(3,3)->getInstanceid()==be->getInstanceid());
        BOOST_ASSERT(mc->getElement(3,3)->getSteppingOnElement()!=NULL);
    }
    bElem* last=new T(mc);
    last->stepOnElement(mc->getElement(3,3));

    //we at first take the last element, at the bottom, because it usually causes issues
    te=findLastStep(mc->getElement(3,3));
    te2=te->removeElement();
    if(mc->getElement(3,3)!=NULL) //check if element is really removed
        BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),te2->getInstanceid())==false);
    delete te2;
    te=mc->getElement(3,3);
    while(findDepth(mc->getElement(3,3))>2)
    {
        if(te==NULL)
        {
            te=mc->getElement(3,3);
            ccc=0;
        }
        //     std::cout<<"depth="<<ccc<<" "<<findDepth(mc->getElement(3,3))<<" "<<(std::string)((te->getSteppingOnElement()!=NULL)?"Middle ":"Edge ")<<te->getInstanceid()<<"\n";
        if(bElem::randomNumberGenerator()%2==0)
        {

            //          std::cout<<"Delete\n";
            bElem* te3=te->getSteppingOnElement();
            te2=te->removeElement();
            te=te3;
            BOOST_ASSERT(te2!=NULL);
            if(mc->getElement(3,3)!=NULL)
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
        bElem* be=new bElem(mc,3,3);
        BOOST_ASSERT(mc->getElement(3,3)->getInstanceid()==be->getInstanceid());
        BOOST_ASSERT(mc->getElement(3,3)->getSteppingOnElement()!=NULL);
    }
    bElem* last=new T(mc);
    last->stepOnElement(mc->getElement(3,3));
    te=findLastStep(mc->getElement(3,3));
    BOOST_CHECK(te->getInstanceid()!=mc->getElement(3,3)->getInstanceid());
    myId=te->getInstanceid();
    te->disposeElement();
    if(mc->getElement(3,3)!=NULL)
        BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),myId)==false);

    te=mc->getElement(3,3);
    while(mc->getElement(3,3)!=NULL && mc->getElement(3,3)->getSteppingOnElement()!=NULL)
    {
        if(bElem::randomNumberGenerator()%2==0)
        {

            bElem* te3=te->getSteppingOnElement();
            myId=te->getInstanceid();
            te->disposeElement();
            te=te3;
            if(mc->getElement(3,3)!=NULL)
                BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),myId)==false);
            BOOST_CHECK(findInstanceInGarbage(myId)==true);

        }
        else
        {
            te=te->getSteppingOnElement();
        }
        if(te==NULL)
            te=mc->getElement(3,3);
    }
    delete mc;
    gCollect::getInstance()->purgeGarbage();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(StackingAndDestroyingTheWholeChamber,T,all_test_types)
{
    chamber* mc=new chamber(11,11);
    bElem* be=NULL;
    bElem* be1=new bElem(mc);
    BOOST_CHECK(be1!=NULL);
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
            mc->getElement(x,y)->mechanics(false);
        }

    }



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
    for(int d=1; d<1000; d++)
    {
        int c=0;
        //std::cout<<"d="<<d<<"\n";
        testObj->setWait(d);
        while(testObj->isWaiting())
        {
            BOOST_ASSERT(testObj->mechanics(false)==false); //mechanics is blocked during waiting time
            c++;
            bElem::tick();
        }
        //  bElem::tick();
        BOOST_ASSERT(testObj->mechanics(false)==true); // mechanics is unblocked after the waiting time
        //std::cout<<"c="<<c<<"\n";
        if(d<=_maxWaitingTtime)
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
    int origType=myObj->getType();
    // std::cout<<"type:"<<myObj->getType()<<" "<<_belemType<<"\n";
    int instance=myObj->getInstanceid();
    bElem::tick();
    myObj->stepOnElement(mc->getElement(3,3));
    myObj->destroy();
    for(int c=0; c<_defaultDestroyTime; c++)
    {
        BOOST_CHECK(mc->getElement(3,3)->isDestroyed()==true);
        bElem::runLiveElements();
    }
    myObj=mc->getElement(3,3);
    BOOST_CHECK(mc->getElement(3,3)->isDestroyed()==false);
    if(origType==_belemType)
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

            while(el!=NULL)
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






// BOOST_AUTO_TEST_CASE()

BOOST_AUTO_TEST_SUITE_END()





//#endif // BELEM_H_INCLUDED
//#endif
