/*
 * The agent interface: the game played without a window, as exRelaxer's agents play Doom.
 */
#include "agentGame.h"
#include "chamber.h"
#include "elementFactory.h"
#include "elements.h"
#include "worldBuilder.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
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

namespace {
/// a free cell next to the player, its direction, and the actions that walk and interact there
struct side
{
    dir::direction d;
    action move, interact, shoot;
};
std::optional<side> freeSide(int skip = 0)
{
    const auto plr = player::getActivePlayer();
    const side ways[] = {{dir::direction::UP, action::moveUp, action::interactUp, action::shootUp},
                         {dir::direction::DOWN, action::moveDown, action::interactDown, action::shootDown},
                         {dir::direction::LEFT, action::moveLeft, action::interactLeft, action::shootLeft},
                         {dir::direction::RIGHT, action::moveRight, action::interactRight, action::shootRight}};
    for (const auto &w : ways) {
        const auto e = plr->getElementInDirection(w.d);
        if (e && e->getType() == bElemTypes::_floorType && skip-- == 0)
            return w;
    }
    return std::nullopt;
}

float events(const game &g, event e)
{
    return g.episodeEvents()[(std::size_t) e];
}

/// waits until the player can act again
void settle(game &g)
{
    for (int s = 0; s < 10 && !g.isEpisodeFinished(); s++)
        g.makeAction(action::noop);
}
} // namespace

TEST(AgentTests, TheDefaultRewardIsTheScore)
{
    game g(withData());
    g.newEpisode(555);
    float reward = 0;
    for (int s = 0; s < 40 && !g.isEpisodeFinished(); s++) {
        const auto move = freeStep();
        reward += g.makeAction(move ? *move : action::noop);
    }
    EXPECT_GT(reward, 0.0f); // new cells walked
    EXPECT_EQ(reward, events(g, event::score));
}

TEST(AgentTests, UnknownEventsAreRefused)
{
    config c = withData();
    c.rewardWeights = {{"nonsense", 1.0f}};
    EXPECT_THROW(game g(c), std::invalid_argument);
}

TEST(AgentTests, CollectingCountsOnceAnItem)
{
    config c = withData();
    c.rewardWeights = {{"collect", 5.0f}, {"apple", 20.0f}};
    game g(c);
    g.newEpisode(555);
    const auto w = freeSide();
    ASSERT_TRUE(w);
    const auto plr = player::getActivePlayer();
    const auto gun = elementFactory::generateAnElement<plainGun>(plr->getBoard(), 0);
    gun->stepOnElement(plr->getElementInDirection(w->d));
    EXPECT_EQ(g.makeAction(w->interact), 5.0f);
    EXPECT_EQ(g.stepEvents()[(std::size_t) event::collect], 1.0f);
    settle(g);
    // dropped and picked up again: the same gun counts once
    plr->getAttrs()->getInventory()->retrieveCollectibleFromInventory(gun->getStats()->getInstanceId(), true);
    gun->stepOnElement(plr->getElementInDirection(w->d));
    g.makeAction(w->interact);
    settle(g);
    EXPECT_EQ(events(g, event::collect), 1.0f);
    // a golden apple is an apple
    const auto apple = elementFactory::generateAnElement<goldenApple>(plr->getBoard(), 0);
    apple->stepOnElement(plr->getElementInDirection(w->d));
    EXPECT_EQ(g.makeAction(w->interact), 20.0f);
    EXPECT_EQ(events(g, event::apple), 1.0f);
    EXPECT_EQ(events(g, event::collect), 1.0f);
}

TEST(AgentTests, OpeningCountsOnceADoor)
{
    config c = withData();
    c.rewardWeights = {{"open", 10.0f}};
    game g(c);
    g.newEpisode(555);
    const auto w = freeSide();
    ASSERT_TRUE(w);
    const auto plr = player::getActivePlayer();
    const auto d = elementFactory::generateAnElement<door>(plr->getBoard(), 1);
    d->stepOnElement(plr->getElementInDirection(w->d));
    d->getAttrs()->setLocked(false);
    d->getAttrs()->setOpen(false);
    float reward = 0;
    for (int k = 0; k < 3; k++) { // open, close, open
        reward += g.makeAction(w->interact);
        settle(g);
    }
    EXPECT_TRUE(d->getAttrs()->isOpen());
    EXPECT_EQ(events(g, event::open), 1.0f);
    EXPECT_EQ(reward, 10.0f);
}

