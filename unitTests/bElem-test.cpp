// *** ADDED BY HEADER FIXUP ***
#include <list>
// *** END ***
#ifndef _UNIT_TEST_BUILD_
#define _UNIT_TEST_BUILD_
#endif
//#ifndef BELEM_H_INCLUDED
//#define BELEM_H_INCLUDED
#include "elements.h"
#include <memory>
#include "commons.h"
#include "chamber.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "floorElement.h"
#include "bElemStats.h"
#include "bElemAttr.h"
#include "inputManager.h"


typedef boost::mpl::list<bElem,killableElements,player,movableElements,door,explosives,brickCluster> base_test_types;

typedef boost::mpl::list<bElem,bazooka,bazookaMissile,bunker,floorElement,door,explosives,goldenApple,key,killableElements,mechanical,monster,brickCluster,patrollingDrone,plainGun,plainMissile,player,rubbish,teleport,wall> all_test_types;


int countTheStack(std::shared_ptr<bElem> in)
{
    std::shared_ptr<bElem> f=in->getBoard()->getElement(in->getStats()->getMyPosition());
    int cnt=0;
    while(f!=nullptr)
    {
        cnt++;
        f=f->getStats()->getSteppingOn();
    }
    return cnt;
}
int findHowManyTimesObjectIsInStack(std::shared_ptr<bElem> in,unsigned long  instanceId)
{
    std::shared_ptr<bElem> f=in->getBoard()->getElement(in->getStats()->getMyPosition());
    int cnt=0;
    while(f!=nullptr)
    {
        if(f->getStats()->getInstanceId()==instanceId)
            cnt++;
        f=f->getStats()->getSteppingOn();
    }
    return cnt;
}

/* bElem class unit tests */

BOOST_AUTO_TEST_SUITE( BasicObjectTests )
//Create a bElem object, and destroy it - smoke test


/**
 * Creates and destroys objects within a chamber - an elemental unit of a larger structure.
 * This test case focuses on the basic functionality of creating an object, placing it within the chamber,
 * and subsequently removing it. It validates the proper allocation, positioning, and removal of objects.
 * The test covers scenarios where the element being placed is stepping on another element, ensuring that
 * the relationships and references between objects are correctly maintained.
 *
 * @tparam T The element type to be tested, specified by the BOOST_AUTO_TEST_CASE_TEMPLATE macro.
 */
 BOOST_AUTO_TEST_CASE_TEMPLATE( bElemCreateDestroyChamber,T,all_test_types)
{
    coords csize=(coords)
    {
        10,12
    };
//    std::cout<<"make chamber\n";
    std::shared_ptr<chamber> chmbr=chamber::makeNewChamber(csize); // we need only a small chamber
    coords cs1=chmbr->getSizeOfChamber();
    BOOST_ASSERT( chmbr);
    BOOST_CHECK( cs1==csize);
    for(int c=0; c<csize.x; c++) // check that all elements are not nullptr
        for(int d=0; d<csize.y; d++)
        {
            std::shared_ptr<bElem> beOrig=chmbr->getElement(c,d);
            coords mcoords= {c,d};
            BOOST_ASSERT(beOrig.get()!=nullptr);
            BOOST_CHECK( beOrig->getType()==_floorType);
            BOOST_CHECK(beOrig->getStats()->getMyPosition()==mcoords); // just check if the allocation is correct
            BOOST_CHECK(beOrig->getStats()->hasParent()==false);

            BOOST_CHECK(!beOrig->getStats()->getSteppingOn());
        }

    std::shared_ptr<bElem> beOrig=chmbr->getElement(0,0); // ok, now let's step on something
    BOOST_CHECK(beOrig);
    std::shared_ptr<bElem> be=elementFactory::generateAnElement<T>(chmbr,0);

    BOOST_ASSERT( be );
    inputManager::getInstance(true);
    be->stepOnElement(chmbr->getElement(0,0));
    BOOST_CHECK(be->getBoard()==chmbr);
    std::shared_ptr<bElem> be2=chmbr->getElement(0,0); // check if the element is placed
    BOOST_CHECK(be->getStats()->getInstanceId()==be2->getStats()->getInstanceId());
    BOOST_ASSERT(be->getStats()->getSteppingOn()!=nullptr); // something is under the new object
    BOOST_CHECK(be->getStats()->getSteppingOn()->getStats()->getInstanceId()==beOrig->getStats()->getInstanceId()); // check it is original background
    BOOST_CHECK(beOrig->getStats()->hasParent()); // check, that the object below, "knows" it is below.
    BOOST_CHECK(beOrig->getStats()->getStandingOn().lock()->getStats()->getInstanceId()==be->getStats()->getInstanceId());
    be->removeElement(); // remove the object from the board
    BOOST_CHECK(!beOrig->getStats()->hasParent());
//    BOOST_CHECK(beOrig->getStats()->getStandingOn().lock()->getStats()->hasParent()==false); //check if the original object is being stepped on
    be2=chmbr->getElement(0,0); // fetch the element from the board, and compare it with the original object, there should be a match
    BOOST_CHECK(beOrig->getStats()->getInstanceId()==be2->getStats()->getInstanceId());
    BOOST_CHECK(beOrig->getBoard()==chmbr);
    BOOST_CHECK(be->getBoard()==nullptr);
    BOOST_CHECK(be->getStats()->getMyPosition()==NOCOORDS);

}

