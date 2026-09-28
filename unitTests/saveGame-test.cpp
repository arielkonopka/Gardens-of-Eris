#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "gameSerializer.h"
#include "randomLevelGenerator.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>

/// a path in the system temp folder (/tmp is not a real folder for native Windows programs)
static std::string tmpFile(const std::string &name)
{
    return (std::filesystem::temp_directory_path() / name).string();
}

namespace {
std::string readFile(const std::string &name)
{
    std::ifstream in(name, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

/// the instance counter (bytes 16..23) grows with every load, everything else must survive a round trip
std::string withoutCounter(std::string s)
{
    if (s.size() >= 24)
        std::fill(s.begin() + 16, s.begin() + 24, '\0');
    return s;
}

/// type -> count over every stack of every chamber
std::map<int, int> census()
{
    std::map<int, int> res;
    for (const auto &c : chamber::allChambers)
        for (int x = 0; x < c->getSize().x; x++)
            for (int y = 0; y < c->getSize().y; y++)
                for (auto e = c->getElement(x, y); e; e = e->getStats()->getSteppingOn())
                    res[e->getType()]++;
    return res;
}

std::shared_ptr<chamber> findChamber(int id)
{
    for (const auto &c : chamber::allChambers)
        if (c->getInstanceId() == id)
            return c;
    return nullptr;
}
} // namespace

BOOST_AUTO_TEST_SUITE(SaveGameTests)

BOOST_AUTO_TEST_CASE(SmallWorldRoundTrip)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(12, 12));
    auto plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(3, 3));
    plr->getStats()->setActive(true);
    auto gun = elementFactory::generateAnElement<plainGun>(mc, 0);
    gun->stepOnElement(mc->getElement(4, 3));
    BOOST_REQUIRE(plr->collect(gun));
    auto k = elementFactory::generateAnElement<key>(mc, 2);
    k->stepOnElement(mc->getElement(5, 5));
    auto w = elementFactory::generateAnElement<wall>(mc, 0);
    w->stepOnElement(mc->getElement(0, 0));
    plr->getStats()->setPoints(COLLECTS, 7);

    auto activeId = player::getActivePlayer()->getStats()->getInstanceId();
    auto plrId = plr->getStats()->getInstanceId();
    auto gunId = gun->getStats()->getInstanceId();
    auto keyId = k->getStats()->getInstanceId();
    auto chamberId = mc->getInstanceId();
    auto chambersBefore = chamber::allChambers.size();
    auto censusBefore = census();

    const std::string f1 = tmpFile("goe-roundtrip-1.goe"), f2 = tmpFile("goe-roundtrip-2.goe");
    BOOST_REQUIRE(gameSerializer::saveGame(f1));
    plr.reset();
    gun.reset();
    k.reset();
    w.reset();
    mc.reset();
    BOOST_REQUIRE(gameSerializer::loadGame(f1));

    BOOST_CHECK_EQUAL(chamber::allChambers.size(), chambersBefore);
    BOOST_CHECK(census() == censusBefore);
    auto lc = findChamber(chamberId);
    BOOST_REQUIRE(lc);
    auto lp = lc->getElement(3, 3);
    BOOST_REQUIRE(lp);
    BOOST_CHECK_EQUAL(lp->getType(), bElemTypes::_player);
    BOOST_CHECK_EQUAL(lp->getStats()->getInstanceId(), plrId);
    BOOST_CHECK(lp->getBoard() == lc);
    BOOST_CHECK_EQUAL(lp->getStats()->getPoints(COLLECTS), 7);
    BOOST_REQUIRE(player::getActivePlayer());
    BOOST_CHECK_EQUAL(player::getActivePlayer()->getStats()->getInstanceId(), activeId);
    auto lgun = lp->getAttrs()->getInventory()->getActiveWeapon();
    BOOST_REQUIRE(lgun);
    BOOST_CHECK_EQUAL(lgun->getStats()->getInstanceId(), gunId);
    BOOST_CHECK(lgun->getStats()->isCollected());
    BOOST_CHECK(lgun->getStats()->getCollector().lock() == lp);
    // the key stands on a floor, and the floor knows it
    auto lk = lc->getElement(5, 5);
    BOOST_REQUIRE(lk);
    BOOST_CHECK_EQUAL(lk->getStats()->getInstanceId(), keyId);
    BOOST_CHECK_EQUAL(lk->getAttrs()->getSubtype(), 2);
    auto under = lk->getStats()->getSteppingOn();
    BOOST_REQUIRE(under);
    BOOST_CHECK_EQUAL(under->getType(), bElemTypes::_floorType);
    BOOST_CHECK(under->getStats()->getStandingOn().lock() == lk);
    BOOST_CHECK(under->getStats()->getMyPosition() == coords(5, 5));
    BOOST_CHECK_EQUAL(lc->getElement(0, 0)->getType(), bElemTypes::_wallType);

    // saving the loaded world again gives the same file
    BOOST_REQUIRE(gameSerializer::saveGame(f2));
    BOOST_CHECK(withoutCounter(readFile(f1)) == withoutCounter(readFile(f2)));

    // the loaded world is playable
    for (int c = 0; c < 200; c++)
        bElem::runLiveElements();
    std::remove(f1.c_str());
    std::remove(f2.c_str());
}