TEST(AgentTests, ShootingCountsKillsAndMines)
{
    config c = withData();
    c.rewardWeights = {{"kill", 10.0f}, {"mine", 3.0f}};
    game g(c);
    g.newEpisode(555);
    const auto plr = player::getActivePlayer();
    const auto gun = elementFactory::generateAnElement<plainGun>(plr->getBoard(), 0);
    gun->getAttrs()->setSubtype(1); // endless ammo
    ASSERT_TRUE(plr->getAttrs()->getInventory()->addToInventory(gun));
    const auto w = freeSide();
    ASSERT_TRUE(w);
    const auto target = elementFactory::generateAnElement<monster>(plr->getBoard(), 0);
    target->stepOnElement(plr->getElementInDirection(w->d));
    target->getStats()->setWaiting(100000); // it stays to be shot
    float reward = 0;
    for (int s = 0; s < 100 && events(g, event::kill) == 0 && !g.isEpisodeFinished(); s++)
        reward += g.makeAction(w->shoot);
    EXPECT_EQ(events(g, event::kill), 1.0f);
    EXPECT_EQ(reward, 10.0f);
    settle(g);
    // a mine next to the player: its blast may take the player too, so it comes last
    const auto w2 = freeSide();
    ASSERT_TRUE(w2);
    const auto mine = elementFactory::generateAnElement<landmine>(plr->getBoard(), 0);
    mine->stepOnElement(plr->getElementInDirection(w2->d));
    for (int s = 0; s < 20 && events(g, event::mine) == 0 && !g.isEpisodeFinished(); s++)
        g.makeAction(w2->shoot);
    EXPECT_EQ(events(g, event::mine), 1.0f);
}

TEST(AgentTests, TeleportingAndUsingAreCounted)
{
    config c = withData();
    c.rewardWeights = {{"teleport", 2.0f}, {"use", 1.0f}};
    game g(c);
    g.newEpisode(555);
    const auto plr = player::getActivePlayer();
    // a broken apple in hand: eating it gives energy
    const auto apple = elementFactory::generateAnElement<goldenApple>(plr->getBoard(), 0);
    apple->hurt(1);
    ASSERT_TRUE(plr->collect(apple));
    plr->getAttrs()->setEnergy(50);
    g.makeAction(action::use);
    EXPECT_EQ(g.stepEvents()[(std::size_t) event::use], 1.0f);
    settle(g);
    const auto w = freeSide();
    ASSERT_TRUE(w);
    const auto there = elementFactory::generateAnElement<teleport>(plr->getBoard(), 7);
    there->stepOnElement(plr->getElementInDirection(w->d));
    const auto w2 = freeSide();
    ASSERT_TRUE(w2);
    const auto back = elementFactory::generateAnElement<teleport>(plr->getBoard(), 7);
    back->stepOnElement(plr->getElementInDirection(w2->d));
    g.makeAction(w->interact);
    settle(g);
    EXPECT_EQ(events(g, event::teleport), 1.0f);
}

TEST(AgentTests, HurtAndDeathArePenalised)
{
    config c = withData();
    c.allowGiveUp = true;
    c.rewardWeights = {{"hurt", -1.0f}, {"death", -50.0f}};
    game g(c);
    g.newEpisode(555);
    const auto plr = player::getActivePlayer();
    const auto w = freeSide();
    ASSERT_TRUE(w);
    // a missile coming at the player
    const auto missile = elementFactory::generateAnElement<plainMissile>(plr->getBoard(), 0);
    missile->stepOnElement(plr->getElementInDirection(w->d));
    missile->getStats()->setMyDirection((dir::direction) (((int) w->d + 2) % 4));
    const int energy = plr->getAttrs()->getEnergy();
    float reward = 0;
    for (int s = 0; s < 5; s++)
        reward += g.makeAction(action::noop);
    EXPECT_LT(plr->getAttrs()->getEnergy(), energy);
    EXPECT_EQ(events(g, event::hurt), (float) (energy - plr->getAttrs()->getEnergy()));
    EXPECT_EQ(reward, -events(g, event::hurt));
    g.makeAction(action::giveUp);
    for (int s = 0; s < 100 && !g.isEpisodeFinished() && g.avatarsLost() == 0; s++)
        g.makeAction(action::noop);
    EXPECT_EQ(events(g, event::death), 1.0f);
}

