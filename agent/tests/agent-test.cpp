/*
 * The agent interface: the game played without a window, as exRelaxer's agents play Doom.
 */
#include "agentGame.h"
#include "elementFactory.h"
#include "elements.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <optional>

using namespace goe::agent;

namespace {
/// the game's data folder; ctest passes it, run by hand the test also works from GoEoOL/
config withData(config c = {})
{
    if (const char *d = std::getenv("GOE_DATA_DIR"))
        c.dataDir = d;
    return c;
}

/// the value of one channel at a cell of the vision grid
float at(const game &g, const state &s, const std::string &name, int row, int col)
{
    const auto &names = g.cellFeatureNames();
    const auto ch = (std::size_t) (std::find(names.begin(), names.end(), name) - names.begin());
    EXPECT_LT(ch, names.size()) << name;
    const auto side = (std::size_t) s.visionSide;
    return s.vision[ch * side * side + (std::size_t) row * side + (std::size_t) col];
}

float playerValue(const game &g, const state &s, const std::string &name)
{
    const auto &names = g.playerFeatureNames();
    const auto i = (std::size_t) (std::find(names.begin(), names.end(), name) - names.begin());
    EXPECT_LT(i, names.size()) << name;
    return s.player[i];
}

/// an action that walks the player onto a free neighbouring cell, if there is one
std::optional<action> freeStep()
{
    const auto plr = player::getActivePlayer();
    const std::pair<dir::direction, action> ways[] = {{dir::direction::UP, action::moveUp},
                                                      {dir::direction::DOWN, action::moveDown},
                                                      {dir::direction::LEFT, action::moveLeft},
                                                      {dir::direction::RIGHT, action::moveRight}};
    for (auto [d, a] : ways) {
        const auto e = plr->getElementInDirection(d);
        if (e && e->getAttrs()->isSteppable() && !e->getAttrs()->isCollectible())
            return a;
    }
    return std::nullopt;
}
} // namespace

TEST(AgentTests, ActionsAreTheGameControls)
{
    EXPECT_EQ(actionName(action::noop), "NOOP");
    EXPECT_EQ(actionName(action::shootLeft), "SHOOT_LEFT");
    EXPECT_EQ(actionName(action::giveUp), "GIVE_UP");
    EXPECT_EQ(controlOf(action::moveUp).type, 0);
    EXPECT_EQ(controlOf(action::moveUp).dir, dir::direction::UP);
    EXPECT_EQ(controlOf(action::shootRight).type, 1);
    EXPECT_EQ(controlOf(action::interactDown).type, 2);
    EXPECT_EQ(controlOf(action::nextItem).type, 3);
    EXPECT_EQ(controlOf(action::dragLeft).type, 4);
    EXPECT_EQ(controlOf(action::nextGun).type, 5);
    EXPECT_EQ(controlOf(action::giveUp).type, 6);
    EXPECT_EQ(controlOf(action::use).type, 8);
    EXPECT_EQ(controlOf(action::drop).type, 9);
    EXPECT_EQ(controlOf(action::noop).type, -1);
    EXPECT_EQ(sectionName(section::usables), "usables");
}

TEST(AgentTests, UnknownFeaturesAreRefused)
{
    config c = withData();
    c.cellFeatures = {"type", "colour"};
    EXPECT_THROW(game{c}, std::invalid_argument);
    c = withData();
    c.itemFeatures = {"in_sight"}; // a cell name, not an item one
    EXPECT_THROW(game{c}, std::invalid_argument);
    c = withData();
    c.visionRadius = -1;
    EXPECT_THROW(game{c}, std::invalid_argument);
}

TEST(AgentTests, OneGamePerProcess)
{
    game first(withData());
    EXPECT_THROW(game{withData()}, std::runtime_error);
}

TEST(AgentTests, TheDefaultObservationHasEverything)
{
    game g(withData());
    g.newEpisode(555);
    const state s = g.getState();
    const auto names = g.cellFeatureNames();
    EXPECT_EQ(names.size(), elementFeatures().size() + 2);
    EXPECT_EQ(names.front(), "type");
    EXPECT_EQ(g.playerFeatureNames().size(), elementFeatures().size() + playerFeatures().size());
    EXPECT_EQ(s.visionSide, 17);
    EXPECT_EQ(s.vision.size(), names.size() * 17 * 17);
    EXPECT_EQ(s.player.size(), g.playerFeatureNames().size());
    EXPECT_EQ(g.sections().size(), (std::size_t) sectionCount);
    EXPECT_EQ(s.inventory.size(), (std::size_t) sectionCount * 5 * g.itemFeatureNames().size());
    EXPECT_EQ(s.inventoryCounts.size(), (std::size_t) sectionCount);
    EXPECT_EQ(g.actions(), actionCount - 1);
    EXPECT_FALSE(g.isEpisodeFinished());
}

