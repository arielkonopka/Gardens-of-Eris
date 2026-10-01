#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "gameSerializer.h"
#include "randomLevelGenerator.h"
#include "worldBuilder.h"
#include "randomStreams.h"
#include <gtest/gtest.h>
#include "testSupport.h"
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
        for (const coords chunk : c->chunkKeys()) {
            const coords first = chamber::chunkOrigin(chunk);
            for (int x = 0; x < chamber::chunkSize; x++)
                for (int y = 0; y < chamber::chunkSize; y++)
                    for (auto e = c->getElement(first + coords(x, y)); e; e = e->getStats()->getSteppingOn())
                        res[e->getType()]++;
        }
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


TEST(SaveGameTests, SmallWorldRoundTrip)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(12, 12));
    auto plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(3, 3));
    plr->getStats()->setActive(true);
    auto gun = elementFactory::generateAnElement<plainGun>(mc, 0);
    gun->stepOnElement(mc->getElement(4, 3));
    ASSERT_TRUE(plr->collect(gun));
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
    ASSERT_TRUE(gameSerializer::saveGame(f1));
    plr.reset();
    gun.reset();
    k.reset();
    w.reset();
    mc.reset();
    ASSERT_TRUE(gameSerializer::loadGame(f1));

    EXPECT_EQ(chamber::allChambers.size(), chambersBefore);
    EXPECT_TRUE(census() == censusBefore);
    auto lc = findChamber(chamberId);
    ASSERT_TRUE(lc);
    auto lp = lc->getElement(3, 3);
    ASSERT_TRUE(lp);
    EXPECT_EQ(lp->getType(), bElemTypes::_player);
    EXPECT_EQ(lp->getStats()->getInstanceId(), plrId);
    EXPECT_TRUE(lp->getBoard() == lc);
    EXPECT_EQ(lp->getStats()->getPoints(COLLECTS), 7);
    ASSERT_TRUE(player::getActivePlayer());
    EXPECT_EQ(player::getActivePlayer()->getStats()->getInstanceId(), activeId);
    auto lgun = lp->getAttrs()->getInventory()->getActiveWeapon();
    ASSERT_TRUE(lgun);
    EXPECT_EQ(lgun->getStats()->getInstanceId(), gunId);
    EXPECT_TRUE(lgun->getStats()->isCollected());
    EXPECT_TRUE(lgun->getStats()->getCollector().lock() == lp);
    // the key stands on a floor, and the floor knows it
    auto lk = lc->getElement(5, 5);
    ASSERT_TRUE(lk);
    EXPECT_EQ(lk->getStats()->getInstanceId(), keyId);
    EXPECT_EQ(lk->getAttrs()->getSubtype(), 2);
    auto under = lk->getStats()->getSteppingOn();
    ASSERT_TRUE(under);
    EXPECT_EQ(under->getType(), bElemTypes::_floorType);
    EXPECT_TRUE(under->getStats()->getStandingOn().lock() == lk);
    EXPECT_TRUE(under->getStats()->getMyPosition() == coords(5, 5));
    EXPECT_EQ(lc->getElement(0, 0)->getType(), bElemTypes::_wallType);

    // saving the loaded world again gives the same file
    ASSERT_TRUE(gameSerializer::saveGame(f2));
    EXPECT_TRUE(withoutCounter(readFile(f1)) == withoutCounter(readFile(f2)));

    // the loaded world is playable
    for (int c = 0; c < 200; c++)
        bElem::runLiveElements();
    std::remove(f1.c_str());
    std::remove(f2.c_str());
}

