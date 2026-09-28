/*
 * Copyright (c) 2023, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
// *** ADDED BY HEADER FIXUP ***
#include <list>
// *** END ***
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include <gtest/gtest.h>
#include "testSupport.h"
#include <memory>


auto preClean=[](std::shared_ptr<chamber> ch,coords point)
{
    inputManager::getInstance(true);
    while (player::getActivePlayer()) player::getActivePlayer()->disposeElement();
    std::shared_ptr<bElem> pl=elementFactory::generateAnElement<player>(ch,0);
    pl->getStats()->setActive(true);
    pl->stepOnElement(ch->getElement(point));

};

TEST(TeleportObjectTests, TeleportAnObjectWithOneTeleport)
{

    std::shared_ptr<chamber> mc=chamber::makeNewChamber(myUtility::Coords(5,5));
    elementFactory::generateAnElement<teleport>(mc,0);
    std::shared_ptr<teleport> tel1=elementFactory::generateAnElement<teleport>(mc,1);
    std::shared_ptr<bElem> transportedE=elementFactory::generateAnElement<brickCluster>(mc,0);
    transportedE->stepOnElement(mc->getElement(2,3));
    //  transportedE->setActive(true);
    coords crds;
    transportedE->getStats()->setMyDirection(dir::direction::RIGHT);
    bElem::tick();
    bElem::tick();
    tel1->stepOnElement(mc->getElement(3,3));
    EXPECT_TRUE(tel1->interact(transportedE)==true);
    std::cout<<"teleporting1:"<<mc->getElement(2,3)->getStats()->isTeleporting();
    EXPECT_TRUE(mc->getElement(2,3)->getStats()->getInstanceId()!=transportedE->getStats()->getInstanceId());
    EXPECT_TRUE(mc->getElement(2,3)->getStats()->isTeleporting()==true);
    crds=transportedE->getStats()->getMyPosition();
    //  std::cout<<"x:"<<crds.x<<" y:"<<crds.y<<"\n";
    EXPECT_TRUE(mc->getElement(crds.x,crds.y)->getStats()->getInstanceId()==transportedE->getStats()->getInstanceId());
    EXPECT_TRUE(mc->getElement(crds.x,crds.y)->getStats()->isTeleporting()==true);
    for(int a=0; a<GoEConstants::_teleportationTime; a++)
    {
        EXPECT_TRUE(mc->getElement(2,3)->getStats()->isTeleporting()==true);
        EXPECT_TRUE(mc->getElement(crds.x,crds.y)->getStats()->isTeleporting()==true);
        bElem::tick();
    }
    EXPECT_TRUE(mc->getElement(crds.x,crds.y)->getStats()->isTeleporting()==false);
    EXPECT_TRUE(mc->getElement(2,3)->getStats()->isTeleporting()==false);
    tel1->disposeElement();

}
//We have two teleports of the same type
TEST(TeleportObjectTests, TeleportAnObjectWithTwoTeleportsOneChamber)
{
    coords crds;
    coords csize={10,10};
    coords ppoint={9,9};
    coords tel1c={1,1};
    coords tel2c={8,8};
    coords telc={1,2};
    std::shared_ptr<chamber> mc=chamber::makeNewChamber(csize);
    preClean(mc,ppoint);
    elementFactory::generateAnElement<teleport>(mc,777);
    std::shared_ptr<teleport> tel1=elementFactory::generateAnElement<teleport>(mc,2);
    std::shared_ptr<teleport>  tel2=elementFactory::generateAnElement<teleport>(mc,2);
    std::shared_ptr<bElem> transportEl=elementFactory::generateAnElement<brickCluster>(mc,0);
    transportEl->stepOnElement(mc->getElement(telc));
    bElem::tick();
    bElem::tick();
    transportEl->getStats()->setMyDirection(dir::direction::RIGHT);
    tel1->stepOnElement(mc->getElement(tel1c));
    tel2->stepOnElement(mc->getElement(tel2c));
    tel1->interact(transportEl);
    crds=transportEl->getStats()->getMyPosition();
    //std::cout<<"crds "<<crds.x<<","<<crds.y<<"\n";
    EXPECT_TRUE(crds.distance(tel2c)<=2);
    EXPECT_TRUE(transportEl->getStats()->isTeleporting()==true);
    for(int c=0; c<1000; c++)
        bElem::runLiveElements();
    EXPECT_TRUE(transportEl->getStats()->isTeleporting()==false);
    transportEl->getStats()->setMyDirection(dir::direction::LEFT);
    EXPECT_TRUE(tel2->interact(transportEl)==false);
    tel1->disposeElement();
    tel2->disposeElement();
}

TEST(TeleportObjectTests, TeleportAnObjectWithTwoTeleportsDifferentType)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber(myUtility::Coords(8,8));
    coords ncrds;
    coords t1b={1,0};
    coords  t2b={7,6};
    coords tel1c={0,0};
    coords tel2c={7,7};
    elementFactory::generateAnElement<teleport>(mc,777);
    std::shared_ptr<teleport>  tel1=elementFactory::generateAnElement<teleport>(mc,3);
    std::shared_ptr<teleport>  tel2=elementFactory::generateAnElement<teleport>(mc,4);
    std::shared_ptr<bElem> _tr1=elementFactory::generateAnElement<brickCluster>(mc,0);
    std::shared_ptr<bElem> _tr2=elementFactory::generateAnElement<brickCluster>(mc,0);
    _tr1->stepOnElement(mc->getElement(t1b));
    _tr2->stepOnElement(mc->getElement(t2b));
    tel1->stepOnElement(mc->getElement(tel1c));
    tel2->stepOnElement(mc->getElement(tel2c));
    _tr1->getStats()->setMyDirection(dir::direction::DOWN);
    _tr2->getStats()->setMyDirection(dir::direction::LEFT);
    tel1->interact(_tr1);
    tel2->interact(_tr2);
    coords tr1c=_tr1->getStats()->getMyPosition();
    EXPECT_TRUE(tr1c!=t1b);
    /// check the distance from desired teleport
    EXPECT_TRUE(tr1c.distance(tel1c)<=2);
    coords tr2c=_tr2->getStats()->getMyPosition();
    EXPECT_TRUE(tr2c!=t2b);
    EXPECT_TRUE(tr2c.distance(tel2c)<=2);
    tel1->disposeElement();
    tel2->disposeElement();
    mc.reset();
}

TEST(TeleportObjectTests, WalkInTeleportTests)
{
    std::shared_ptr<chamber> mc=chamber::makeNewChamber(myUtility::Coords(10,10));
    coords pointA= {3,5};
    coords pointAt= {2,5};
    coords pointB= {5,5};
    preClean(mc,{9,9});
    std::shared_ptr<bElem> tel1,tel2,transported;
    bElem::tick();
    transported=elementFactory::generateAnElement<brickCluster>(mc,0);
    transported->stepOnElement(mc->getElement(pointAt));
    transported->getStats()->setMyDirection(dir::direction::RIGHT);
    EXPECT_TRUE(transported->getStats()->isTeleporting()==false);
    tel1=elementFactory::generateAnElement<teleport>(mc,5);
    tel2=elementFactory::generateAnElement<teleport>(mc,5);
    tel1->stepOnElement(mc->getElement(pointA));
    tel2->stepOnElement(mc->getElement(pointB));
    //ok now step on that teleport
    transported->stepOnElement(tel1);
    EXPECT_TRUE(tel1->getStats()->hasActivatedMechanics()==true);
    EXPECT_TRUE(transported->getStats()->getMyPosition()==pointA);
    EXPECT_TRUE(mc->getElement(pointA)->getStats()->getInstanceId()==transported->getStats()->getInstanceId());
    for(int c=0; c<GoEConstants::_teleportStandTime; c++)
    {
        EXPECT_TRUE(transported->getStats()->isTeleporting()==false);
        bElem::runLiveElements();
    }
    EXPECT_TRUE(transported->getStats()->isTeleporting()==true);
    tel1->disposeElement();
    tel2->disposeElement();
    mc.reset();
}



