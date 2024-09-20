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

coords point=(coords)
{
    10,10
};
BOOST_AUTO_TEST_SUITE(GoldenAppleTests)
/***
 * @brief unit test for testing type of the object, its subtype, and the change in subtype after being hurt.
 * We also test whether the interactive flag changed
 */
BOOST_AUTO_TEST_CASE(GetTypeTest)
{

    std::shared_ptr<chamber> chamber = chamber::makeNewChamber(point);
    std::shared_ptr<goldenApple> goldenAppleObj = elementFactory::generateAnElement<goldenApple>(chamber,0);
    for(int x=0; x<point.x; x++)
    {
        std::shared_ptr<goldenApple> goldenAppleObj2 = elementFactory::generateAnElement<goldenApple>(chamber,0);
        goldenAppleObj2->stepOnElement(chamber->getElement(x,0));
        if(x%3==2)
        {
            BOOST_CHECK(goldenAppleObj2->getAttrs()->getSubtype()==0);
            BOOST_CHECK(!goldenAppleObj2->getAttrs()->isInteractive());
            goldenAppleObj2->hurt(1);
            goldenAppleObj2->hurt(1);
            BOOST_CHECK(goldenAppleObj2->getAttrs()->getSubtype()!=0);
            BOOST_CHECK(goldenAppleObj2->getAttrs()->isInteractive());
        }
    }
    bElem::tick();
    bElem::tick();
    BOOST_CHECK(chamber->getElement(point/2)->getType()!=bElemTypes::_goldenAppleType);
    goldenAppleObj->stepOnElement(chamber->getElement(point/2));
    BOOST_CHECK(goldenAppleObj->getType()==bElemTypes::_goldenAppleType);
    BOOST_CHECK_EQUAL(chamber->getElement(point/2)->getType(),bElemTypes::_goldenAppleType);
    BOOST_CHECK(goldenAppleObj->getAttrs()->getSubtype()==0);
    BOOST_CHECK(!goldenAppleObj->getAttrs()->isInteractive());
    goldenAppleObj->hurt(1);
    BOOST_CHECK(goldenAppleObj->getAttrs()->getSubtype()!=0);
    BOOST_CHECK(goldenAppleObj->getAttrs()->isInteractive());
    goldenAppleObj->disposeElement();
    for(int x=0; x<point.x; x++)
    {
        chamber->getElement(x,0)->disposeElement();
    }
    // Add more assertions or scenarios if needed
}

/***
 * @brief Unit test of kill method
 * We test here only close neighborhood of the apple on the board, when exploding it.
 */
BOOST_AUTO_TEST_CASE(KillTest)
{
    std::shared_ptr<chamber> chamber = chamber::makeNewChamber(point);
    std::shared_ptr<goldenApple> goldenAppleObj = elementFactory::generateAnElement<goldenApple>(chamber,0);
    std::shared_ptr<bElem> be=nullptr;
    goldenAppleObj->stepOnElement(chamber->getElement(point/2));
    bElem::tick();
    BOOST_CHECK(goldenAppleObj->kill());
    BOOST_CHECK(!goldenAppleObj->kill());
    BOOST_CHECK(goldenAppleObj->getStats()->isDestroying());
    for(int x=0; x<4; x++)
    {
        BOOST_CHECK(goldenAppleObj->getElementInDirection((dir::direction)x) && goldenAppleObj->getElementInDirection((dir::direction)x)->getStats()->isDestroying());
        BOOST_CHECK(goldenAppleObj->getElementInDirection((dir::direction)x) && goldenAppleObj->getElementInDirection((dir::direction)x)->getElementInDirection((dir::direction)((x+3)%4))->getStats()->isDestroying());
    }
    goldenAppleObj->disposeElement();
}

/***
 * @brief Unit test of destroy method
 * We test here only close neighborhood of the apple on the board, when exploding it
 */
BOOST_AUTO_TEST_CASE(DestroyTest)
{
    std::shared_ptr<chamber> chamber = chamber::makeNewChamber(point);
    std::shared_ptr<goldenApple> goldenAppleObj = elementFactory::generateAnElement<goldenApple>(chamber,0);
    std::shared_ptr<bElem> be=nullptr;
    goldenAppleObj->stepOnElement(chamber->getElement(point/2));
    bElem::tick();
    BOOST_CHECK(goldenAppleObj->destroy());
    BOOST_CHECK(goldenAppleObj->getStats()->isDestroying());
    for(int x=0; x<4; x++)
    {
        BOOST_CHECK(goldenAppleObj->getElementInDirection((dir::direction)x)->getStats()->isDestroying());
        BOOST_CHECK(goldenAppleObj->getElementInDirection((dir::direction)x)->getElementInDirection((dir::direction)((x+3)%4))->getStats()->isDestroying());
    }
    goldenAppleObj->disposeElement();
}

BOOST_AUTO_TEST_CASE(GetAppleNumberTest)
{
//increase number of apples, decrease the number of apples in different ways
}


BOOST_AUTO_TEST_SUITE_END()