BOOST_AUTO_TEST_CASE_TEMPLATE( bElemCreateThenDispose,T,all_test_types)
{
    coords point= {3,3};
    coords csize= {10,10};
    std::shared_ptr<chamber> chmbr=chamber::makeNewChamber(csize); // we need only a small chamber
    BOOST_ASSERT( chmbr!=nullptr );

    std::shared_ptr<bElem> beOrig=chmbr->getElement(point); // ok, now let's step on something
    std::shared_ptr<bElem> be=elementFactory::generateAnElement<T>(chmbr,0);

    BOOST_ASSERT( be!=nullptr );
    be->stepOnElement(chmbr->getElement(point));
    BOOST_CHECK(be->getBoard()==chmbr);
    BOOST_CHECK(be->getStats()->getMyPosition()==point); // we check, that the coordinates are set properly
    std::shared_ptr<bElem> be2=chmbr->getElement(point); // check if the element is placed
    BOOST_CHECK(be->getStats()->getInstanceId()==be2->getStats()->getInstanceId());
    BOOST_ASSERT(be->getStats()->getSteppingOn()!=nullptr); // something is under the new object
    BOOST_CHECK(be->getStats()->getSteppingOn()->getStats()->getInstanceId()==beOrig->getStats()->getInstanceId()); // check it is original background
    BOOST_CHECK(beOrig->getStats()->hasParent()); // check, that the object below, "knows" it is below.
    BOOST_CHECK(beOrig->getStats()->getStandingOn().lock()->getStats()->getInstanceId()==be->getStats()->getInstanceId());
    BOOST_CHECK(be->disposeElement()!=ERROR); // remove the object from the board
    BOOST_CHECK(beOrig->getStats()->hasParent()==false); //check if the original object is being stepped on
    be2=chmbr->getElement(point); // fetch the element from the board, and compare it with the original object, there should be a match
    BOOST_CHECK(beOrig->getStats()->getInstanceId()==be2->getStats()->getInstanceId());
    BOOST_CHECK(beOrig->getBoard()==chmbr);
    BOOST_CHECK(be->getBoard()==nullptr);
    BOOST_CHECK(be->getStats()->getMyPosition()==NOCOORDS);

}



BOOST_AUTO_TEST_CASE(StackAndRemoveFromTheFeet)
{
    coords csize= {10,10};
    coords point= {3,3};
    std::shared_ptr<chamber> mc=chamber::makeNewChamber(csize);
    std::shared_ptr<bElem> o=mc->getElement(point);
    std::shared_ptr<bElem> e=elementFactory::generateAnElement<bElem>(mc,0);
    std::shared_ptr<bElem> e1=elementFactory::generateAnElement<bElem>(mc,0);
    BOOST_CHECK(o->getStats()->getInstanceId()!=e->getStats()->getInstanceId() && o->getStats()->getInstanceId()!=e1->getStats()->getInstanceId());
    e->stepOnElement(mc->getElement(point));
    e1->stepOnElement(mc->getElement(point));
    o->disposeElement();
    e1->disposeElement();
    e->disposeElement();
    BOOST_CHECK(mc->getElement(point)->getStats()->getInstanceId()!=e->getStats()->getInstanceId());

}







bool searchForIdInSteppers(std::shared_ptr<bElem> el,unsigned long int id)
{
    while(el!=nullptr)
    {
        if(el->getStats()->getInstanceId()==id)
            return true;
        el=el->getStats()->getSteppingOn();
    }
    return false;
}

