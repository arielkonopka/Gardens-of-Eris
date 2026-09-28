#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include <gtest/gtest.h>
#include "testSupport.h"
#include <cstdlib>
#include <memory>
#include <vector>
#include <algorithm>

namespace {
struct rig
{
    std::shared_ptr<chamber> mc;
    std::shared_ptr<bElem> plr;
    std::shared_ptr<patrollingDrone> drone;
    std::shared_ptr<puppetMasterFR> brain;
};

/// a walled room with a player at (3,3) and a drone at (dx,dy) driven by a controller of the given kind
rig makeRig(int kind, int size, int dx, int dy)
{
    rig r;
    inputManager::getInstance(true);
    r.mc = chamber::makeNewChamber(coords(size, size));
    for (int c = 0; c < size; c++)
        for (auto p : {coords(c, 0), coords(c, size - 1), coords(0, c), coords(size - 1, c)})
            elementFactory::generateAnElement<wall>(r.mc, 0)->stepOnElement(r.mc->getElement(p));
    // earlier tests leave their player active, and only the active player's chamber ticks
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    r.plr = elementFactory::generateAnElement<player>(r.mc, 0);
    r.plr->stepOnElement(r.mc->getElement(3, 3));
    r.plr->getStats()->setActive(true);
    r.brain = puppetMasterFR::create(r.mc, kind);
    r.brain->stepOnElement(r.mc->getElement(4, 3));
    REQUIRE_IN_HELPER(r.plr->collect(r.brain));
    r.drone = elementFactory::generateAnElement<patrollingDrone>(r.mc, 0);
    r.drone->stepOnElement(r.mc->getElement(dx, dy));
    REQUIRE_IN_HELPER(r.drone->interact(r.plr));
    return r;
}

int distance(const std::shared_ptr<bElem> &a, const std::shared_ptr<bElem> &b)
{
    auto pa = a->getStats()->getMyPosition(), pb = b->getStats()->getMyPosition();
    return std::abs(pa.x - pb.x) + std::abs(pa.y - pb.y);
}

/// an empty walled room with the active player at player position and a camera at cam
std::shared_ptr<chamber> cameraRoom(coords size, coords plrAt, coords camAt, std::shared_ptr<bElem> &plr,
                                    std::shared_ptr<securityCamera> &cam)
{
    inputManager::getInstance(true);
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    auto mc = chamber::makeNewChamber(size);
    for (int x = 0; x < size.x; x++)
        for (auto p : {coords(x, 0), coords(x, size.y - 1)})
            elementFactory::generateAnElement<wall>(mc, 0)->stepOnElement(mc->getElement(p));
    for (int y = 0; y < size.y; y++)
        for (auto p : {coords(0, y), coords(size.x - 1, y)})
            elementFactory::generateAnElement<wall>(mc, 0)->stepOnElement(mc->getElement(p));
    plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(plrAt));
    cam = elementFactory::generateAnElement<securityCamera>(mc, 0);
    cam->stepOnElement(mc->getElement(camAt));
    return mc;
}

std::vector<std::shared_ptr<patrollingDrone>> guardiansOf(const std::shared_ptr<chamber> &mc,
                                                          const std::shared_ptr<securityCamera> &cam)
{
    std::vector<std::shared_ptr<patrollingDrone>> res;
    for (int x = 0; x < mc->getSize().x; x++)
        for (int y = 0; y < mc->getSize().y; y++)
            if (auto d = std::dynamic_pointer_cast<patrollingDrone>(mc->getElement(x, y)))
                if (auto g = std::dynamic_pointer_cast<puppetMasterGuardian>(d->getBrainModule()))
                    if (g->getCamera() == cam)
                        res.push_back(d);
    return res;
}
} // namespace


TEST(ControllerTests, FactoryCreatesTheRightKind)
{
    auto mc = chamber::makeNewChamber(coords(5, 5));
    EXPECT_TRUE(typeid(*puppetMasterFR::create(mc, puppetMasterFR::patrol)) == typeid(puppetMasterFR));
    EXPECT_TRUE(std::dynamic_pointer_cast<puppetMasterCollector>(puppetMasterFR::create(mc, puppetMasterFR::collector)));
    EXPECT_TRUE(std::dynamic_pointer_cast<puppetMasterHunter>(puppetMasterFR::create(mc, puppetMasterFR::hunter)));
    EXPECT_TRUE(std::dynamic_pointer_cast<puppetMasterWallFollower>(puppetMasterFR::create(mc, puppetMasterFR::wallFollower)));
    for (int k = 0; k < puppetMasterFR::kindCount; k++) {
        auto c = puppetMasterFR::create(mc, k);
        EXPECT_EQ(c->getType(), bElemTypes::_puppetMasterType);
        EXPECT_EQ(c->getAttrs()->getSubtype(), k);
    }
}

