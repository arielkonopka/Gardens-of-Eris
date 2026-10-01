/*
 * The autopilot that plays the title screen's demo.
 */
#include "autopilot.h"
#include "chamber.h"
#include "elements.h"
#include "gameSerializer.h"
#include "inputManager.h"
#include "randomStreams.h"
#include "worldBuilder.h"
#include <gtest/gtest.h>
#include <set>

namespace {
std::shared_ptr<bElem> startWorld(goe::rng::seed seed)
{
    inputManager::getInstance(true);
    gameSerializer::clearWorld();
    goe::rng::setWorldSeed(seed);
    worldBuilder::startNew();
    return player::getActivePlayer();
}

/// a free floor cell next to the player, and its direction
std::optional<dir::direction> freeSide(const std::shared_ptr<bElem> &plr)
{
    for (auto d : {dir::direction::UP, dir::direction::LEFT, dir::direction::DOWN, dir::direction::RIGHT}) {
        const auto e = plr->getElementInDirection(d);
        if (e && e->getType() == bElemTypes::_floorType && e->getAttrs()->isSteppable())
            return d;
    }
    return std::nullopt;
}
} // namespace

TEST(AutopilotTests, ItCollectsWhatLiesNextToIt)
{
    const auto plr = startWorld(555);
    ASSERT_TRUE(plr);
    const auto d = freeSide(plr);
    ASSERT_TRUE(d);
    const auto apple = elementFactory::generateAnElement<goldenApple>(plr->getBoard(), 0);
    apple->stepOnElement(plr->getElementInDirection(*d));
    goe::autopilot pilot;
    const controlItem c = pilot.decide(plr);
    EXPECT_EQ(c.type, 0);
    EXPECT_EQ(c.dir, *d);
}

TEST(AutopilotTests, ItShootsAMonsterInLine)
{
    const auto plr = startWorld(555);
    ASSERT_TRUE(plr);
    const auto d = freeSide(plr);
    ASSERT_TRUE(d);
    goe::autopilot pilot;
    // unarmed, it does not shoot
    const auto monsterThere = elementFactory::generateAnElement<monster>(plr->getBoard(), 0);
    monsterThere->stepOnElement(plr->getElementInDirection(*d));
    EXPECT_NE(pilot.decide(plr).type, 1);
    const auto gun = elementFactory::generateAnElement<plainGun>(plr->getBoard(), 0);
    plr->getAttrs()->getInventory()->addToInventory(gun);
    ASSERT_TRUE(plr->getAttrs()->getInventory()->getActiveWeapon());
    const controlItem c = pilot.decide(plr);
    EXPECT_EQ(c.type, 1);
    EXPECT_EQ(c.dir, *d);
}

TEST(AutopilotTests, ItWandersAboutTheMaze)
{
    const auto plr = startWorld(4242);
    ASSERT_TRUE(plr);
    goe::autopilot pilot;
    std::set<std::pair<int, int>> seen;
    for (int t = 0; t < 50 * 60 && player::getActivePlayer(); t++) {
        const auto p = player::getActivePlayer();
        inputManager::getInstance(true).setControlItem(pilot.decide(p));
        const coords at = p->getStats()->getMyPosition();
        if (!worldBuilder::growAround(p->getBoard(), at))
            worldBuilder::shrinkAround(p->getBoard(), at);
        bElem::runLiveElements();
        if (const auto now = player::getActivePlayer())
            seen.insert({now->getStats()->getMyPosition().x, now->getStats()->getMyPosition().y});
    }
    inputManager::getInstance(true).setControlItem(controlItem(-1, dir::direction::NODIRECTION));
    // a minute of play: it walks rather than standing about
    EXPECT_GT(seen.size(), 100u);
    gameSerializer::clearWorld();
}