namespace {
/// a room of 8 x 8 with the player, a key and a golden apple; repeated, it fills a chunk with rooms
const std::vector<std::string> roomRows = {
    "########",
    "#......#",
    "#.k..A.#",
    "#......#",
    "#..@...#",
    "#......#",
    "#......#",
    "########",
};
const std::map<char, goe::patternCell> roomLegend = {
    {'#', {bElemTypes::_wallType, 0}},
    {'.', {}},
    {'k', {bElemTypes::_key, 1}},
    {'A', {bElemTypes::_goldenAppleType, 0}},
    {'@', {bElemTypes::_player, 0}},
};

/// the same room without the player
std::shared_ptr<const goe::chunkPattern> emptyRoom()
{
    auto rows = roomRows;
    rows[4][3] = '.';
    return std::make_shared<const goe::chunkPattern>(goe::chunkPattern::fromRows(rows, roomLegend));
}

/// whether the chunk's cells hold what the pattern places (the floor where it places nothing)
bool chunkFollows(const std::shared_ptr<chamber> &board, coords chunk, const goe::chunkPattern &p)
{
    const coords first = chamber::chunkOrigin(chunk);
    for (int y = 0; y < chamber::chunkSize; y++)
        for (int x = 0; x < chamber::chunkSize; x++) {
            const auto e = board->getElement(first + coords(x, y));
            const int want = p.at(x, y).type == bElemTypes::_belemType ? bElemTypes::_floorType : p.at(x, y).type;
            if (!e || e->getType() != want)
                return false;
        }
    return true;
}
} // namespace

TEST(PatternTests, PatternsAreReadFromJson)
{
    const auto p = goe::chunkPattern::fromJson(R"({"legend": {"#": "wall", "k": ["key", 3], "D": [52, 1],
        "~": [0, 2], ".": null}, "rows": ["#k", "D~", ".#"]})");
    EXPECT_EQ(p.width(), 2);
    EXPECT_EQ(p.height(), 3);
    EXPECT_EQ(p.at(0, 0), (goe::patternCell{bElemTypes::_wallType, 0}));
    EXPECT_EQ(p.at(1, 0), (goe::patternCell{bElemTypes::_key, 3}));
    EXPECT_EQ(p.at(0, 1), (goe::patternCell{bElemTypes::_door, 1}));
    EXPECT_EQ(p.at(1, 1), (goe::patternCell{bElemTypes::_floorType, 2}));
    EXPECT_EQ(p.at(0, 2), goe::patternCell{});
    // repeated to fill a chunk
    EXPECT_EQ(p.at(2, 3), p.at(0, 0));
    EXPECT_EQ(p.at(63, 63), p.at(1, 0));
    EXPECT_TRUE(p.places(bElemTypes::_key));
    EXPECT_FALSE(p.places(bElemTypes::_player));
    EXPECT_EQ(goe::chunkPattern::typeByName("golden_apple"), bElemTypes::_goldenAppleType);
}