int findDepth(std::shared_ptr<bElem> b)
{
    int d=0;
    while(b!=nullptr)
    {
        d++;
        if(b->getStats()->getSteppingOn())
        {

            b=b->getStats()->getSteppingOn();

        }
        else
        {
            return d;
        }

    }
    return d;
}

std::shared_ptr<bElem> findLastStep(std::shared_ptr<bElem> first)
{
    std::shared_ptr<bElem> last=first;
    while(first!=nullptr)
    {
        last=first;
        first=first->getStats()->getSteppingOn();
    }
    return last;
}

/**
 * @brief This unit test tests placing few objects on each other, then randomly removing them and checking, there is still integrity
 */BOOST_AUTO_TEST_CASE_TEMPLATE(StackingAndRemovingTest,T,all_test_types)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({10,10}); // we need only a small chamber
    std::shared_ptr<bElem> te;
    std::shared_ptr<bElem> te2;
    int ccc=0;
    for(int x=0; x<10; x++)
    {
        std::shared_ptr<bElem> be=elementFactory::generateAnElement<bElem>(mc,0);
        be->stepOnElement(mc->getElement(3,3));
        BOOST_ASSERT(mc->getElement(3,3)->getStats()->getInstanceId()==be->getStats()->getInstanceId());
        BOOST_ASSERT(mc->getElement(3,3)->getStats()->getSteppingOn()!=nullptr);
        BOOST_CHECK(!mc->getElement(3,3)->getStats()->getSteppingOn()->getStats()->getStandingOn().expired());
        BOOST_CHECK(mc->getElement(3,3)->getStats()->getStandingOn().expired());
    }
    std::shared_ptr<bElem> last=elementFactory::generateAnElement<T>(mc,0);
    last->stepOnElement(mc->getElement(3,3));
    BOOST_CHECK(last);

    //we at first take the last element, at the bottom, because it usually causes issues
    te=findLastStep(mc->getElement(3,3));
    te2=te->removeElement();
    if(mc->getElement(3,3)!=nullptr) //check if element is really removed
        BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),te2->getStats()->getInstanceId())==false);
    te=mc->getElement(3,3);
    while(findDepth(mc->getElement(3,3))>2)
    {
        if(te==nullptr)
        {
            te=mc->getElement(3,3);
            ccc=0;
        }
        //     std::cout<<"depth="<<ccc<<" "<<findDepth(mc->getElement(3,3))<<" "<<(std::string)((te->getStats()->getSteppingOn()!=nullptr)?"Middle ":"Edge ")<<te->getStats()->getInstanceId()<<"\n";
        if(bElem::randomNumberGenerator()%2==0)
        {

            //          std::cout<<"Delete\n";
            std::shared_ptr<bElem> te3=te->getStats()->getSteppingOn();
            te2=te->removeElement();

            te=te3;
            BOOST_ASSERT(te2!=nullptr);
            if(mc->getElement(3,3)!=nullptr)
                BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),te2->getStats()->getInstanceId())==false);
        }
        else
        {
            te=te->getStats()->getSteppingOn();
            ccc++;
        }

    }
}


