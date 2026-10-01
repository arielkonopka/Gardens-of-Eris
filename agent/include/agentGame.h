#ifndef GOE_AGENT_GAME_H
#define GOE_AGENT_GAME_H

#include "agentFeatures.h"
#include "chunkPattern.h"
#include "commons.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <map>
#include <set>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class bElem;
class chamber;

/**
 * Gardens of Eris without a window, a sound or a keyboard, for a program (an agent) to play.
 *
 * The shape follows ViZDoom's DoomGame, which exRelaxer already drives: newEpisode, getState,
 * makeAction, isEpisodeFinished. The Python package (agent/python) wraps it, and adds a
 * Gymnasium-style reset/step on top.
 *
 * The game keeps its world in static state, so there is one game per process; a second one
 * throws. Run games in parallel in separate processes.
 */
namespace goe::agent {

/// what the agent can do in one step; the game's own controls, one at a time
enum class action : int {
    noop,
    moveUp, moveDown, moveLeft, moveRight,
    shootUp, shootDown, shootLeft, shootRight,
    interactUp, interactDown, interactLeft, interactRight,
    dragUp, dragDown, dragLeft, dragRight,
    nextItem, nextGun, use, drop,
    giveUp, ///< only when config::allowGiveUp: the avatar dies
    count
};
constexpr int actionCount = (int) action::count;
/// "MOVE_UP", ...
std::string actionName(action a);
/// the game's control command for the action
controlItem controlOf(action a);

/// the inventory parts the agent can see
enum class section : int { weapons, usables, keys, mods, tokens, count };
constexpr int sectionCount = (int) section::count;
std::string sectionName(section s);

/// what the agent's own avatar did or suffered in a step; the reward weighs these
enum class event : int {
    score,    ///< points gained (the game's score: new cells visited, items collected, damage dealt)
    collect,  ///< items collected, each item once an episode (golden apples count as apple)
    apple,    ///< golden apples collected, each once an episode
    use,      ///< the usable in hand used (a broken golden apple eaten for energy)
    open,     ///< doors opened, each door once an episode
    teleport, ///< trips through a teleporter
    kill,     ///< monsters, drones and puppet masters killed by the player's missiles and blasts
    mine,     ///< mines and bombs set off by the player's missiles and blasts
    hurt,     ///< energy lost
    death,    ///< avatars lost (the last one included)
    count
};
constexpr int eventCount = (int) event::count;
/// "score", "collect", ...
std::string eventName(event e);
std::optional<event> eventByName(std::string_view name);
using eventCounts = std::array<float, eventCount>;

struct config
{
    /// the folder with data/skins.json (the game's GoEoOL folder)
    std::filesystem::path dataDir;

    /// the vision grid is (2 * visionRadius + 1) cells on each side
    int visionRadius = 8;
    /// cells further than visionRadius from the centre are left empty, so the grid is a circle
    bool circle = true;
    /// centre the vision on the active player, or on a fixed cell
    bool followPlayer = true;
    /// the fixed centre, counted from the middle of the start area (chamber::origin); used when followPlayer is false
    coords fixedCentre = coords(0, 0);
    /// what each vision cell holds: element feature names, plus the cell names "exists" (the
    /// cell is built) and "in_sight" (within the player's view radius). Empty: all of them.
    std::vector<std::string> cellFeatures;
    /// the player's numbers: element and player feature names. Empty: all of them.
    std::vector<std::string> playerFeatures;
    /// the inventory sections shown, in order. Empty: all of them.
    std::vector<section> inventorySections;
    /// items shown per section; more are left out, fewer leave empty slots (type -1)
    int inventorySlots = 5;
    /// what each item slot holds: element feature names plus "selected" (the active weapon, the
    /// usable in hand). Empty: type, subtype, energy, ammo, max_ammo, selected.
    std::vector<std::string> itemFeatures;

    /// game ticks per action (the game runs 50 ticks a second; a step takes 8)
    int ticksPerStep = GoEConstants::_mov_delay;
    /// the episode is truncated after this many ticks; 0 for no limit (the maze never ends)
    std::uint64_t episodeTicks = 0;
    /// adds action::giveUp to the actions
    bool allowGiveUp = false;
    /// the reward of a step: the sum of each event's count (eventName) times its weight; events
    /// left out weigh 0. The default is the score gained. Penalties take negative weights.
    std::map<std::string, float> rewardWeights = {{"score", 1.0f}};

