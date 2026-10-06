# Agent interface (C++): `goe::agent`

The `goe-agent` library (`agent/`) runs Gardens of Eris without a window, a sound or a keyboard,
so that a program can play it: a learning agent, a test, a scripted scenario. It is the layer the
Python package binds (see [python-package.md](python-package.md)).

| File | What is in it |
|---|---|
| `agent/include/agentGame.h`, `agent/src/agentGame.cpp` | `game`, `config`, `state`, `action`, `section`, `event` |
| `agent/include/agentFeatures.h`, `agent/src/agentFeatures.cpp` | `feature` and the feature tables |
| `agent/tests/agent-test.cpp` | GoogleTest program `agent-test` (23 tests) |
| `agent/patterns/rooms.json` | an example chunk pattern |
| `agent/CMakeLists.txt` | the `goe-agent` static library and `agent-test` |

The shape follows ViZDoom's `DoomGame`, which exRelaxer already drives: `newEpisode`, `getState`,
`makeAction`, `isEpisodeFinished`.

```cpp
#include "agentGame.h"
using namespace goe::agent;

config c;
c.dataDir = "GoEoOL";                  // the folder with data/skins.json
c.cellFeatures = {"type", "in_sight"};
c.rewardWeights = {{"collect", 5}, {"death", -50}};
game g(c);
g.newEpisode(4242);
while (!g.isEpisodeFinished()) {
    state s = g.getState();
    float reward = g.makeAction(action::moveRight);
}
```

## Rules that hold for every class here

- **One game per process.** The game keeps its world in static state (`chamber::allChambers`,
  the active player, the apple and teleporter registries). A second `game` throws
  `std::runtime_error` until the first one is destroyed. Run games in parallel in separate
  processes.
- **Determinism.** `newEpisode(seed)` sets the world seed (`goe::rng::setWorldSeed`), reseeds the
  running game's engine (`goe::rng::saved()`) and resets `gameClock`, so the same seed and the
  same actions play the same game.
- **Single thread.** Everything runs on the caller's thread: the world grows and shrinks inside
  `advance`, there is no input thread (`inputManager::getInstance(true)` is in test mode) and
  OpenAL is pointed at its null driver (`ALSOFT_DRIVERS=null`), so no sound card is needed.
- **Coordinates as the agent sees them.** Cells are counted from `chamber::origin`, the middle of
  the start area, x to the right and y downwards. Chunks are counted from the start chunk `(0, 0)`:
  `(1, 0)` is east of it, `(0, -1)` north of it.

## `goe::agent::game`

The game itself. Not copyable.

### Lifetime

| Member | What it does |
|---|---|
| `explicit game(config cfg)` | Checks the config (`visionRadius >= 0`, `ticksPerStep >= 1`, `inventorySlots >= 0`, known reward events and feature names; throws `std::invalid_argument` otherwise), claims the one game of the process, changes into `dataDir` just long enough to read `skins.json` (`configManager`), starts `inputManager` in test mode and starts listening to `goe::events`. |
| `~game()` | Stops listening to events, clears the world (`gameSerializer::clearWorld`), drops every chunk pattern and frees the process for the next game. |

### Episodes and steps

| Member | What it does |
|---|---|
| `void newEpisode(std::optional<std::uint32_t> seed = std::nullopt)` | Clears the world and builds a new one: the start chunk and every chunk within `worldBuilder::buildRadius` (2) of it, from the configured patterns or as random mazes. No seed: a fresh one from the system (`goe::rng::freshSeed`). Resets the tick count, the score baseline, the event counts and the once-an-episode sets. |
| `float makeAction(action a)` | Plays one step: `config::ticksPerStep` ticks (default 8, one move). The action reaches the player once, in the first tick it can act (when every timer it waits on runs out by the next tick), like a key pressed once; the other ticks get no command. Stops early when the episode ends. Returns the reward: each event's count in the step times its weight. Throws `std::out_of_range` for an action outside `actions()`. |
| `bool advance(controlItem control)` | One game tick with a raw control (`commons.h`). Grows the world around the player or swaps one far chunk out (as the presenter does), then runs `bElem::runLiveElements`. False once the episode is over. Use it to drive the game tick by tick; `makeAction` is built on it. |
| `bool actionTaken() const` | Whether the last `makeAction` reached the player; false when the player was busy (moving, dying, teleporting) for the whole step. |