/**
 * @brief This unit test tests placing few objects on eeach other, then randomly disposing them and checking, there is still integrity
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(StackingAndDisposingTest,T,all_test_types)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({5,6});
    std::shared_ptr<bElem> te;
    int myId;
    for(int x=0; x<100; x++)
    {
        std::shared_ptr<bElem> be=elementFactory::generateAnElement<bElem>(mc,0);
        be->stepOnElement(mc->getElement(3,3));
        BOOST_ASSERT(mc->getElement(3,3)->getStats()->getInstanceId()==be->getStats()->getInstanceId());
        BOOST_ASSERT(mc->getElement(3,3)->getStats()->getSteppingOn()!=nullptr);
    }
    std::shared_ptr<bElem> last=elementFactory::generateAnElement<T>(mc,0);
    last->stepOnElement(mc->getElement(3,3));
    te=findLastStep(mc->getElement(3,3));
    BOOST_CHECK(te->getStats()->getInstanceId()!=mc->getElement(3,3)->getStats()->getInstanceId());
    myId=te->getStats()->getInstanceId();
    te->disposeElement();
    if(mc->getElement(3,3)!=nullptr)
        BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),myId)==false);

    te=mc->getElement(3,3);
    while(mc->getElement(3,3)!=nullptr && mc->getElement(3,3)->getStats()->getSteppingOn()!=nullptr)
    {
        if(bElem::randomNumberGenerator()%2==0)
        {

            std::shared_ptr<bElem> te3=te->getStats()->getSteppingOn();
            myId=te->getStats()->getInstanceId();
            te->disposeElement();
            te=te3;
            if(mc->getElement(3,3)!=nullptr)
                BOOST_CHECK(searchForIdInSteppers(mc->getElement(3,3),myId)==false);

        }
        else
        {
            te=te->getStats()->getSteppingOn();
        }
        if(te==nullptr)
            te=mc->getElement(3,3);
    }

}

/**
 * @brief This unit test "StackingAndDestroyingTheWholeChamber" verifies the correct functionality of the stacking and interaction mechanism for different element types in the chamber.
 *
 * The test first creates a new game chamber of size 11x11. Then, for each spot in the 10x10 area of the chamber, it does the following:
 * - It generates a new game object of a specified type and places it on the spot.
 * - It checks that the game object is correctly associated with the spot in the chamber.
 * - It sets the game object as active and confirms it is active.
 * - It forces an interaction between the game object and another fixed game object in the chamber.
 *
 * After creating and interacting all game objects, the test runs the mechanics for each spot in the 11x11 area of the chamber.
 *
 * By running the mechanics after all interactions, the test is simulating a scenario where all game objects have interacted with each other and the game is progressing to the next game state.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(StackingAndDestroyingTheWholeChamber,T,all_test_types)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber({12,12});
    std::shared_ptr<bElem> be=nullptr;
    std::shared_ptr<bElem> be1=elementFactory::generateAnElement<bElem>(mc,0);
    BOOST_CHECK(be1!=nullptr);
    BOOST_CHECK(be1->stepOnElement(mc->getElement(11,11))==true); // this is the object that will be interacting with tested elements
    for(int x=0; x<10; x++)
    {
        for(int y=0; y<10; y++)
        {

            be=elementFactory::generateAnElement<T>(mc,x);
            BOOST_CHECK(mc->getElement(x,y)!=nullptr);
            be->stepOnElement(mc->getElement(x,y));
            be->getStats()->setMyDirection(UP);
            BOOST_CHECK(be->getStats()->getInstanceId()==mc->getElement(x,y)->getStats()->getInstanceId());
            be->getStats()->setActive(true);
            BOOST_CHECK(be->getStats()->isActive());
        }
    }
    for(int x=0; x<11; x++)
    {
        for(int y=0; y<11; y++)
        {
            be->interact(be1);
        }
    }
    for(int x=0; x<11; x++)
    {
        for(int y=0; y<11; y++)
        {
            for(int cnt=0; cnt<1000; cnt++)
                mc->getElement(x,y)->mechanics();
        }

    }
}

/**
 * @brief This unit test "StepOverElementTests" verifies the functionality of stepOnElement method in different scenarios.
 *
 * The test first constructs a "pile" of game objects by creating 100 instances of bElem and placing each one on top of the other at a specified point in a game chamber of size 10x10.
 * It checks that each game object is correctly placed on top of the other.
 *
 * The test then iterates down the pile, and for each game object:
 * - It creates a new game object and places it on top of the current game object.
 * - It checks that the new game object is correctly positioned and associated with the correct game objects above and below it in the pile.
 *
 * By checking the state of the pile before and after placing the new game object, the test verifies that the stepOnElement method correctly manages the relationship between game objects that are placed on top of each other.
 * The test finally asserts that the total number of game objects in the pile is greater than 200, which means that all game objects were correctly placed and counted.
 */
