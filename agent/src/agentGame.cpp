#include "agentGame.h"
#include "chamber.h"
#include "configManager.h"
#include "elements.h"
#include "gameClock.h"
#include "gameEvents.h"
#include "gameSerializer.h"
#include "inputManager.h"
#include "randomStreams.h"
#include "worldBuilder.h"
#include <array>
#include <atomic>
#include <cstdlib>
#include <stdexcept>

namespace goe::agent {
namespace {
/// one game per process: the world lives in static state
std::atomic<bool> gameAlive = false;

constexpr controlItem nothing = controlItem(-1, dir::direction::NODIRECTION);

struct actionInfo
{
    std::string_view name;
    controlItem control;
};
// the command numbers are the ones player::mechanics reads (see controlBindings::translate)
constexpr std::array<actionInfo, actionCount> actionTable = {{
    {"NOOP", nothing},
    {"MOVE_UP", {0, dir::direction::UP}},
    {"MOVE_DOWN", {0, dir::direction::DOWN}},
    {"MOVE_LEFT", {0, dir::direction::LEFT}},
    {"MOVE_RIGHT", {0, dir::direction::RIGHT}},
    {"SHOOT_UP", {1, dir::direction::UP}},
    {"SHOOT_DOWN", {1, dir::direction::DOWN}},
    {"SHOOT_LEFT", {1, dir::direction::LEFT}},
    {"SHOOT_RIGHT", {1, dir::direction::RIGHT}},
    {"INTERACT_UP", {2, dir::direction::UP}},
    {"INTERACT_DOWN", {2, dir::direction::DOWN}},
    {"INTERACT_LEFT", {2, dir::direction::LEFT}},
    {"INTERACT_RIGHT", {2, dir::direction::RIGHT}},
    {"DRAG_UP", {4, dir::direction::UP}},
    {"DRAG_DOWN", {4, dir::direction::DOWN}},
    {"DRAG_LEFT", {4, dir::direction::LEFT}},
    {"DRAG_RIGHT", {4, dir::direction::RIGHT}},
    {"NEXT_ITEM", {3, dir::direction::NODIRECTION}},
    {"NEXT_GUN", {5, dir::direction::NODIRECTION}},
    {"USE", {8, dir::direction::NODIRECTION}},
    {"DROP", {9, dir::direction::NODIRECTION}},
    {"GIVE_UP", {6, dir::direction::NODIRECTION}},
}};

constexpr std::array<std::string_view, eventCount> eventNames = {
    "score", "collect", "apple", "use", "open", "teleport", "kill", "mine", "hurt", "death"};

constexpr std::array<std::string_view, sectionCount> sectionNames = {"weapons", "usables", "keys", "mods", "tokens"};

// Names that only make sense in one place. Their read is a stand-in: game fills them itself.
// clang-format off
const std::array cellOnly = {
    feature{"exists", "the cell is built (1), or not built yet or outside the circle (0)", [](bElem &) { return 1.0f; }},
    feature{"in_sight", "the cell is within the player's view radius", [](bElem &) { return 0.0f; }},
};
const std::array itemOnly = {
    feature{"selected", "the active weapon, or the usable in hand", [](bElem &) { return 0.0f; }},
};
// clang-format on

const std::vector<std::string> defaultItemFeatures = {"type", "subtype", "energy", "ammo", "max_ammo", "selected"};

int indexOf(const std::vector<std::string> &names, std::string_view name)
{
    for (std::size_t i = 0; i < names.size(); i++)
        if (names[i] == name)
            return (int) i;
    return -1;
}

std::vector<std::string> joined(std::initializer_list<std::span<const feature>> tables)
{
    std::vector<std::string> res;
    for (const auto &t : tables)
        for (const auto &n : namesOf(t))
            res.push_back(n);
    return res;
}

/// the player acts in the next tick: every timer it waits on has run out by then
bool readyNextTick(const bElem &e)
{
    const bElemStats &st = *e.getStats();
    // a getter gives the ticks left; one left now is none left in the next tick
    const auto soon = [](int left) { return left <= 1; };
    return soon(st.getWaiting()) && soon(st.getTelInProgress()) && soon(st.getKilled())
           && soon(st.getDestroyed()) && soon(st.getMoved()) && soon(st.getFadingIn())
           && soon(st.getFadingOut()) && (!e.getAttrs()->isInteractive() || soon(st.getInteracted()));
}

const std::vector<std::shared_ptr<bElem>> &itemsOf(inventory &inv, section s)
{
    switch (s) {
    case section::weapons:
        return inv.weapons;
    case section::usables:
        return inv.usables;
    case section::keys:
        return inv.keys;
    case section::mods:
        return inv.mods;
    default:
        return inv.tokens;
    }
}

/// the board the game is on, also once the last avatar is gone
std::shared_ptr<chamber> worldBoard(const std::shared_ptr<bElem> &plr)
{
    if (plr && plr->getBoard())
        return plr->getBoard();
    return chamber::allChambers.empty() ? nullptr : chamber::allChambers.front();
}
} // namespace

std::string actionName(action a)
{
    return std::string(actionTable.at((std::size_t) a).name);
}

controlItem controlOf(action a)
{
    return actionTable.at((std::size_t) a).control;
}

std::string sectionName(section s)
{
    return std::string(sectionNames.at((std::size_t) s));
}

std::string eventName(event e)
{
    return std::string(eventNames.at((std::size_t) e));
}

std::optional<event> eventByName(std::string_view name)
{
    for (int e = 0; e < eventCount; e++)
        if (eventNames[(std::size_t) e] == name)
            return (event) e;
    return std::nullopt;
}

game::game(config c) : cfg(std::move(c))
{
    if (this->cfg.visionRadius < 0)
        throw std::invalid_argument("visionRadius must be 0 or more");
    if (this->cfg.ticksPerStep < 1)
        throw std::invalid_argument("ticksPerStep must be 1 or more");
    if (this->cfg.inventorySlots < 0)
        throw std::invalid_argument("inventorySlots must be 0 or more");
    for (const auto &[name, w] : this->cfg.rewardWeights) {
        const auto e = eventByName(name);
        if (!e)
            throw std::invalid_argument("no such event in rewardWeights: " + name);
        this->weights[(std::size_t) *e] = w;
    }

    this->cellNames = this->cfg.cellFeatures.empty() ? joined({elementFeatures(), cellOnly}) : this->cfg.cellFeatures;
    this->playerNames = this->cfg.playerFeatures.empty() ? joined({elementFeatures(), playerFeatures()})
                                                         : this->cfg.playerFeatures;
    this->itemNames = this->cfg.itemFeatures.empty() ? defaultItemFeatures : this->cfg.itemFeatures;
    this->cellReads = select(this->cellNames, {elementFeatures(), cellOnly});
    this->playerReads = select(this->playerNames, {elementFeatures(), playerFeatures()});
    this->itemReads = select(this->itemNames, {elementFeatures(), itemOnly});
    this->inSightAt = indexOf(this->cellNames, "in_sight");
    this->selectedAt = indexOf(this->itemNames, "selected");
    if (this->cfg.inventorySections.empty())
        for (int s = 0; s < sectionCount; s++)
            this->shownSections.push_back((section) s);
    else
        this->shownSections = this->cfg.inventorySections;

    if (gameAlive.exchange(true))
        throw std::runtime_error("one Gardens of Eris game per process: close the other one first");
    try {
#ifndef _WIN32
        // no sound is ever played; OpenAL's silent output keeps it from looking for a sound card
        setenv("ALSOFT_DRIVERS", "null", 0);
#endif
        // skins.json is read from the working directory, once
        const auto here = std::filesystem::current_path();
        if (!this->cfg.dataDir.empty())
            std::filesystem::current_path(this->cfg.dataDir);
        struct back
        {
            std::filesystem::path to;
            ~back() { std::filesystem::current_path(this->to); }
        } restore{here};
        configManager::getInstance();
        inputManager::getInstance(true); // no keyboard and no input thread
        goe::events::observe([this](goe::events::kind k, const bElem &subject, const bElem *actor) {
            this->noteEvent((int) k, subject, actor);
        });
    } catch (...) {
        gameAlive = false;
        throw;
    }
}

game::~game()
{
    goe::events::observe({});
    gameSerializer::clearWorld();
    worldBuilder::clearPatterns();
    inputManager::getInstance(true).setControlItem(nothing);
    gameAlive = false;
}

void game::newEpisode(std::optional<std::uint32_t> seed)
{
    gameSerializer::clearWorld();
    this->worldSeed = seed.value_or(goe::rng::freshSeed());
    goe::rng::setWorldSeed(this->worldSeed);
    // the running game's engine too, so one seed replays the same game for the same actions
    goe::rng::saved().seed(this->worldSeed);
    gameClock::ticks = 5;
    inputManager::getInstance(true).setControlItem(nothing);
    this->installPatterns();
    worldBuilder::startNew();
    this->ticks = 0;
    this->taken = false;
    const auto plr = player::getActivePlayer();
    this->playerId = plr ? plr->getStats()->getInstanceId() : 0;
    this->lastScore = this->score();
    this->lostAvatars = 0;
    this->stepCounts.fill(0.0f);
    this->episodeCounts.fill(0.0f);
    this->collected.clear();
    this->opened.clear();
}

void game::installPatterns() const
{
    worldBuilder::clearPatterns();
    for (const auto &[chunk, pattern] : this->cfg.chunkPatterns)
        worldBuilder::setPattern(coords(chunk.first, chunk.second), pattern);
    worldBuilder::setDefaultPattern(this->cfg.defaultPattern);
}

void game::setChunkPattern(std::pair<int, int> chunk, std::shared_ptr<const chunkPattern> pattern)
{
    if (pattern)
        this->cfg.chunkPatterns[chunk] = std::move(pattern);
    else
        this->cfg.chunkPatterns.erase(chunk);
    this->installPatterns();
}

void game::setDefaultPattern(std::shared_ptr<const chunkPattern> pattern)
{
    this->cfg.defaultPattern = std::move(pattern);
    this->installPatterns();
}

void game::clearChunkPatterns()
{
    this->cfg.chunkPatterns.clear();
    this->cfg.defaultPattern = nullptr;
    this->installPatterns();
}

std::pair<int, int> game::chunkAt(coords cell) const
{
    const auto board = worldBoard(player::getActivePlayer());
    const coords origin = board && board->origin != NOCOORDS ? board->origin : coords(0, 0);
    const coords chunk = chamber::chunkOf(origin + cell);
    return {chunk.x, chunk.y};
}

void game::noteEvent(int k, const bElem &subject, const bElem *actor)
{
    // only what the agent's avatars do; the world's other doings (a monster opening a door) are not its
    if (!actor || actor->getType() != bElemTypes::_player)
        return;
    const unsigned long id = subject.getStats()->getInstanceId();
    const int type = subject.getType();
    auto add = [this](event e) { this->stepCounts[(std::size_t) e] += 1.0f; };
    switch ((goe::events::kind) k) {
    case goe::events::kind::collect:
        if (this->collected.insert(id).second)
            add(type == bElemTypes::_goldenAppleType ? event::apple : event::collect);
        break;
    case goe::events::kind::use:
        add(event::use);
        break;
    case goe::events::kind::open:
        if (this->opened.insert(id).second)
            add(event::open);
        break;
    case goe::events::kind::teleport:
        add(event::teleport);
        break;
    case goe::events::kind::kill:
        if (type == bElemTypes::_simpleBombType || type == bElemTypes::_landmineType)
            add(event::mine);
        else if (type == bElemTypes::_monster || type == bElemTypes::_patrollingDrone
                 || type == bElemTypes::_puppetMasterType)
            add(event::kill);
        break;
    }
}

bool game::advance(controlItem control)
{
    const auto plr = player::getActivePlayer();
    if (!plr || this->isTruncated())
        return false;
    inputManager::getInstance(true).setControlItem(control);
    // as the presenter does: the maze grows ahead of the player, far chunks go to disk
    const coords at = plr->getStats()->getMyPosition();
    if (!worldBuilder::growAround(plr->getBoard(), at))
        worldBuilder::shrinkAround(plr->getBoard(), at);
    bElem::runLiveElements();
    inputManager::getInstance(true).setControlItem(nothing);
    this->ticks++;
    return !this->isEpisodeFinished();
}

float game::makeAction(action a)
{
    if ((int) a < 0 || (int) a >= this->actions())
        throw std::out_of_range("no such action: " + std::to_string((int) a));
    // The control reaches the player once, in the first tick it can act, like a key pressed
    // once. A step is a decision, whatever the step's length and however long a move takes.
    this->taken = false;
    this->stepCounts.fill(0.0f);
    const bool aliveBefore = !this->isPlayerDead();
    for (int t = 0; t < this->cfg.ticksPerStep && !this->isEpisodeFinished(); t++) {
        const auto plr = player::getActivePlayer();
        const bool deliver = !this->taken && plr && readyNextTick(*plr);
        const int energy = plr ? plr->getAttrs()->getEnergy() : 0;
        this->advance(deliver ? controlOf(a) : nothing);
        this->taken = this->taken || deliver;
        // energy lost by the same avatar; a golden apple's energy gained is not counted
        const auto after = player::getActivePlayer();
        if (plr && after == plr && after->getAttrs()->getEnergy() < energy)
            this->stepCounts[(std::size_t) event::hurt] += (float) (energy - after->getAttrs()->getEnergy());
    }
    const auto plr = player::getActivePlayer();
    if (plr && plr->getStats()->getInstanceId() != this->playerId) {
        // a spare avatar taking over brings its own score: no score for the switch
        this->playerId = plr->getStats()->getInstanceId();
        this->lastScore = this->score();
        this->lostAvatars++;
        this->stepCounts[(std::size_t) event::death] += 1.0f;
    } else {
        const int now = this->score();
        this->stepCounts[(std::size_t) event::score] = (float) (now - this->lastScore);
        this->lastScore = now;
        if (aliveBefore && this->isPlayerDead())
            this->stepCounts[(std::size_t) event::death] += 1.0f;
    }
    float reward = 0.0f;
    for (std::size_t e = 0; e < (std::size_t) eventCount; e++) {
        this->episodeCounts[e] += this->stepCounts[e];
        reward += this->weights[e] * this->stepCounts[e];
    }
    return reward;
}

int game::score() const
{
    const auto plr = player::getActivePlayer();
    return plr ? plr->getStats()->getPoints(TOTAL) : this->lastScore;
}

bool game::isPlayerDead() const
{
    return player::getActivePlayer() == nullptr;
}

bool game::isTruncated() const
{
    return this->cfg.episodeTicks > 0 && this->ticks >= this->cfg.episodeTicks;
}

bool game::isEpisodeFinished() const
{
    return this->isPlayerDead() || this->isTruncated();
}

state game::getState() const
{
    state s;
    s.tick = this->ticks;
    s.score = this->score();
    const auto plr = player::getActivePlayer();
    this->fillVision(s, plr, worldBoard(plr));
    s.player.resize(this->playerReads.size());
    for (std::size_t f = 0; f < this->playerReads.size(); f++)
        s.player[f] = plr ? this->playerReads[f].read(*plr) : this->playerReads[f].absent;
    this->fillInventory(s, plr);
    return s;
}

void game::fillVision(state &s, const std::shared_ptr<bElem> &plr, const std::shared_ptr<chamber> &board) const
{
    const int r = this->cfg.visionRadius;
    const int side = 2 * r + 1;
    const std::size_t cells = (std::size_t) side * side;
    const std::size_t channels = this->cellReads.size();
    s.visionSide = side;
    s.vision.assign(channels * cells, 0.0f);
    for (std::size_t f = 0; f < channels; f++)
        std::fill_n(s.vision.begin() + (long) (f * cells), cells, this->cellReads[f].absent);
    if (!board)
        return;
    const coords origin = board->origin == NOCOORDS ? coords(0, 0) : board->origin;
    const coords centre = this->cfg.followPlayer && plr ? plr->getStats()->getMyPosition()
                                                        : origin + this->cfg.fixedCentre;
    s.centre = centre - origin;
    const coords seer = plr ? plr->getStats()->getMyPosition() : NOCOORDS;
    const float sight = plr ? plr->getViewRadius() : -1.0f;
    for (int row = 0; row < side; row++)
        for (int col = 0; col < side; col++) {
            const int dx = col - r, dy = row - r;
            if (this->cfg.circle && dx * dx + dy * dy > r * r)
                continue;
            const coords cell = centre + coords(dx, dy);
            const std::size_t at = (std::size_t) row * side + col;
            if (const auto e = board->getElement(cell))
                for (std::size_t f = 0; f < channels; f++)
                    s.vision[f * cells + at] = this->cellReads[f].read(*e);
            if (this->inSightAt >= 0 && plr)
                s.vision[(std::size_t) this->inSightAt * cells + at] = cell.distance(seer) <= sight ? 1.0f : 0.0f;
        }
}

void game::fillInventory(state &s, const std::shared_ptr<bElem> &plr) const
{
    const std::size_t slots = (std::size_t) this->cfg.inventorySlots;
    const std::size_t per = this->itemReads.size();
    s.inventory.assign(this->shownSections.size() * slots * per, 0.0f);
    s.inventoryCounts.assign(this->shownSections.size(), 0.0f);
    for (std::size_t i = 0; i < s.inventory.size(); i++)
        s.inventory[i] = this->itemReads[i % per].absent;
    const auto inv = plr ? plr->getAttrs()->getInventory() : nullptr;
    if (!inv)
        return;
    const auto weapon = inv->getActiveWeapon();
    const auto usable = inv->getUsable();
    for (std::size_t sec = 0; sec < this->shownSections.size(); sec++) {
        const auto &items = itemsOf(*inv, this->shownSections[sec]);
        s.inventoryCounts[sec] = (float) items.size();
        for (std::size_t k = 0; k < slots && k < items.size(); k++) {
            const std::size_t base = (sec * slots + k) * per;
            for (std::size_t f = 0; f < per; f++)
                s.inventory[base + f] = this->itemReads[f].read(*items[k]);
            if (this->selectedAt >= 0)
                s.inventory[base + (std::size_t) this->selectedAt]
                    = items[k] == weapon || items[k] == usable ? 1.0f : 0.0f;
        }
    }
}

} // namespace goe::agent