BOOST_AUTO_TEST_CASE(GeneratedLevelRoundTrip)
{
    inputManager::getInstance(true);
    auto rl = new randomLevelGenerator(120, 120);
    rl->generateLevel(5);
    delete rl;
    // let the level live a little, so timers, missiles and dying elements exist
    for (int c = 0; c < 300; c++)
        bElem::runLiveElements();

    auto censusBefore = census();
    auto apples = goldenApple::getAppleNumber();
    const std::string f1 = tmpFile("goe-level-1.goe"), f2 = tmpFile("goe-level-2.goe");
    BOOST_REQUIRE(gameSerializer::saveGame(f1));
    BOOST_REQUIRE(gameSerializer::loadGame(f1));
    BOOST_CHECK(census() == censusBefore);
    BOOST_CHECK_EQUAL(goldenApple::getAppleNumber(), apples);
    BOOST_REQUIRE(gameSerializer::saveGame(f2));
    BOOST_CHECK(withoutCounter(readFile(f1)) == withoutCounter(readFile(f2)));

    // mechanics keep running on the restored world
    for (int c = 0; c < 300; c++)
        bElem::runLiveElements();
    std::remove(f1.c_str());
    std::remove(f2.c_str());
}

BOOST_AUTO_TEST_CASE(BadFilesLeaveTheWorldAlone)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(8, 8));
    auto plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(2, 2));
    const std::string good = tmpFile("goe-good.goe"), bad = tmpFile("goe-bad.goe"),
                      cut = tmpFile("goe-cut.goe");
    BOOST_REQUIRE(gameSerializer::saveGame(good));
    auto chambersBefore = chamber::allChambers.size();
    auto censusBefore = census();
    auto active = player::getActivePlayer();

    BOOST_CHECK(!gameSerializer::loadGame(tmpFile("goe-does-not-exist.goe")));
    {
        std::ofstream out(bad, std::ios::binary);
        out << "this is not a save file at all";
    }
    BOOST_CHECK(!gameSerializer::loadGame(bad));
    {
        auto data = readFile(good);
        std::ofstream out(cut, std::ios::binary);
        out.write(data.data(), (std::streamsize) data.size() / 2);
    }
    BOOST_CHECK(!gameSerializer::loadGame(cut));

    BOOST_CHECK_EQUAL(chamber::allChambers.size(), chambersBefore);
    BOOST_CHECK(census() == censusBefore);
    BOOST_CHECK(player::getActivePlayer() == active);
    std::remove(good.c_str());
    std::remove(bad.c_str());
    std::remove(cut.c_str());
}