TEST(SaveGameTests, GeneratedLevelRoundTrip)
{
    inputManager::getInstance(true);
    randomLevelGenerator(120, 120).generateLevel(5);
    // let the level live a little, so timers, missiles and dying elements exist
    for (int c = 0; c < 300; c++)
        bElem::runLiveElements();

    auto censusBefore = census();
    auto apples = goldenApple::getAppleNumber();
    const std::string f1 = tmpFile("goe-level-1.goe"), f2 = tmpFile("goe-level-2.goe");
    ASSERT_TRUE(gameSerializer::saveGame(f1));
    ASSERT_TRUE(gameSerializer::loadGame(f1));
    EXPECT_TRUE(census() == censusBefore);
    EXPECT_EQ(goldenApple::getAppleNumber(), apples);
    ASSERT_TRUE(gameSerializer::saveGame(f2));
    EXPECT_TRUE(withoutCounter(readFile(f1)) == withoutCounter(readFile(f2)));

    // mechanics keep running on the restored world
    for (int c = 0; c < 300; c++)
        bElem::runLiveElements();
    std::remove(f1.c_str());
    std::remove(f2.c_str());
}

TEST(SaveGameTests, HardLevelKeepsItsDifficultyAndLandmines)
{
    inputManager::getInstance(true);
    randomLevelGenerator gen(120, 120);
    ASSERT_TRUE(gen.generateLevel(1));
    const int id = gen.mychamber->getInstanceId();
    const coords origin = gen.mychamber->origin;
    for (int c = 0; c < 100; c++)
        bElem::runLiveElements();

    auto censusBefore = census();
    EXPECT_GT(censusBefore[bElemTypes::_landmineType], 0);
    const std::string f1 = tmpFile("goe-hard-1.goe"), f2 = tmpFile("goe-hard-2.goe");
    ASSERT_TRUE(gameSerializer::saveGame(f1));
    ASSERT_TRUE(gameSerializer::loadGame(f1));
    EXPECT_TRUE(census() == censusBefore);
    auto loaded = findChamber(id);
    ASSERT_TRUE(loaded);
    EXPECT_TRUE(loaded->origin == origin);
    ASSERT_TRUE(gameSerializer::saveGame(f2));
    EXPECT_TRUE(withoutCounter(readFile(f1)) == withoutCounter(readFile(f2)));
    std::remove(f1.c_str());
    std::remove(f2.c_str());
}

TEST(SaveGameTests, BadFilesLeaveTheWorldAlone)
{
    inputManager::getInstance(true);
    auto mc = chamber::makeNewChamber(coords(8, 8));
    auto plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(2, 2));
    const std::string good = tmpFile("goe-good.goe"), bad = tmpFile("goe-bad.goe"),
                      cut = tmpFile("goe-cut.goe");
    ASSERT_TRUE(gameSerializer::saveGame(good));
    auto chambersBefore = chamber::allChambers.size();
    auto censusBefore = census();
    auto active = player::getActivePlayer();

    EXPECT_TRUE(!gameSerializer::loadGame(tmpFile("goe-does-not-exist.goe")));
    {
        std::ofstream out(bad, std::ios::binary);
        out << "this is not a save file at all";
    }
    EXPECT_TRUE(!gameSerializer::loadGame(bad));
    {
        auto data = readFile(good);
        std::ofstream out(cut, std::ios::binary);
        out.write(data.data(), (std::streamsize) data.size() / 2);
    }
    EXPECT_TRUE(!gameSerializer::loadGame(cut));
    // Continue is offered for a real save only; a cut one passes this quick check and fails on load
    EXPECT_TRUE(gameSerializer::canLoad(good));
    EXPECT_FALSE(gameSerializer::canLoad(tmpFile("goe-does-not-exist.goe")));
    EXPECT_FALSE(gameSerializer::canLoad(bad));

    EXPECT_EQ(chamber::allChambers.size(), chambersBefore);
    EXPECT_TRUE(census() == censusBefore);
    EXPECT_TRUE(player::getActivePlayer() == active);
    std::remove(good.c_str());
    std::remove(bad.c_str());
    std::remove(cut.c_str());
}