### State and ending

| Member | What it does |
|---|---|
| `state getState() const` | The observation: the vision grid, the player's numbers, the inventory (see `state`). |
| `bool isEpisodeFinished() const` | `isPlayerDead() || isTruncated()`. |
| `bool isPlayerDead() const` | No active player is left: game over. |
| `bool isTruncated() const` | `episodeTicks` is set and that many ticks passed. The maze never ends, so this is how an episode is cut. |
| `std::uint64_t episodeTick() const` | Ticks since `newEpisode`. |
| `int avatarsLost() const` | How many times a spare avatar took over from a lost one. |
| `int score() const` | The active avatar's total points (`pointsType::TOTAL`), or the last score once the last avatar is gone. |
| `std::uint32_t seed() const` | The world seed of this episode. |
| `const eventCounts &stepEvents() const` | What happened in the last `makeAction`, one count per `event`. |
| `const eventCounts &episodeEvents() const` | The same, summed over the episode. |

### Chunk patterns

A chunk can be built from a fixed `goe::chunkPattern` instead of a random maze
(see [world.md](world.md#goechunkpattern-chunkpatternh)). The patterns live in `config::chunkPatterns` and
`config::defaultPattern`, and are handed to `worldBuilder` whenever they change and at every
`newEpisode`.

| Member | What it does |
|---|---|
| `void setChunkPattern(std::pair<int, int> chunk, std::shared_ptr<const chunkPattern> pattern)` | The chunk's own pattern; `nullptr` removes it. Applies to chunks made from now on: those not built yet in this episode, and every chunk of the next episodes. |
| `void setDefaultPattern(std::shared_ptr<const chunkPattern> pattern)` | The pattern of every chunk without its own; `nullptr`: random mazes. |
| `void clearChunkPatterns()` | Every chunk a random maze again. |
| `std::pair<int, int> chunkAt(coords cell) const` | The chunk holding a cell given as the agent sees it (from `chamber::origin`). |
| `bool generateChunk(std::pair<int, int> chunk, std::shared_ptr<const chunkPattern> pattern = nullptr)` | Within an episode: with a pattern, sets it as the chunk's own (as `setChunkPattern`). Returns whether the chunk is in memory; false when no episode has started. As it is now it does not build a missing chunk or read one back from disk: it hands the chunk's coordinates to `worldBuilder::bringIn`, which expects a cell. The 5 x 5 chunks around the start are built with the episode, so for those it returns true and the pattern shows from the next episode on; a chunk further out takes the pattern when the player comes near it. |

### The shape of things

| Member | What it returns |
|---|---|
| `const config &getConfig() const` | The config, with the patterns as they are now. |
| `int actions() const` | The number of actions: `actionCount`, or one fewer without `allowGiveUp`. |
| `cellFeatureNames()`, `playerFeatureNames()`, `itemFeatureNames()` | The feature names in observation order (the defaults filled in). |
| `sections()` | The inventory sections shown, in order. |

### How events are counted

`game` listens to `goe::events` (see [rules.md](rules.md#goeevents-namespace-gameeventsh)) and counts only what the
agent's own avatars did (the actor is a `player`):

| `event` | Counted when |
|---|---|
| `score` | Points gained in the step. When a spare avatar takes over, that step's score is 0. |
| `collect` | The player collected an item, each item once an episode (by instance id). |
| `apple` | The collected item was a golden apple (it counts here instead of `collect`). |
| `use` | The usable in hand was used. |
| `open` | A door was opened, each door once an episode. |
| `teleport` | The player went through a teleporter. |
| `kill` | A monster, drone or puppet master was killed or destroyed by the player's missile or blast (`goe::events::blame`). |
| `mine` | A bomb or landmine was set off the same way. |
| `hurt` | Energy the same avatar lost during the step (energy gained is not subtracted). |
| `death` | An avatar was lost, the last one included. |
| `explore` | Cells that came into the player's sight for the first time this episode (counted at the end of the step). A small weight makes it a count-based exploration bonus. |
| `avatar` | The player woke a spare avatar by walking into it (`goe::events::kind::activate`); each avatar once, since a woken one stays marked. |

## `goe::agent::config`

How the game is observed and played. Every field has a default.

| Field | Default | Meaning |
|---|---|---|
| `std::filesystem::path dataDir` | empty (working directory) | The folder with `data/skins.json`, the game's `GoEoOL` folder. |
| `int visionRadius` | 8 | The vision grid is `2 * visionRadius + 1` cells on each side. |
| `bool circle` | true | Cells further than the radius from the centre are left empty (their absent values). |
| `bool followPlayer` | true | Centre the vision on the active player; false: on `fixedCentre`. |
| `coords fixedCentre` | (0, 0) | The fixed centre, from `chamber::origin`. |
| `std::vector<std::string> cellFeatures` | all | What each vision cell holds: element feature names plus the cell names (`cellOnlyFeatureNames()`): `exists`, `in_sight`, `visits`, `seen`, `novelty`. |
| `std::vector<std::string> playerFeatures` | all | Element and player feature names. |
| `std::vector<section> inventorySections` | all | The sections shown, in order. |
| `int inventorySlots` | 5 | Items shown per section; more are left out, fewer leave empty slots (type -1). |
| `std::vector<std::string> itemFeatures` | `type, subtype, energy, ammo, max_ammo, selected` | Element feature names plus `selected`. |
| `int ticksPerStep` | `GoEConstants::_mov_delay` (8) | Game ticks per `makeAction`; the game runs 50 ticks a second. |
| `std::uint64_t episodeTicks` | 0 | Truncate the episode after this many ticks; 0 for no limit. |
| `bool allowGiveUp` | false | Adds `action::giveUp` (the avatar dies). |
| `std::map<std::string, float> rewardWeights` | `{"score": 1}` | Reward = sum of each event's count times its weight; events left out weigh 0. |
| `std::map<std::pair<int, int>, std::shared_ptr<const chunkPattern>> chunkPatterns` | none | Chunks built from a pattern, by chunk. |
| `std::shared_ptr<const chunkPattern> defaultPattern` | none | The pattern of every other chunk; none: random mazes. |

## `goe::agent::state`

One observation. Flat `float` vectors, so a binding can hand them out as arrays without copying
element by element.

| Field | Shape | Holds |
|---|---|---|
| `int visionSide` | | `2 * visionRadius + 1` |
| `std::vector<float> vision` | channels x rows x columns | Channel c is cell feature c. Cell (row r, column c) is `centre + (c - radius, r - radius)`. A cell that is not built yet, or outside the circle, holds each feature's absent value. |
| `std::vector<float> player` | player features | The active avatar; absent values once the last avatar is gone. |
| `std::vector<float> inventory` | sections x slots x item features | The first `inventorySlots` items of each section; an empty slot holds absent values (type -1). |
| `std::vector<float> inventoryCounts` | sections | How many items each section holds, all of them. |
| `coords centre` | | The centre of the vision, from `chamber::origin`. |
| `std::uint64_t tick` | | Ticks since the episode started. |
| `int score` | | As `game::score()`. |

## `goe::agent::action`

What the agent can do in one step: the game's own controls, one at a time.

| Value | Name (`actionName`) | Control item (`controlOf`) |
|---|---|---|
| `noop` | `NOOP` | -1, no command |
| `moveUp` ... `moveRight` | `MOVE_UP` ... | 0, walk |
| `shootUp` ... `shootRight` | `SHOOT_UP` ... | 1, shoot the active gun |
| `interactUp` ... `interactRight` | `INTERACT_UP` ... | 2, interact or pick up |
| `dragUp` ... `dragRight` | `DRAG_UP` ... | 4, drag what is behind |
| `nextItem` | `NEXT_ITEM` | 3, next usable |
| `nextGun` | `NEXT_GUN` | 5, next kind of gun |
| `use` | `USE` | 8, use the usable in hand |
| `drop` | `DROP` | 9, drop the usable in hand |
| `giveUp` | `GIVE_UP` | 6, the avatar dies (only with `allowGiveUp`) |

`actionCount` is the number of values; `std::string actionName(action)` and
`controlItem controlOf(action)` read the table in `agentGame.cpp`.

## `goe::agent::section`

The inventory parts the agent can see: `weapons`, `usables`, `keys`, `mods`, `tokens`
(`sectionCount`, `sectionName`). They are the five vectors of `inventory`
(see [elements-core.md](elements-core.md#inventory-inventoryh)).

## `goe::agent::event`

What the agent's avatar did or suffered in a step: `score`, `collect`, `apple`, `use`, `open`,
`teleport`, `kill`, `mine`, `hurt`, `death`, `explore`, `avatar` (see the table above). `eventCount`,
`eventName(event)`, `eventByName(name)` and `eventCounts` (one `float` per event) go with it.

## `goe::agent::feature` (`agentFeatures.h`)

One number read from an element:

```cpp
struct feature {
    std::string_view name;
    std::string_view about;
    float (*read)(bElem &);
    float absent = 0.0f;   // where there is no element
};
```

The same table describes a vision cell (its top element), an inventory item and the player, so
a name means the same everywhere. Yes/no qualities are 0 or 1, timed states are the ticks left
(0 when not in that state), directions are `dir::direction` (0 up, 1 left, 2 down, 3 right,
4 none).

| Function | Returns |
|---|---|
| `std::span<const feature> elementFeatures()` | What any element has. |
| `std::span<const feature> playerFeatures()` | What only the player has. |
| `std::vector<std::string> namesOf(table)` | The names of a table, in order. |
| `std::vector<feature> select(names, {tables...})` | The named features, looked up in the tables in turn; throws `std::invalid_argument` naming the first unknown one. |

Element features: `type`, `subtype` (absent -1), `energy`, `max_energy`, `ammo`, `max_ammo`,
`killable`, `destroyable`, `steppable`, `movable`, `interactive`, `collectible`, `can_push`,
`can_be_pushed`, `can_collect`, `weapon`, `open`, `locked`, `mod`, `active`, `busy`, `moving`,
`waiting`, `dying`, `destroying`, `teleporting`, `fading_in`, `fading_out`, `interacting`,
`facing`, `direction`, `items`, `stack`, `below_type`.

Player features: `x`, `y`, `score`, `shots`, `steps`, `collects`, `view_radius`, `dex`,
`difficulty`, `avatars`.

Names only one place has, filled in by `game` itself: `exists` and `in_sight` for cells,
`selected` for items. Cells also carry the episode's memory of them:

| Name | Value |
|------|-------|
| `visits` | times the player stepped onto the cell this episode (the start cell counts once) |
| `seen` | steps that ended with the cell within the player's view radius (as `in_sight`); cells not built yet are never seen |
| `novelty` | 1 / sqrt(1 + `seen`): 1 for a cell never seen, falling as it grows familiar |

The memory is kept by `game` per board cell (`cellMemory`), so it survives chunks going to disk,
and is cleared by `newEpisode`. A visit is counted in every tick the active avatar stands on a
cell other than in the tick before; sight is counted once per `makeAction`. Unlike an element's
instance id, these values mean the same thing on every cell, so a network still learns
patterns that carry over from place to place, while an empty floor no longer looks the same
before and after a step.

To add a feature, add a row to the table in `agentFeatures.cpp`; it shows up in the C++ defaults,
in Python's `ELEMENT_FEATURES` or `PLAYER_FEATURES`, and in `describe_features()` without
further changes. Keep `agent/python/README.md` in step.

## Tests

`agent-test` runs from the build folder with `GOE_DATA_DIR` pointing at `GoEoOL`, so it checks
that the game finds its data from elsewhere. It covers the actions, refused features and events,
one game per process, the default observation, the circle and the fixed centre, chosen player and
inventory features, a step moving one cell, same seed same game, truncation, giving up, the
reward and every event, and the chunk patterns (`PatternTests`).