BOOST_AUTO_TEST_CASE(ControlledDroneKeepsItsController)
{
    inputManager::getInstance(true);
    // earlier tests leave their player active, and only the active player's chamber ticks
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    auto mc = chamber::makeNewChamber(coords(20, 20));
    auto plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(3, 3));
    plr->getStats()->setActive(true);
    auto brain = puppetMasterFR::create(mc, puppetMasterFR::collector);
    brain->stepOnElement(mc->getElement(4, 3));
    BOOST_REQUIRE(plr->collect(brain));
    auto drone = elementFactory::generateAnElement<patrollingDrone>(mc, 0);
    drone->stepOnElement(mc->getElement(10, 10));
    BOOST_REQUIRE(drone->interact(plr));
    auto droneId = drone->getStats()->getInstanceId();
    auto brainId = brain->getStats()->getInstanceId();

    const std::string f = tmpFile("goe-drone.goe");
    BOOST_REQUIRE(gameSerializer::saveGame(f));
    plr.reset();
    brain.reset();
    drone.reset();
    mc.reset();
    BOOST_REQUIRE(gameSerializer::loadGame(f));
    std::remove(f.c_str());

    std::shared_ptr<bElem> ld;
    for (const auto &c : chamber::allChambers)
        for (int x = 0; x < c->getSize().x && !ld; x++)
            for (int y = 0; y < c->getSize().y && !ld; y++)
                for (auto e = c->getElement(x, y); e; e = e->getStats()->getSteppingOn())
                    if (e->getStats()->getInstanceId() == droneId)
                        ld = e;
    BOOST_REQUIRE(ld);
    auto d = std::dynamic_pointer_cast<patrollingDrone>(ld);
    BOOST_REQUIRE(d);
    auto lb = d->getBrainModule();
    BOOST_REQUIRE(lb);
    BOOST_CHECK_EQUAL(lb->getStats()->getInstanceId(), brainId);
    // the loaded controller is the same kind of controller, not a plain puppet master
    BOOST_CHECK(std::dynamic_pointer_cast<puppetMasterCollector>(lb));
    BOOST_CHECK(lb->getStats()->getCollector().lock() == ld);

    // and it keeps driving the drone after the load
    auto start = ld->getStats()->getMyPosition();
    bool moved = false;
    for (int c = 0; c < 2000 && !moved; c++) {
        bElem::runLiveElements();
        moved = !(ld->getStats()->getMyPosition() == start);
    }
    BOOST_CHECK(moved);
}

BOOST_AUTO_TEST_CASE(CameraAndGuardiansRoundTrip)
{
    inputManager::getInstance(true);
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    auto mc = chamber::makeNewChamber(coords(30, 30));
    auto plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(10, 15));
    auto cam = elementFactory::generateAnElement<securityCamera>(mc, 0);
    cam->stepOnElement(mc->getElement(16, 15));
    for (int c = 0; c < 40; c++)
        bElem::runLiveElements();
    BOOST_REQUIRE_GT(cam->getAlertNumber(), 0u);
    auto camId = cam->getStats()->getInstanceId();
    auto alerts = cam->getAlertNumber();

    const std::string f1 = tmpFile("goe-cam-1.goe"), f2 = tmpFile("goe-cam-2.goe");
    BOOST_REQUIRE(gameSerializer::saveGame(f1));
    plr.reset();
    cam.reset();
    mc.reset();
    BOOST_REQUIRE(gameSerializer::loadGame(f1));
    BOOST_REQUIRE(gameSerializer::saveGame(f2));
    BOOST_CHECK(withoutCounter(readFile(f1)) == withoutCounter(readFile(f2)));
    std::remove(f1.c_str());
    std::remove(f2.c_str());

    std::shared_ptr<securityCamera> lcam;
    int guardians = 0;
    for (const auto &c : chamber::allChambers)
        for (int x = 0; x < c->getSize().x; x++)
            for (int y = 0; y < c->getSize().y; y++) {
                auto e = c->getElement(x, y);
                if (auto sc = std::dynamic_pointer_cast<securityCamera>(e); sc && sc->getStats()->getInstanceId() == camId)
                    lcam = sc;
            }
    BOOST_REQUIRE(lcam);
    BOOST_CHECK_EQUAL(lcam->getAlertNumber(), alerts);
    for (int x = 0; x < lcam->getBoard()->getSize().x; x++)
        for (int y = 0; y < lcam->getBoard()->getSize().y; y++)
            if (auto d = std::dynamic_pointer_cast<patrollingDrone>(lcam->getBoard()->getElement(x, y)))
                if (auto g = std::dynamic_pointer_cast<puppetMasterGuardian>(d->getBrainModule())) {
                    BOOST_CHECK(g->getCamera() == lcam);
                    guardians++;
                }
    BOOST_CHECK_EQUAL(guardians, securityCamera::guardianCount);
    for (int c = 0; c < 300; c++)
        bElem::runLiveElements();
}

BOOST_AUTO_TEST_CASE(SavingTwiceReplacesTheOldSave)
{
    // std::rename cannot overwrite a file on Windows, so the second quick save used to fail there
    inputManager::getInstance(true);
    chamber::makeNewChamber(coords(8, 8));
    const std::string f = tmpFile("goe-twice.goe");
    BOOST_REQUIRE(gameSerializer::saveGame(f));
    BOOST_REQUIRE(gameSerializer::saveGame(f));
    BOOST_CHECK(std::filesystem::exists(f));
    BOOST_CHECK(!std::filesystem::exists(f + ".tmp"));
    BOOST_CHECK(gameSerializer::loadGame(f));
    std::remove(f.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