TEST(SaveGameTests, ControlledDroneKeepsItsController)
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
    ASSERT_TRUE(plr->collect(brain));
    auto drone = elementFactory::generateAnElement<patrollingDrone>(mc, 0);
    drone->stepOnElement(mc->getElement(10, 10));
    ASSERT_TRUE(drone->interact(plr));
    auto droneId = drone->getStats()->getInstanceId();
    auto brainId = brain->getStats()->getInstanceId();

    const std::string f = tmpFile("goe-drone.goe");
    ASSERT_TRUE(gameSerializer::saveGame(f));
    plr.reset();
    brain.reset();
    drone.reset();
    mc.reset();
    ASSERT_TRUE(gameSerializer::loadGame(f));
    std::remove(f.c_str());

    std::shared_ptr<bElem> ld;
    for (const auto &c : chamber::allChambers)
        for (int x = 0; x < c->getSize().x && !ld; x++)
            for (int y = 0; y < c->getSize().y && !ld; y++)
                for (auto e = c->getElement(x, y); e; e = e->getStats()->getSteppingOn())
                    if (e->getStats()->getInstanceId() == droneId)
                        ld = e;
    ASSERT_TRUE(ld);
    auto d = std::dynamic_pointer_cast<patrollingDrone>(ld);
    ASSERT_TRUE(d);
    auto lb = d->getBrainModule();
    ASSERT_TRUE(lb);
    EXPECT_EQ(lb->getStats()->getInstanceId(), brainId);
    // the loaded controller is the same kind of controller, not a plain puppet master
    EXPECT_TRUE(std::dynamic_pointer_cast<puppetMasterCollector>(lb));
    EXPECT_TRUE(lb->getStats()->getCollector().lock() == ld);

    // and it keeps driving the drone after the load
    auto start = ld->getStats()->getMyPosition();
    bool moved = false;
    for (int c = 0; c < 2000 && !moved; c++) {
        bElem::runLiveElements();
        moved = !(ld->getStats()->getMyPosition() == start);
    }
    EXPECT_TRUE(moved);
}

TEST(SaveGameTests, CameraAndGuardiansRoundTrip)
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
    ASSERT_GT(cam->getAlertNumber(), 0u);
    auto camId = cam->getStats()->getInstanceId();
    auto alerts = cam->getAlertNumber();

    const std::string f1 = tmpFile("goe-cam-1.goe"), f2 = tmpFile("goe-cam-2.goe");
    ASSERT_TRUE(gameSerializer::saveGame(f1));
    plr.reset();
    cam.reset();
    mc.reset();
    ASSERT_TRUE(gameSerializer::loadGame(f1));
    ASSERT_TRUE(gameSerializer::saveGame(f2));
    EXPECT_TRUE(withoutCounter(readFile(f1)) == withoutCounter(readFile(f2)));
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
    ASSERT_TRUE(lcam);
    EXPECT_EQ(lcam->getAlertNumber(), alerts);
    for (int x = 0; x < lcam->getBoard()->getSize().x; x++)
        for (int y = 0; y < lcam->getBoard()->getSize().y; y++)
            if (auto d = std::dynamic_pointer_cast<patrollingDrone>(lcam->getBoard()->getElement(x, y)))
                if (auto g = std::dynamic_pointer_cast<puppetMasterGuardian>(d->getBrainModule())) {
                    EXPECT_TRUE(g->getCamera() == lcam);
                    guardians++;
                }
    EXPECT_EQ(guardians, difficulty::guardianCount(0));
    for (int c = 0; c < 300; c++)
        bElem::runLiveElements();
}

TEST(SaveGameTests, SavingTwiceReplacesTheOldSave)
{
    // std::rename cannot overwrite a file on Windows, so the second quick save used to fail there
    inputManager::getInstance(true);
    chamber::makeNewChamber(coords(8, 8));
    const std::string f = tmpFile("goe-twice.goe");
    ASSERT_TRUE(gameSerializer::saveGame(f));
    ASSERT_TRUE(gameSerializer::saveGame(f));
    EXPECT_TRUE(std::filesystem::exists(f));
    EXPECT_TRUE(!std::filesystem::exists(f + ".tmp"));
    EXPECT_TRUE(gameSerializer::loadGame(f));
    std::remove(f.c_str());
}


