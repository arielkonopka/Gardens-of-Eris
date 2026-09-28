#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "gameSerializer.h"
#include "randomLevelGenerator.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>

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

    const std::string f1 = "/tmp/goe-roundtrip-1.goe", f2 = "/tmp/goe-roundtrip-2.goe";
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
    const std::string f1 = "/tmp/goe-level-1.goe", f2 = "/tmp/goe-level-2.goe";
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
    const std::string good = "/tmp/goe-good.goe", bad = "/tmp/goe-bad.goe",
                      cut = "/tmp/goe-cut.goe";
    BOOST_REQUIRE(gameSerializer::saveGame(good));
    auto chambersBefore = chamber::allChambers.size();
    auto censusBefore = census();
    auto active = player::getActivePlayer();

    BOOST_CHECK(!gameSerializer::loadGame("/tmp/goe-does-not-exist.goe"));
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

BOOST_AUTO_TEST_SUITE_END()