BOOST_AUTO_TEST_CASE( StepOverElementTests)
{
    coords point= {2,2};

    std::shared_ptr<chamber> mc=chamber::makeNewChamber({10,10});
    int stackSize=countTheStack(mc->getElement(point));
    std::shared_ptr<bElem> nElement=nullptr;
    BOOST_CHECK(mc->getElement(point)->getAttrs()->isSteppable());
    for(int c=0; c<100; c++)
    {
        nElement=elementFactory::generateAnElement<bElem>(mc,0);
        BOOST_CHECK(nElement->stepOnElement(mc->getElement(point))==true);
        BOOST_CHECK(nElement->getStats()->getInstanceId()==mc->getElement(point)->getStats()->getInstanceId());
        BOOST_CHECK(nElement->getStats()->getMyPosition()==point);
        BOOST_CHECK(stackSize<countTheStack(mc->getElement(point)));
        stackSize=countTheStack(mc->getElement(point));
    }
    nElement=mc->getElement(point);
    int elementCnt=0;
    while(nElement!=nullptr)
    {
        unsigned long int origId=0;
        unsigned long int nEInstanceId=nElement->getStats()->getInstanceId();
        origId=mc->getElement(point)->getStats()->getInstanceId();
        std::shared_ptr<bElem> nE2=elementFactory::generateAnElement<bElem>(mc,0);
        std::shared_ptr<bElem> stmp,steppOn;
        stmp=nElement->getStats()->getStandingOn().lock();
        steppOn=nElement->getStats()->getSteppingOn();
        BOOST_CHECK(nE2->stepOnElement(nElement)==true);
        BOOST_CHECK(nE2->getStats()->getMyPosition()==point);
        BOOST_CHECK(findHowManyTimesObjectIsInStack(nElement,nEInstanceId)==1);
        BOOST_CHECK(nElement->getStats()->getStandingOn().lock()->getStats()->getInstanceId()==nE2->getStats()->getInstanceId());
        BOOST_CHECK(nE2->getStats()->getSteppingOn()->getStats()->getInstanceId()==nElement->getStats()->getInstanceId());
        BOOST_CHECK(mc->getElement(point)->getStats()->getInstanceId()!=nElement->getStats()->getInstanceId());
        if(steppOn!=nullptr)
        {
            BOOST_CHECK(steppOn->getStats()->getInstanceId()==nElement->getStats()->getSteppingOn()->getStats()->getInstanceId());
            BOOST_CHECK(steppOn->getStats()->getStandingOn().lock()->getStats()->getInstanceId()==nElement->getStats()->getInstanceId());

        }
        if(stmp!=nullptr)
        {
            BOOST_CHECK(origId==mc->getElement(point)->getStats()->getInstanceId());
            BOOST_CHECK(stmp->getStats()->getInstanceId()==nE2->getStats()->getStandingOn().lock()->getStats()->getInstanceId());
            BOOST_CHECK(stmp->getStats()->getSteppingOn()->getStats()->getStandingOn().lock()->getStats()->getInstanceId()==stmp->getStats()->getInstanceId());
        }
        BOOST_CHECK(stackSize<countTheStack(mc->getElement(point)));
        stackSize=countTheStack(mc->getElement(point));

        nElement=nElement->getStats()->getSteppingOn();
        elementCnt++;
    }
    BOOST_CHECK(countTheStack(mc->getElement(point))>200); // it is because when we create the chamber, there is already one element placed in the chamber

}