TEST(ControllerTests, EveryKindDrivesItsDrone)
{
    for (int k = 0; k < puppetMasterFR::kindCount; k++) {
        auto r = makeRig(k, 20, 12, 12);
        EXPECT_TRUE(r.drone->getBrainModule() == r.brain);
        EXPECT_TRUE(r.brain->getStats()->getCollector().lock() == r.drone);
        auto start = r.drone->getStats()->getMyPosition();
        bool moved = false;
        for (int c = 0; c < 2000 && !moved; c++) {
            bElem::runLiveElements();
            moved = !(r.drone->getStats()->getMyPosition() == start);
        }
        EXPECT_TRUE(moved) << "controller kind " << k << " never moved its drone";
    }
}

TEST(ControllerTests, HunterClosesInOnThePlayer)
{
    auto r = makeRig(puppetMasterFR::hunter, 20, 9, 9);
    int before = distance(r.drone, r.plr);
    for (int c = 0; c < 400 && distance(r.drone, r.plr) > 1; c++)
        bElem::runLiveElements();
    EXPECT_LT(distance(r.drone, r.plr), before);
    EXPECT_LE(distance(r.drone, r.plr), 2);
}

TEST(ControllerTests, WallFollowerReachesTheWalls)
{
    auto r = makeRig(puppetMasterFR::wallFollower, 20, 10, 10);
    // in an empty room it must not circle in the middle: it reaches a wall and then follows it
    bool touchedWall = false;
    for (int c = 0; c < 1500 && !touchedWall; c++) {
        bElem::runLiveElements();
        auto p = r.drone->getStats()->getMyPosition();
        touchedWall = p.x == 1 || p.y == 1 || p.x == 18 || p.y == 18;
    }
    EXPECT_TRUE(touchedWall);
}

TEST(ControllerTests, CameraSpawnsItsGuardians)
{
    std::shared_ptr<bElem> plr;
    std::shared_ptr<securityCamera> cam;
    auto mc = cameraRoom(coords(40, 40), coords(2, 2), coords(30, 30), plr, cam);
    for (int c = 0; c < 3; c++)
        bElem::runLiveElements();
    EXPECT_EQ(guardiansOf(mc, cam).size(), (size_t) difficulty::guardianCount(0));
    // the player is far away: no alarm
    EXPECT_EQ(cam->getAlertNumber(), 0u);
}

TEST(ControllerTests, CameraCallsGuardiansAndTheyAttack)
{
    std::shared_ptr<bElem> plr;
    std::shared_ptr<securityCamera> cam;
    // the player stands 7 cells from the camera, in plain view
    auto mc = cameraRoom(coords(40, 40), coords(13, 20), coords(20, 20), plr, cam);
    int energy = plr->getAttrs()->getEnergy();
    bool hurt = false;
    for (int c = 0; c < 1500 && !hurt; c++) {
        bElem::runLiveElements();
        hurt = plr->getStats()->isDying() || plr->getAttrs()->getEnergy() < energy;
    }
    EXPECT_GT(cam->getAlertNumber(), 0u);
    EXPECT_TRUE(cam->getAlertPosition() == coords(13, 20));
    EXPECT_TRUE(hurt);
}

TEST(ControllerTests, CameraDoesNotSeeThroughWalls)
{
    std::shared_ptr<bElem> plr;
    std::shared_ptr<securityCamera> cam;
    auto mc = cameraRoom(coords(40, 40), coords(13, 20), coords(20, 20), plr, cam);
    for (int y = 1; y < 39; y++)
        elementFactory::generateAnElement<wall>(mc, 0)->stepOnElement(mc->getElement(16, y));
    for (int c = 0; c < 300; c++)
        bElem::runLiveElements();
    EXPECT_EQ(cam->getAlertNumber(), 0u);
}

TEST(ControllerTests, GuardiansStayOnTheLeash)
{
    std::shared_ptr<bElem> plr;
    std::shared_ptr<securityCamera> cam;
    // a long corridor, the player at its far end, well outside the leash
    auto mc = cameraRoom(coords(150, 7), coords(140, 3), coords(5, 3), plr, cam);
    int furthest = 0;
    for (int c = 0; c < 6000; c++) {
        bElem::runLiveElements();
        for (auto &g : guardiansOf(mc, cam))
            furthest = std::max(furthest, g->getStats()->getMyPosition().x - 5);
    }
    EXPECT_GT(furthest, 5); // they do patrol
    EXPECT_LE(furthest, securityCamera::leash);
}