TEST(AgentTests, TheVisionIsACircleAroundThePlayer)
{
    game g(withData());
    g.newEpisode(555);
    const state s = g.getState();
    // the centre is the player, at the start of the maze
    EXPECT_EQ(at(g, s, "type", 8, 8), (float) bElemTypes::_player);
    EXPECT_EQ(at(g, s, "active", 8, 8), 1.0f);
    EXPECT_EQ(at(g, s, "in_sight", 8, 8), 1.0f);
    const auto plr = player::getActivePlayer();
    const coords centre = plr->getStats()->getMyPosition();
    EXPECT_TRUE(s.centre == centre - plr->getBoard()->origin);
    // the corners are outside the circle: empty
    EXPECT_EQ(at(g, s, "type", 0, 0), -1.0f);
    EXPECT_EQ(at(g, s, "exists", 0, 0), 0.0f);
    EXPECT_EQ(at(g, s, "facing", 0, 0), 4.0f);
    // the edge of the circle is in it, and the start is built all around
    EXPECT_EQ(at(g, s, "exists", 0, 8), 1.0f);
    EXPECT_EQ(at(g, s, "exists", 8, 16), 1.0f);
    // far away cells are out of the player's sight at the start (view radius 2)
    EXPECT_EQ(at(g, s, "in_sight", 0, 8), 0.0f);
    // every cell matches the board, row by row and column by column
    for (int row = 0; row < 17; row++)
        for (int col = 0; col < 17; col++) {
            if ((row - 8) * (row - 8) + (col - 8) * (col - 8) > 64)
                continue;
            const auto e = plr->getBoard()->getElement(centre + coords(col - 8, row - 8));
            ASSERT_TRUE(e);
            EXPECT_EQ(at(g, s, "type", row, col), (float) e->getType()) << row << "," << col;
            EXPECT_EQ(at(g, s, "subtype", row, col), (float) e->getAttrs()->getSubtype());
        }
}

TEST(AgentTests, TheVisionCanStayOnAFixedCell)
{
    config c = withData();
    c.followPlayer = false;
    c.fixedCentre = coords(3, -2);
    c.circle = false;
    c.visionRadius = 2;
    c.cellFeatures = {"type", "in_sight"};
    game g(c);
    g.newEpisode(555);
    const state s = g.getState();
    ASSERT_EQ(s.visionSide, 5);
    ASSERT_EQ(s.vision.size(), 2u * 25u);
    EXPECT_TRUE(s.centre == coords(3, -2));
    const auto plr = player::getActivePlayer();
    const coords origin = plr->getBoard()->origin;
    // not a circle: the corners are filled too
    for (int row = 0; row < 5; row++)
        for (int col = 0; col < 5; col++)
            EXPECT_EQ(at(g, s, "type", row, col),
                      (float) plr->getBoard()->getElement(origin + coords(3 + col - 2, -2 + row - 2))->getType());
}

TEST(AgentTests, ThePlayerAndItsInventoryCanBeChosen)
{
    config c = withData();
    c.playerFeatures = {"x", "y", "score", "energy", "view_radius"};
    c.inventorySections = {section::weapons, section::keys};
    c.inventorySlots = 3;
    c.itemFeatures = {"type", "ammo", "selected"};
    game g(c);
    g.newEpisode(555);
    state s = g.getState();
    ASSERT_EQ(s.player.size(), 5u);
    const auto plr = player::getActivePlayer();
    // x and y count from the middle of the start area
    const coords from = plr->getStats()->getMyPosition() - plr->getBoard()->origin;
    EXPECT_EQ(s.player[0], (float) from.x);
    EXPECT_EQ(s.player[1], (float) from.y);
    EXPECT_GT(s.player[3], 0.0f);
    EXPECT_EQ(s.player[4], plr->getViewRadius());
    ASSERT_EQ(s.inventory.size(), 2u * 3u * 3u);
    ASSERT_EQ(s.inventoryCounts.size(), 2u);

    // give the player a gun: it shows in a weapon slot, selected
    const auto gun = elementFactory::generateAnElement<plainGun>(plr->getBoard(), 0);
    ASSERT_TRUE(plr->getAttrs()->getInventory()->addToInventory(gun));
    s = g.getState();
    const auto weapons = (std::size_t) s.inventoryCounts[0];
    ASSERT_GE(weapons, 1u);
    bool found = false;
    for (std::size_t k = 0; k < std::min<std::size_t>(weapons, 3); k++)
        if (s.inventory[k * 3] == (float) bElemTypes::_plainGun) {
            found = true;
            EXPECT_EQ(s.inventory[k * 3 + 1], (float) gun->getAttrs()->getAmmo());
        }
    EXPECT_TRUE(found);
    float selected = 0;
    for (std::size_t k = 0; k < 3; k++)
        selected += s.inventory[k * 3 + 2];
    EXPECT_EQ(selected, 1.0f); // one active weapon
    // empty slots: type -1
    for (std::size_t k = std::min<std::size_t>(weapons, 3); k < 3; k++)
        EXPECT_EQ(s.inventory[k * 3], -1.0f);
}