    /// chunks built from a fixed pattern instead of a random maze, by chunk: (0, 0) is the start
    /// chunk, (1, 0) the one east of it, (0, -1) the one north of it (see game::chunkAt). A
    /// chunk is chamber::chunkSize (64) cells on each side.
    std::map<std::pair<int, int>, std::shared_ptr<const chunkPattern>> chunkPatterns;
    /// the pattern of every chunk not in chunkPatterns; none: a random maze
    std::shared_ptr<const chunkPattern> defaultPattern;
};

/// one observation
struct state
{
    int visionSide = 0;
    /// channels x rows x columns: cell (row r, column c) is centre + (c - radius, r - radius)
    std::vector<float> vision;
    std::vector<float> player;
    /// sections x slots x item features
    std::vector<float> inventory;
    /// how many items each section holds, all of them (not only those in slots)
    std::vector<float> inventoryCounts;
    /// the centre of the vision, counted from the middle of the start area
    coords centre = coords(0, 0);
    std::uint64_t tick = 0;
    int score = 0;
};

class game
{
public:
    explicit game(config cfg);
    ~game();
    game(const game &) = delete;
    game &operator=(const game &) = delete;

    /// a new world; the same seed builds the same world. No seed: a fresh one.
    void newEpisode(std::optional<std::uint32_t> seed = std::nullopt);
    /// plays the action for config::ticksPerStep ticks and returns the reward: the step's events
    /// weighed by config::rewardWeights (by default the score gained)
    float makeAction(action a);
    /// runs one game tick with the control given to the player; false when the episode is over
    bool advance(controlItem control);

    state getState() const;
    /// the last avatar is gone, or episodeTicks passed
    bool isEpisodeFinished() const;
    /// the last avatar is gone (game over)
    bool isPlayerDead() const;
    /// episodeTicks passed
    bool isTruncated() const;
    /// whether the last makeAction reached the player; false when it was busy the whole step
    bool actionTaken() const { return this->taken; }
    std::uint64_t episodeTick() const { return this->ticks; }
    /// how many times this episode a spare avatar took over from a lost one
    int avatarsLost() const { return this->lostAvatars; }
    int score() const;
    std::uint32_t seed() const { return this->worldSeed; }
    /// what happened in the last makeAction, and in the episode so far
    const eventCounts &stepEvents() const { return this->stepCounts; }
    const eventCounts &episodeEvents() const { return this->episodeCounts; }

    /// a chunk's own pattern (nullptr removes it), for chunks made from now on: those not built
    /// yet in this episode, and every chunk of the next episodes
    void setChunkPattern(std::pair<int, int> chunk, std::shared_ptr<const chunkPattern> pattern);
    /// the pattern of every chunk without its own (nullptr: a random maze), as setChunkPattern
    void setDefaultPattern(std::shared_ptr<const chunkPattern> pattern);
    /// every chunk a random maze again, as setChunkPattern
    void clearChunkPatterns();
    /// the chunk holding a cell given as the agent sees it, counted from the middle of the start area
    std::pair<int, int> chunkAt(coords cell) const;
    /// generates the chunk at (chunk_x, chunk_y) with the given pattern (or the default/random);
    /// the chunk must not exist yet; true if successful (the chunk was built), false if no world
    bool generateChunk(std::pair<int, int> chunk, std::shared_ptr<const chunkPattern> pattern = nullptr);

    const config &getConfig() const { return this->cfg; }
    int actions() const { return this->cfg.allowGiveUp ? actionCount : actionCount - 1; }
    const std::vector<std::string> &cellFeatureNames() const { return this->cellNames; }
    const std::vector<std::string> &playerFeatureNames() const { return this->playerNames; }
    const std::vector<std::string> &itemFeatureNames() const { return this->itemNames; }
    const std::vector<section> &sections() const { return this->shownSections; }

private:
    void fillVision(state &s, const std::shared_ptr<bElem> &plr, const std::shared_ptr<chamber> &board) const;
    void fillInventory(state &s, const std::shared_ptr<bElem> &plr) const;
    /// counts a game event the player's avatar took part in (goe::events)
    void noteEvent(int k, const bElem &subject, const bElem *actor);
    /// hands config::chunkPatterns and defaultPattern to the world builder
    void installPatterns() const;

    config cfg;
    std::vector<std::string> cellNames, playerNames, itemNames;
    std::vector<section> shownSections;
    std::vector<feature> cellReads, playerReads, itemReads;
    /// where the names game fills itself sit among the names; -1 when not asked for
    int inSightAt = -1;
    int selectedAt = -1;
    /// the avatar the score is read from
    unsigned long playerId = 0;
    int lostAvatars = 0;
    std::uint64_t ticks = 0;
    std::uint32_t worldSeed = 0;
    int lastScore = 0;
    bool taken = false;
    std::array<float, eventCount> weights{};
    eventCounts stepCounts{}, episodeCounts{};
    /// what counts once an episode: items collected and doors opened, by instance id
    std::set<unsigned long> collected, opened;
};

} // namespace goe::agent

#endif // GOE_AGENT_GAME_H
