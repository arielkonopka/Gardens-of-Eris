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
 * @brief unit test for testing type of the object, its subtype, and that a hurt apple turns into a refresher
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
            // shot, it is a refresher now
            EXPECT_EQ(chamber->getElement(x, 0)->getType(), bElemTypes::_refresherType);
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
    EXPECT_EQ(chamber->getElement(point/2)->getType(), bElemTypes::_refresherType);
    chamber->getElement(point/2)->disposeElement();
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



/// a shot apple becomes a refresher on its cell, and stops counting as an apple
TEST(GoldenAppleTests, AShotAppleTurnsIntoARefresher)
{
    auto board = chamber::makeNewChamber(point);
    auto apple = elementFactory::generateAnElement<goldenApple>(board, 0);
    apple->stepOnElement(board->getElement(4, 4));
    const int before = goldenApple::getAppleNumber();
    EXPECT_TRUE(apple->hurt(1));
    EXPECT_TRUE(apple->getStats()->isDisposed());
    EXPECT_EQ(goldenApple::getAppleNumber(), before - 1);
    const auto there = board->getElement(4, 4);
    ASSERT_TRUE(there);
    EXPECT_EQ(there->getType(), bElemTypes::_refresherType);
    EXPECT_TRUE(there->getAttrs()->isCollectible());
    EXPECT_TRUE(there->getAttrs()->isInteractive());
    there->disposeElement();
}

/// using a refresher gives a dose of energy and costs it the same
TEST(GoldenAppleTests, ARefresherFeedsWhoUsesIt)
{
    auto board = chamber::makeNewChamber(point);
    auto fresh = elementFactory::generateAnElement<refresher>(board, 0);
    fresh->stepOnElement(board->getElement(4, 4));
    auto who = elementFactory::generateAnElement<monster>(board, 0);
    who->stepOnElement(board->getElement(5, 4));
    who->getAttrs()->setEnergy(20);
    const int stock = fresh->getAttrs()->getEnergy();
    bElem::tick();
    EXPECT_TRUE(fresh->interact(who));
    EXPECT_EQ(who->getAttrs()->getEnergy(), 20 + refresher::dose);
    EXPECT_EQ(fresh->getAttrs()->getEnergy(), stock - refresher::dose);
    // a drained one gives nothing
    fresh->getAttrs()->setEnergy(0);
    for (int i = 0; i < refresher::pause + 1; i++)
        bElem::tick();
    fresh->interact(who);
    EXPECT_EQ(who->getAttrs()->getEnergy(), 20 + refresher::dose);
    fresh->disposeElement();
    who->disposeElement();
}
