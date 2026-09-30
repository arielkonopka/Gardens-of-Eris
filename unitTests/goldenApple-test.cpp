// *** ADDED BY HEADER FIXUP ***
#include <list>
// *** END ***
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include <gtest/gtest.h>
#include "testSupport.h"

const coords point(10, 10);
/***
 * @brief unit test for testing type of the object, its subtype, and the change in subtype after being hurt.
 * We also test whether the interactive flag changed
 */
TEST(GoldenAppleTests, GetTypeTest)
{

    std::shared_ptr<chamber> chamber = chamber::makeNewChamber(point);
    std::shared_ptr<goldenApple> goldenAppleObj = elementFactory::generateAnElement<goldenApple>(chamber,0);
    for(int x=0; x<point.x; x++)
    {
        std::shared_ptr<goldenApple> goldenAppleObj2 = elementFactory::generateAnElement<goldenApple>(chamber,0);
        goldenAppleObj2->stepOnElement(chamber->getElement(x,0));
        if(x%3==2)
        {
            EXPECT_TRUE(goldenAppleObj2->getAttrs()->getSubtype()==0);
            EXPECT_TRUE(!goldenAppleObj2->getAttrs()->isInteractive());
            goldenAppleObj2->hurt(1);
            goldenAppleObj2->hurt(1);
            EXPECT_TRUE(goldenAppleObj2->getAttrs()->getSubtype()!=0);
            EXPECT_TRUE(goldenAppleObj2->getAttrs()->isInteractive());
        }
    }
    bElem::tick();
    bElem::tick();
    EXPECT_TRUE(chamber->getElement(point/2)->getType()!=bElemTypes::_goldenAppleType);
    goldenAppleObj->stepOnElement(chamber->getElement(point/2));
    EXPECT_TRUE(goldenAppleObj->getType()==bElemTypes::_goldenAppleType);
    EXPECT_EQ(chamber->getElement(point/2)->getType(), bElemTypes::_goldenAppleType);
    EXPECT_TRUE(goldenAppleObj->getAttrs()->getSubtype()==0);
    EXPECT_TRUE(!goldenAppleObj->getAttrs()->isInteractive());
    goldenAppleObj->hurt(1);
    EXPECT_TRUE(goldenAppleObj->getAttrs()->getSubtype()!=0);
    EXPECT_TRUE(goldenAppleObj->getAttrs()->isInteractive());
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
TEST(GoldenAppleTests, KillTest)
{
    std::shared_ptr<chamber> chamber = chamber::makeNewChamber(point);
    std::shared_ptr<goldenApple> goldenAppleObj = elementFactory::generateAnElement<goldenApple>(chamber,0);
    std::shared_ptr<bElem> be=nullptr;
    goldenAppleObj->stepOnElement(chamber->getElement(point/2));
    bElem::tick();
    EXPECT_TRUE(goldenAppleObj->kill());
    EXPECT_TRUE(!goldenAppleObj->kill());
    EXPECT_TRUE(goldenAppleObj->getStats()->isDestroying());
    for(int x=0; x<4; x++)
    {
        EXPECT_TRUE(goldenAppleObj->getElementInDirection((dir::direction)x) && goldenAppleObj->getElementInDirection((dir::direction)x)->getStats()->isDestroying());
        EXPECT_TRUE(goldenAppleObj->getElementInDirection((dir::direction)x) && goldenAppleObj->getElementInDirection((dir::direction)x)->getElementInDirection((dir::direction)((x+3)%4))->getStats()->isDestroying());
    }
    goldenAppleObj->disposeElement();
}

/***
 * @brief Unit test of destroy method
 * We test here only close neighborhood of the apple on the board, when exploding it
 */
TEST(GoldenAppleTests, DestroyTest)
{
    std::shared_ptr<chamber> chamber = chamber::makeNewChamber(point);
    std::shared_ptr<goldenApple> goldenAppleObj = elementFactory::generateAnElement<goldenApple>(chamber,0);
    std::shared_ptr<bElem> be=nullptr;
    goldenAppleObj->stepOnElement(chamber->getElement(point/2));
    bElem::tick();
    EXPECT_TRUE(goldenAppleObj->destroy());
    EXPECT_TRUE(goldenAppleObj->getStats()->isDestroying());
    for(int x=0; x<4; x++)
    {
        EXPECT_TRUE(goldenAppleObj->getElementInDirection((dir::direction)x)->getStats()->isDestroying());
        EXPECT_TRUE(goldenAppleObj->getElementInDirection((dir::direction)x)->getElementInDirection((dir::direction)((x+3)%4))->getStats()->isDestroying());
    }
    goldenAppleObj->disposeElement();
}

TEST(GoldenAppleTests, GetAppleNumberTest)
{
//increase number of apples, decrease the number of apples in different ways
}