TEST(SaveGameTests, SaveKeepsTheWorldSeed)
{
    // chunks built after a load must fit the ones built before it, so they need the same seed
    inputManager::getInstance(true);
    chamber::makeNewChamber(coords(8, 8));
    const std::string f = tmpFile("goe-seed.goe");
    goe::rng::setWorldSeed(2323);
    ASSERT_TRUE(gameSerializer::saveGame(f));
    goe::rng::setWorldSeed(55);
    ASSERT_TRUE(gameSerializer::loadGame(f));
    EXPECT_EQ(goe::rng::worldSeed(), (goe::rng::seed) 2323);
    std::remove(f.c_str());
}

TEST(SaveGameTests, EndlessWorldRoundTrip)
{
    inputManager::getInstance(true);
    gameSerializer::clearWorld();
    goe::rng::setWorldSeed(2323);
    auto world = worldBuilder::startNew();
    // grow it to the west too, where cells are negative
    while (worldBuilder::growAround(world, world->origin - coords(3 * chamber::chunkSize, 0)))
        ;
    for (int c = 0; c < 100; c++)
        bElem::runLiveElements();
    const auto keys = world->chunkKeys();
    const auto censusBefore = census();
    const int id = world->getInstanceId();
    const coords origin = world->origin;

    const std::string f1 = tmpFile("goe-world-1.goe"), f2 = tmpFile("goe-world-2.goe");
    ASSERT_TRUE(gameSerializer::saveGame(f1));
    ASSERT_TRUE(gameSerializer::loadGame(f1));
    ASSERT_EQ(chamber::allChambers.size(), 1u);
    auto loaded = findChamber(id);
    ASSERT_TRUE(loaded);
    EXPECT_FALSE(loaded->isBounded());
    EXPECT_TRUE(loaded->chunkKeys() == keys);
    EXPECT_TRUE(loaded->origin == origin);
    EXPECT_TRUE(census() == censusBefore);
    ASSERT_TRUE(player::getActivePlayer());
    EXPECT_TRUE(player::getActivePlayer()->getBoard() == loaded);
    // the loaded world still grows
    const auto before = loaded->chunkKeys().size();
    EXPECT_TRUE(worldBuilder::growAround(loaded, loaded->origin + coords(5 * chamber::chunkSize, 0)));
    EXPECT_EQ(loaded->chunkKeys().size(), before + 1);

    ASSERT_TRUE(gameSerializer::loadGame(f1));
    ASSERT_TRUE(gameSerializer::saveGame(f2));
    EXPECT_TRUE(withoutCounter(readFile(f1)) == withoutCounter(readFile(f2)));
    std::remove(f1.c_str());
    std::remove(f2.c_str());
}

TEST(SaveGameTests, NewGameReplacesTheOldSave)
{
    inputManager::getInstance(true);
    const std::string f = tmpFile("goe-replace.goe");
    gameSerializer::clearWorld();
    goe::rng::setWorldSeed(111);
    worldBuilder::startNew();
    ASSERT_TRUE(gameSerializer::saveGame(f));

    // a new game over the old one: Continue now brings back the new world
    gameSerializer::clearWorld();
    goe::rng::setWorldSeed(222);
    worldBuilder::startNew();
    ASSERT_TRUE(gameSerializer::replaceSave(f));
    goe::rng::setWorldSeed(333);
    ASSERT_TRUE(gameSerializer::loadGame(f));
    EXPECT_EQ(goe::rng::worldSeed(), 222u);

    // when the old save can be neither written over nor removed, it says so
    const std::string blocked = tmpFile("goe-replace-blocked");
    std::filesystem::create_directories(blocked + "/inside");
    EXPECT_FALSE(gameSerializer::replaceSave(blocked));
    std::filesystem::remove_all(blocked);
    std::remove(f.c_str());
}