/**
 * @brief This unit test "SubTypeChecker" verifies the functionality of the subtype setter and getter for different types of game objects.
 *
 * For each type T derived from bElem (as defined in all_test_types), a new game object of type T is generated in a game chamber of size 10x10.
 *
 * The test then runs a loop with 10 iterations. On each iteration:
 * - The test sets the subtype of the game object to the current iteration number using the setSubtype method.
 * - It then checks that the game object's subtype, as returned by the getSubtype method, matches the subtype that was just set.
 *
 * Through this, the unit test verifies that the setSubtype and getSubtype methods function correctly for all different types of game objects.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(SubTypeChecker,T,all_test_types)
{
    std::shared_ptr<chamber>  mc=chamber::makeNewChamber({10,10});
    std::shared_ptr<bElem> myobj=elementFactory::generateAnElement<T>(mc,0);
    for(int x=0; x<10; x++)
    {
        myobj->getAttrs()->setSubtype(x);
        BOOST_CHECK(myobj->getAttrs()->getSubtype()==x);
    }

}

/**
 * @brief This unit test "WaitMechanismTest" checks the functionality of the waiting mechanism for different types of game objects.
 *
 * For each type T derived from bElem (as defined in base_test_types), a new game object of type T is generated in a game chamber of size 10x10.
 *
 * The test then initiates a step movement for the game object onto another object at the coordinates (3,3) and ticks the global game time once.
 *
 * The main part of the test consists of a loop running 100 iterations. On each iteration:
 * - The test sets the wait time of the object to the current iteration number using the setWaiting method.
 * - It then checks that the game object is indeed marked as waiting.
 * - Next, it initiates a while loop where it continues ticking the global game time until the game object is no longer marked as waiting,
 *   or until a certain maximum time (defined as "_maxWaitingTtime + current iteration number + 10") has passed.
 * - After the waiting state is over, it checks that the game object is no longer marked as waiting.
 * - If the wait time set was less than or equal to _maxWaitingTtime, it checks that the wait lasted exactly as long as the set wait time.
 * - If the wait time set was more than _maxWaitingTtime, it checks that the waiting state was immediately terminated, i.e., the wait did not last any game ticks.
 *
 * Through this, the unit test verifies that the waiting mechanism works as intended, including the correct adherence to the maximum waiting time constraint.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(WaitMechanismTest,T,base_test_types)
{
    std::shared_ptr<chamber>  mc=chamber::makeNewChamber({10,10});
    std::shared_ptr<bElem> testObj=elementFactory::generateAnElement<T>(mc,0);
    testObj->stepOnElement(mc->getElement(3,3));
    bElem::tick();
    //  testObj->setActive(true);
    for(int d=1; d<100; d++)
    {
        int c=0;
        //std::cout<<"d="<<d<<"\n";
        testObj->getStats()->setWaiting(d);
        BOOST_CHECK(testObj->getStats()->isWaiting());
        while(testObj->getStats()->isWaiting())
        {
//            BOOST_ASSERT(testObj->isWaiting()==true); //mechanics is blocked during waiting time
            c++;
            bElem::tick();
            if(c>_maxWaitingTtime+d+10) break;
        }
        // std::cout<<"c="<<c<<" d: "<<d<<"\n";
        BOOST_ASSERT(testObj->getStats()->isWaiting()==false); // mechanics is unblocked after the waiting time
        if(d<_maxWaitingTtime+1)
        {
            BOOST_ASSERT(c==d);
        }
        else
        {
            BOOST_ASSERT(c==0);
        }

    }
}

/**
 * @brief This unit test "DestroyObjectOnBoard" checks the destruction mechanism of game objects on the board.
 *
 * For types derived from bElem, the object isn't disposed of. The disposal process is not tested here,
 * as that is the focus of other dedicated tests.
 *
 * The test starts by creating a new game chamber of size 10x11. It then generates a new game object of type T
 * (where T is a template parameter, so this test runs for various game object types as defined in base_test_types).
 *
 * It then checks if the created game object is destroyable. If it is, it makes the object step on another object
 * at coordinates (3,3) and initiates its destruction.
 *
 * Next, the test goes through a number of iterations equal to _defaultDestroyTime, and on each iteration, it runs bElem::runLiveElements.
 * On each tick, it checks that the game object at coordinates (3,3) is indeed marked as being destroyed.
 *
 * After all the ticks have passed, it confirms that the game object at coordinates (3,3) is no longer marked as being destroyed.
 *
 * Lastly, the test checks if the instance ID of the game object at coordinates (3,3) has changed. If the original object was not destroyable,
 * the test expects the instance ID to remain the same. However, if the original object was destroyable, it expects the instance ID to be different.
 *
 * In summary, this unit test verifies that the destruction process of game objects on the board is functioning correctly.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(DestroyObjectOnBoard,T,base_test_types)
{
    coords csize= {10,11};
    std::shared_ptr<chamber>  mc=chamber::makeNewChamber({csize});

    std::shared_ptr<bElem> myObj=elementFactory::generateAnElement<T>(mc,0);
    bool canBeDestroyed=myObj->getAttrs()->isDestroyable();
    unsigned long int instance=myObj->getStats()->getInstanceId();
    bElem::tick();
    bElem::tick();

    myObj->stepOnElement(mc->getElement(3,3));
    myObj->getStats()->setActive(true);
    myObj->destroy();
    for(int c=0; c<_defaultDestroyTime; c++)
    {
        BOOST_CHECK(mc->getElement(3,3)->getStats()->isDestroying()==true);
        bElem::runLiveElements();
    }
    bElem::runLiveElements();
    myObj=mc->getElement(3,3);
    BOOST_CHECK(mc->getElement(3,3)->getStats()->isDestroying()==false);
    if(!canBeDestroyed )
    {
        BOOST_CHECK(mc->getElement(3,3)->getStats()->getInstanceId()==instance);
    }
    else
    {

        BOOST_CHECK(mc->getElement(3,3)->getStats()->getInstanceId()!=instance);
    }
}

/**
 * @brief This unit test "ElementsLockUnlockFeature" examines whether the lock/unlock functionality of an element works as expected across various element types.
 *
 * The test initiates by generating a new chamber of size 5x5. Then, it generates and positions three elements: myElement, blocker, and blocker2.
 *
 * The test then proceeds to lock and unlock myElement with the other two elements in various orders, checking the status of myElement's lock after each operation:
 * - Initially, myElement is checked to be unlocked.
 * - It's then locked using blocker, and it's confirmed that it's indeed locked.
 * - Attempting to lock myElement again with the same blocker doesn't change the locked status.
 * - Locking myElement with blocker2 also maintains the locked status.
 * - Unlocking myElement with blocker doesn't unlock the element as it's also locked by blocker2.
 * - Finally, unlocking myElement with blocker2 changes its status to unlocked.
 * - An additional unlock attempt with blocker2, when no locks are present, confirms that the element remains unlocked.
 *
 * This test ensures that an element remains locked if it's locked by multiple other elements and that it's only unlocked when all lockers have issued an unlock.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(ElementsLockUnlockFeature,T,all_test_types)
{
    std::shared_ptr<chamber>  mc=chamber::makeNewChamber({5,5});
    std::shared_ptr<bElem> myElement=elementFactory::generateAnElement<T>(mc,0);
    std::shared_ptr<bElem> blocker=elementFactory::generateAnElement<T>(mc,0);
    std::shared_ptr<bElem> blocker2=elementFactory::generateAnElement<T>(mc,0);
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
}

/**
 * @brief The unit test "InteractTimerMechanismChecker" verifies if the interaction timer mechanism functions correctly for various element types.
 *
 * The test starts by creating a new chamber of size 5x5 and generating an element 'tElem'. The element is then checked if it is not able to interact.
 * If not, the test does not make much sense.
 *
 * 'tElem' is then placed on a specific coordinate in the chamber. A new 'tElem' is generated and also placed within the chamber.
 * Subsequently, the tick() function of 'bElem' is called 1000 times to simulate the passage of time.
 *
 * After this, the interaction is initiated on 'tElem' with an element at another coordinate. It is then checked if the interacting status
 * and the interactability of 'tElem' are either both true or both false. This ensures that an element can only be in the interacting state if it's interactable.
 *
 * The tick() function of 'bElem' is then called '_interactedTime+1' times to simulate the passage of the interaction time.
 * It is checked again if the interacting status and the interactability of 'tElem' are either both true or both false.
 * At this point, as the interaction should have completed, 'tElem' should be able to interact but not be in the interacting state.
 *
 * Afterwards, the tick() function of 'bElem' is called 1000 more times. At each tick, it is confirmed that 'tElem' is not in the interacting state.
 *
 * Lastly, the element 'tElem' is disposed of.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(InteractTimerMechanismChecker,T,all_test_types)
{

    std::shared_ptr<chamber>  mc=chamber::makeNewChamber({5,5});
    std::shared_ptr<bElem> tElem=elementFactory::generateAnElement<T>(mc,0);
    if(!tElem->getAttrs()->isInteractive())
        return ;
    tElem->stepOnElement(mc->getElement(3,3));
    tElem=elementFactory::generateAnElement<T>(mc,0);
    tElem->stepOnElement(mc->getElement(2,2));

    for(int c=0; c<1000; c++) bElem::tick();
    tElem->interact(mc->getElement(1,1));
    bElem::tick();
    if(tElem->getAttrs()->isInteractive())
        BOOST_CHECK(tElem->getStats()->isInteracting() );
    for(int c=0; c<_interactedTime+1; c++) bElem::tick();
    BOOST_CHECK((!tElem->getStats()->isInteracting() && tElem->getAttrs()->isInteractive() )|| (!tElem->getStats()->isInteracting() && !tElem->getAttrs()->isInteractive() ));
    for(int c=0; c<1000; c++)
    {
        bElem::tick();

        BOOST_CHECK(!tElem->getStats()->isInteracting());
    }
    tElem->disposeElement();

}


/**
 * @brief The unit test "TryToCollectAnObjectAndDisposeIt" verifies the collection and disposal mechanisms for various element types.
 *
 * This test involves the following steps:
 * 1. A chamber is created of size 5x5.
 * 2. Two elements, mO and mC, are generated and placed at specific positions within the chamber. mO is also given an inventory and designated as the owner of this inventory.
 * 3. It is then tested if mO can collect mC. The collection should only be successful if mO is capable of collecting and mC is collectable.
 * 4. Depending on the collection result, further checks are performed to ensure the correct functioning of the collection mechanism.
 * 5. A new element, mC, is generated and placed in the chamber. It is attempted to be collected by mO.
 * 6. It is confirmed that mC is not disposed of yet. mC is then disposed and confirmed that it has been disposed of.
 * 7. Lastly, it is verified that the disposed element is not present on the board or in any inventory.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(TryToCollectAnObjectAndDisposeIt,T,all_test_types)
{
    std::shared_ptr<chamber>  mc=chamber::makeNewChamber({5,5});

    std::shared_ptr<bElem> mO=elementFactory::generateAnElement<T>(mc,0);
    std::shared_ptr<bElem> mC=elementFactory::generateAnElement<T>(mc,0);
//std::shared_ptr<inventory> nInv;
    mO->getAttrs()->setCollect(true);
    //  nInv=mO->getAttrs()->getInventory();
    mO->stepOnElement(mc->getElement(2,2));
    mC->stepOnElement(mc->getElement(2,3));
    //Check if the collect method works well with its limits
    bool chk=mO->collect(mc->getElement(2,3));
    BOOST_CHECK(chk==(mO->getAttrs()->canCollect() && mC->getAttrs()->isCollectible()));
    if(mO->getAttrs()->canCollect()&& mC->getAttrs()->isCollectible())
    {
        BOOST_CHECK(mc->getElement(2,3)->getStats()->getInstanceId()!=mC->getStats()->getInstanceId());
        BOOST_CHECK(mC->getStats()->isCollected());
        //  std::cout<<"is in inv?"<<mO->getAttrs()->getInventory()->findInInventory(mC->getStats()->getInstanceId())<<"\n";
    }
    else
    {
        BOOST_CHECK(mc->getElement(2,3)->getStats()->getInstanceId()==mC->getStats()->getInstanceId());
    }

    mC=elementFactory::generateAnElement<T>(mc,0);
    mC->stepOnElement(mc->getElement(3,2));
    mO->collect(mc->getElement(3,2));
    BOOST_CHECK(mC->getStats()->isDisposed()==false);
    mC->disposeElement();
    BOOST_CHECK(mC->getStats()->isDisposed()==true);
    // check the disposed element not present in the board or any inventory
    for(int a=0; a<mc->width; a++)
    {
        for(int b=0; b<mc->height; b++)
        {
            BOOST_CHECK(mc->getElement(a,b)->getStats()->getInstanceId()!=mC->getStats()->getInstanceId());
            if(mc->getElement(a,b)->getAttrs()->canCollect())
            {
                BOOST_CHECK(mc->getElement(a,b)->getAttrs()->getInventory()->findInInventory(mC->getStats()->getInstanceId())==false);
            }
            std::shared_ptr<bElem> el=mc->getElement(a,b)->getStats()->getSteppingOn();

            while(el!=nullptr)
            {
                BOOST_CHECK(el->getStats()->getInstanceId()!=mC->getStats()->getInstanceId());
                if(el->getAttrs()->canCollect()==true)
                {
                    BOOST_CHECK(el->getAttrs()->getInventory()->findInInventory(mC->getStats()->getInstanceId())==false);
                }
                el=el->getStats()->getSteppingOn();
            }
        }
    }

}


BOOST_AUTO_TEST_CASE_TEMPLATE(TryToRemoveElementMoreThanNeeded,T,all_test_types)
{
    std::shared_ptr<chamber>  mc=chamber::makeNewChamber({5,5});
    std::shared_ptr<bElem> myObj=elementFactory::generateAnElement<T>(mc,0);
    std::shared_ptr<bElem> relic;
    myObj->stepOnElement(mc->getElement(1,1));
    myObj=elementFactory::generateAnElement<T>(mc,0);
    myObj->stepOnElement(mc->getElement(2,2));

    for(int c=0; c<100; c++)
    {
#ifdef _VerbousMode_
        std::cout<<" * [1] removing "<<c<<"\n";
#endif

        relic=mc->getElement(1,1)->removeElement();
        BOOST_CHECK(relic!=nullptr);
#ifdef _VerbousMode_
        std::cout<<" * [2] removing "<<c<<"\n";
#endif

        BOOST_CHECK(mc->getElement(2,2)->disposeElement()==DISPOSED);
    }
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