TEST(PatternTests, BadPatternsAreRefused)
{
    using goe::chunkPattern;
    EXPECT_THROW(chunkPattern(0, 1, {}), std::invalid_argument);
    EXPECT_THROW(chunkPattern(65, 1, std::vector<goe::patternCell>(65)), std::invalid_argument);
    EXPECT_THROW(chunkPattern(2, 2, std::vector<goe::patternCell>(3)), std::invalid_argument);
    // missiles are not placed, and types the game does not know neither
    EXPECT_THROW(chunkPattern(1, 1, {{bElemTypes::_plainMissile, 0}}), std::invalid_argument);
    EXPECT_THROW(chunkPattern(1, 1, {{12345, 0}}), std::invalid_argument);
    EXPECT_THROW(chunkPattern::fromJson("not json"), std::invalid_argument);
    EXPECT_THROW(chunkPattern::fromJson(R"({"legend": {"#": "wall"}, "rows": ["#?"]})"), std::invalid_argument);
    EXPECT_THROW(chunkPattern::fromJson(R"({"legend": {"#": "dragon"}, "rows": ["#"]})"), std::invalid_argument);
    EXPECT_THROW(chunkPattern::fromJson(R"({"legend": {"##": "wall"}, "rows": ["#"]})"), std::invalid_argument);
    EXPECT_THROW(chunkPattern::fromJson(R"({"legend": {"#": "wall"}, "rows": ["##", "#"]})"), std::invalid_argument);
    EXPECT_THROW(chunkPattern::load("no/such/pattern.json"), std::invalid_argument);
}

TEST(PatternTests, TheStartChunkCanBeBuiltFromAPattern)
{
    config c = withData();
    const auto room = std::make_shared<const goe::chunkPattern>(goe::chunkPattern::fromRows(roomRows, roomLegend));
    c.chunkPatterns[{0, 0}] = room;
    game g(c);
    g.newEpisode(555);
    const auto plr = player::getActivePlayer();
    ASSERT_TRUE(plr);
    const auto board = plr->getBoard();
    // the first player of the pattern is the one the game starts with, and the origin is there
    EXPECT_TRUE(plr->getStats()->getMyPosition() == coords(3, 4));
    EXPECT_TRUE(board->origin == coords(3, 4));
    EXPECT_TRUE(chunkFollows(board, coords(0, 0), *room));
    // the chunks around are random mazes
    EXPECT_FALSE(chunkFollows(board, coords(1, 0), *room));
    // the agent sees the pattern: a wall one cell left of the room's first column, three cells left
    const state s = g.getState();
    EXPECT_EQ(at(g, s, "type", 8, 8), (float) bElemTypes::_player);
    EXPECT_EQ(at(g, s, "type", 8, 5), (float) bElemTypes::_wallType);
    EXPECT_EQ(at(g, s, "type", 6, 7), (float) bElemTypes::_key);
    EXPECT_EQ(at(g, s, "type", 6, 10), (float) bElemTypes::_goldenAppleType);
    // chunks as the agent counts cells, from the origin
    EXPECT_EQ(g.chunkAt(coords(0, 0)), std::make_pair(0, 0));
    EXPECT_EQ(g.chunkAt(coords(60, 0)), std::make_pair(0, 0));
    EXPECT_EQ(g.chunkAt(coords(61, 0)), std::make_pair(1, 0));
    EXPECT_EQ(g.chunkAt(coords(-4, -5)), std::make_pair(-1, -1));
}

TEST(PatternTests, EveryChunkCanTakeTheSamePattern)
{
    config c = withData();
    const auto room = emptyRoom();
    c.defaultPattern = room;
    game g(c);
    std::vector<int> first;
    for (std::uint32_t seed : {1u, 2u}) {
        g.newEpisode(seed);
        const auto plr = player::getActivePlayer();
        ASSERT_TRUE(plr);
        const auto board = plr->getBoard();
        // no player in the pattern: the player goes on the free floor nearest the chunk's middle
        const coords at = plr->getStats()->getMyPosition();
        EXPECT_TRUE(board->origin == at);
        EXPECT_LE(at.distance(coords(32, 32)), 1.5f);
        for (int x = -1; x <= 1; x++)
            for (int y = -1; y <= 1; y++) {
                // the start chunk holds the player on one of its cells
                if (x == 0 && y == 0)
                    continue;
                EXPECT_TRUE(chunkFollows(board, coords(x, y), *room)) << x << "," << y;
            }
        // any seed builds the same world
        std::vector<int> types;
        for (int y = -64; y < 128; y++)
            for (int x = -64; x < 128; x++)
                types.push_back(board->getElement(coords(x, y))->getType());
        if (first.empty())
            first = types;
        else
            EXPECT_EQ(types, first);
    }
}

TEST(PatternTests, PatternsCanChangeBetweenChunks)
{
    config c = withData();
    game g(c);
    g.newEpisode(555);
    const auto board = player::getActivePlayer()->getBoard();
    // a chunk not built yet takes the pattern; random chunks stay random
    const auto room = emptyRoom();
    g.setChunkPattern({5, 0}, room);
    EXPECT_FALSE(board->hasChunk(coords(5, 0)));
    worldBuilder::growAround(board, chamber::chunkOrigin(coords(5, 0)));
    EXPECT_TRUE(chunkFollows(board, coords(5, 0), *room));
    g.setChunkPattern({5, 0}, nullptr);
    g.setDefaultPattern(room);
    worldBuilder::growAround(board, chamber::chunkOrigin(coords(-5, 0)));
    EXPECT_TRUE(chunkFollows(board, coords(-5, 0), *room));
    // the next episode is random again once the patterns are cleared
    g.clearChunkPatterns();
    g.newEpisode(555);
    EXPECT_FALSE(chunkFollows(player::getActivePlayer()->getBoard(), coords(1, 0), *room));
    EXPECT_TRUE(g.getConfig().chunkPatterns.empty());
}
