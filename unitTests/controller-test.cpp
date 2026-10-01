#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include <gtest/gtest.h>
#include "testSupport.h"
#include "elementSound.h"
#include "configManager.h"
#include "lineOfSight.h"
#include <cstdlib>
#include <filesystem>
#include <set>
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


/* ---------------------------------------------------- chasers do not cheat (2026-10-01) */

TEST(ControllerTests, SightIsBlockedByWallsAndClosedDoorsButNotByItems)
{
    auto mc = chamber::makeNewChamber(coords(10, 10));
    EXPECT_TRUE(goe::sight::clear(mc, coords(1, 1), coords(8, 8)));
    elementFactory::generateAnElement<goldenApple>(mc, 0)->stepOnElement(mc->getElement(4, 4));
    EXPECT_TRUE(goe::sight::clear(mc, coords(1, 1), coords(8, 8))) << "an apple does not hide anyone";
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    w->stepOnElement(mc->getElement(6, 6));
    EXPECT_FALSE(goe::sight::clear(mc, coords(1, 1), coords(8, 8)));
    EXPECT_TRUE(goe::sight::clear(mc, coords(1, 1), coords(6, 6))) << "the end cells never block";

    auto d = elementFactory::generateAnElement<door>(mc, 0);
    d->stepOnElement(mc->getElement(5, 1));
    ASSERT_FALSE(d->getAttrs()->isSteppable());
    EXPECT_FALSE(goe::sight::clear(mc, coords(1, 1), coords(8, 1)));

    // two walls touching at their corners leave no gap to look through
    elementFactory::generateAnElement<wall>(mc, 0)->stepOnElement(mc->getElement(2, 8));
    EXPECT_TRUE(goe::sight::clear(mc, coords(1, 8), coords(2, 7)));
    elementFactory::generateAnElement<wall>(mc, 0)->stepOnElement(mc->getElement(1, 7));
    EXPECT_FALSE(goe::sight::clear(mc, coords(1, 8), coords(2, 7)));
    // the dark beyond the board blocks too
    EXPECT_FALSE(goe::sight::clear(mc, coords(1, 1), coords(1, 12)));
}

// The hunter, a guardian and the Hound only learn where the player is by seeing them. A wall with
// a gap at its far end stands between them and the player: a cheater would know the player is there
// and walk round through the gap; a fair chaser does not know, and patrols the walls instead.
// Without the wall they spot the player at once.
TEST(ControllerTests, ChasersDoNotSeeThroughWalls)
{
    for (int kind : {(int) puppetMasterFR::hunter, (int) puppetMasterFR::guardian, (int) puppetMasterFR::hound}) {
        for (bool walled : {true, false}) {
            auto r = makeRig(kind, 20, 9, 3);
            if (walled)
                for (int y = 1; y < 17; y++)
                    elementFactory::generateAnElement<wall>(r.mc, 0)->stepOnElement(r.mc->getElement(7, y));
            bool spotted = false, alongWall = false;
            // from its side of the wall (x > 7) the drone cannot see the player at (3,3)
            for (int c = 0; c < 1500 && !spotted && r.drone->getStats()->getMyPosition().x > 7; c++) {
                bElem::runLiveElements();
                spotted = !(r.brain->getLastSeen() == NOCOORDS);
                auto p = r.drone->getStats()->getMyPosition();
                alongWall = alongWall || p.x == 8 || p.x == 18 || p.y == 1 || p.y == 18;
            }
            if (walled) {
                EXPECT_FALSE(spotted) << "kind " << kind << " saw the player through a wall";
                EXPECT_TRUE(alongWall) << "kind " << kind << " should patrol the walls";
            } else {
                EXPECT_TRUE(spotted) << "kind " << kind << " never saw the player in plain view";
            }
        }
    }
}

// A hunter that loses sight of the player goes to where it saw them last; when they are not there,
// the trail goes cold and it patrols. It never learns where the player went while out of sight.
TEST(ControllerTests, HunterChecksWhereThePlayerWasLastSeenThenPatrols)
{
    auto r = makeRig(puppetMasterFR::hunter, 30, 12, 3);
    const coords seenAt(3, 3), hideout(25, 25);
    for (int c = 0; c < 200 && !(r.brain->getLastSeen() == seenAt); c++)
        bElem::runLiveElements();
    ASSERT_TRUE(r.brain->getLastSeen() == seenAt);

    // the player slips away into a closed box of walls
    r.plr->stepOnElement(r.mc->getElement(hideout));
    for (int x = hideout.x - 1; x <= hideout.x + 1; x++)
        for (int y = hideout.y - 1; y <= hideout.y + 1; y++)
            if (!(coords(x, y) == hideout))
                elementFactory::generateAnElement<wall>(r.mc, 0)->stepOnElement(r.mc->getElement(x, y));

    bool checked = false;
    for (int c = 0; c < 600 && !checked; c++) {
        bElem::runLiveElements();
        ASSERT_FALSE(r.brain->getLastSeen() == hideout) << "the hunter cannot know where the player hid";
        auto p = r.drone->getStats()->getMyPosition();
        checked = std::abs(p.x - seenAt.x) + std::abs(p.y - seenAt.y) <= 1;
    }
    ASSERT_TRUE(checked) << "the hunter never went to where it saw the player";
    for (int c = 0; c < 50; c++)
        bElem::runLiveElements();
    EXPECT_TRUE(r.brain->getLastSeen() == NOCOORDS) << "nobody there: the trail goes cold";
    // and it patrols on
    auto at = r.drone->getStats()->getMyPosition();
    bool moved = false;
    for (int c = 0; c < 300 && !moved; c++) {
        bElem::runLiveElements();
        moved = !(r.drone->getStats()->getMyPosition() == at);
    }
    EXPECT_TRUE(moved);
}

// Every controller kind says "controller enabled" in its own language when it takes over a drone.
TEST(ControllerTests, EveryKindAnnouncesItselfInItsOwnVoice)
{
    auto cfg = configManager::getInstance()->getConfig();
    std::set<std::string> files;
    for (int kind = 0; kind < puppetMasterFR::kindCount; kind++) {
        const auto &sample = cfg->samples[bElemTypes::_puppetMasterType][kind]["Controller"]["Enabled"];
        ASSERT_TRUE(sample.configured) << "kind " << kind;
        EXPECT_TRUE(std::filesystem::exists(sample.fname)) << sample.fname;
        files.insert(sample.fname);
    }
    EXPECT_EQ(files.size(), (size_t) puppetMasterFR::kindCount) << "each kind needs its own language";

    for (int kind = 0; kind < puppetMasterFR::looseKinds; kind++) {
        std::vector<int> announced;
        goe::sound::observe([&announced](const bElem &e, const std::string &type, const std::string &event) {
            if (e.getType() == bElemTypes::_puppetMasterType && type == "Controller" && event == "Enabled")
                announced.push_back(e.getAttrs()->getSubtype());
        });
        auto r = makeRig(kind, 9, 5, 5);
        goe::sound::observe({});
        EXPECT_EQ(announced, std::vector<int>{kind});
    }
}