TEST(AgentTests, AStepMovesThePlayerOneCell)
{
    game g(withData());
    g.newEpisode(555);
    const auto move = freeStep();
    ASSERT_TRUE(move) << "the start has a free cell next to the player";
    const state before = g.getState();
    g.makeAction(*move);
    EXPECT_TRUE(g.actionTaken());
    EXPECT_EQ(g.episodeTick(), 8u);
    // let the move finish
    g.makeAction(action::noop);
    const state after = g.getState();
    const float dx = playerValue(g, after, "x") - playerValue(g, before, "x");
    const float dy = playerValue(g, after, "y") - playerValue(g, before, "y");
    EXPECT_EQ(std::abs(dx) + std::abs(dy), 1.0f);
    // the vision moved with the player
    EXPECT_FALSE(after.centre == before.centre);
    EXPECT_EQ(at(g, after, "type", 8, 8), (float) bElemTypes::_player);
}

TEST(AgentTests, TheSameSeedPlaysTheSameGame)
{
    game g(withData());
    const auto play = [&g](std::uint32_t seed) {
        g.newEpisode(seed);
        std::vector<float> trace;
        const action plan[] = {action::moveRight, action::moveDown, action::shootUp, action::moveLeft,
                               action::noop, action::moveUp, action::interactRight, action::moveDown};
        for (int round = 0; round < 3; round++)
            for (auto a : plan) {
                trace.push_back(g.makeAction(a));
                const state s = g.getState();
                trace.insert(trace.end(), s.vision.begin(), s.vision.end());
                trace.insert(trace.end(), s.player.begin(), s.player.end());
            }
        return trace;
    };
    const auto a = play(4242);
    EXPECT_EQ(g.seed(), 4242u);
    const auto b = play(4242);
    const auto c = play(4243);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(AgentTests, EpisodesCanBeCutShort)
{
    config c = withData();
    c.episodeTicks = 20;
    game g(c);
    g.newEpisode(555);
    g.makeAction(action::noop);
    g.makeAction(action::noop);
    EXPECT_FALSE(g.isEpisodeFinished());
    g.makeAction(action::noop);
    EXPECT_EQ(g.episodeTick(), 20u); // the last step stops at the limit
    EXPECT_TRUE(g.isTruncated());
    EXPECT_TRUE(g.isEpisodeFinished());
    EXPECT_FALSE(g.isPlayerDead());
    // a new episode starts over
    g.newEpisode(555);
    EXPECT_FALSE(g.isEpisodeFinished());
}

TEST(AgentTests, GivingUpIsOnlyThereWhenAsked)
{
    {
        game g(withData());
        g.newEpisode(555);
        EXPECT_THROW(g.makeAction(action::giveUp), std::out_of_range);
    }
    config c = withData();
    c.allowGiveUp = true;
    game g(c);
    g.newEpisode(555);
    EXPECT_EQ(g.actions(), actionCount);
    g.makeAction(action::giveUp);
    // the avatar dies; the game is over unless a spare avatar takes over
    for (int s = 0; s < 100 && !g.isEpisodeFinished() && g.avatarsLost() == 0; s++)
        g.makeAction(action::noop);
    EXPECT_TRUE(g.isPlayerDead() || g.avatarsLost() > 0);
    if (g.isPlayerDead()) {
        const state s = g.getState();
        EXPECT_EQ(s.player.size(), g.playerFeatureNames().size());
        EXPECT_FLOAT_EQ(s.player[0], -1.0f); // no player: type -1
        // a step after the end does nothing
        EXPECT_EQ(g.makeAction(action::noop), 0.0f);
    }
}
