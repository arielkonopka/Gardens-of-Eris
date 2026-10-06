# Python bindings: the `goe` package

`agent/python` is the Python package `goe`. It binds the C++ agent library
([agent-interface.md](agent-interface.md)) with [nanobind](https://github.com/wjakob/nanobind)
and adds two ways to drive it on top: `goe.Game`, shaped like ViZDoom's `DoomGame`, and
`goe.GoeEnv`, a Gymnasium environment. How to install and use it is in
[agent/python/README.md](../../agent/python/README.md); this page describes each class.

```
goe/
  __init__.py   the public names
  game.py       Game, GameState, EVENTS, EVENT_MEANINGS, SHAPED_REWARD, default_data_dir
  env.py        GoeEnv, GLYPHS
  pattern.py    ChunkPattern, ELEMENT_TYPES, CHUNK_SIZE, as_pattern
  _goe          the compiled extension (src/bindings.cpp)
```

## Layers

```
training script
   |  GoeEnv.reset / step              (env.py: Gymnasium spaces, info dict, render)
   |  Game.new_episode / make_action   (game.py: keyword options, action names, GameState)
   |  _goe.Game                        (bindings.cpp: numpy arrays, dicts, GIL released)
   v  goe::agent::game                 (agentGame.h, C++)
```

Each layer only converts; the rules live in C++. One game per process holds here too: a second
`Game` raises until the first is closed (`Game.close()`, `GoeEnv.close()`, or leaving a `with`
block).

## Building

- `pip install ./agent/python`: `pyproject.toml` uses scikit-build-core with
  `cmake.source-dir = "../.."`, so pip builds the repository's root `CMakeLists.txt` with
  `GOE_BUILD_PYTHON=ON` and `GOE_BUILD_TESTS=OFF`, only the `_goe` target, and installs the
  `python` component (`_goe` plus `goe/*.py`).
- `cmake -S . -B build -DGOE_BUILD_PYTHON=ON`: `agent/python/CMakeLists.txt` stages the package in
  `build/agent/python/package` and registers `python-tests` (pytest) with ctest.
- The extension links `goe-agent`, which is built from a second, position independent copy of the
  game's objects (`GardenOfErisPicLib`), so the game executable is built as before.
- `GOE_DEFAULT_DATA_DIR` is compiled in as the checkout's `GoEoOL` folder.

## The extension module `goe._goe` (`agent/python/src/bindings.cpp`)

Not meant to be used directly, but everything the Python files use comes from here.

### Module attributes

| Name | Value |
|---|---|
| `DEFAULT_DATA_DIR` | the `GoEoOL` folder of the checkout the module was built from |
| `ACTIONS` | action names, in `goe::agent::action` order (`NOOP`, `MOVE_UP`, ..., `GIVE_UP`) |
| `EVENTS` | event names (`score`, `collect`, ..., `death`, `explore`, `avatar`) |
| `SECTIONS` | inventory section names (`weapons`, `usables`, `keys`, `mods`, `tokens`) |
| `ELEMENT_FEATURES`, `PLAYER_FEATURES` | the names of the C++ feature tables |
| `CELL_FEATURES` | `["exists", "in_sight", "visits", "seen", "novelty"]` |
| `ITEM_FEATURES` | `["selected"]` |
| `ELEMENT_TYPES` | `{name: type}` from `goe::chunkPattern::typeNames()` |
| `CHUNK_SIZE` | `chamber::chunkSize` (64) |
| `describe_features()` | `"name: meaning"` for every element and player feature |

### `_goe.ChunkPattern`

Binds `goe::chunkPattern`. `ChunkPattern(width, height, types, subtypes)` takes flat lists, row
by row (raises `ValueError` when their lengths differ); `from_json(text)` and `load(path)` are
static; `width`, `height`, `types`, `subtypes` are read-only; `==` compares two patterns. C++
`std::invalid_argument` becomes Python `ValueError`.

### `_goe.Config`

Binds `goe::agent::config` field by field, in snake case (`vision_radius`, `follow_player`,
`reward_weights`, ...). Three fields are converted: `fixed_centre` is an `(x, y)` tuple,
`inventory_sections` a list of section names (an unknown name raises `ValueError`), and
`chunk_patterns` / `default_pattern` hold `_goe.ChunkPattern` values. The game keeps its own copy
of every pattern, so changing a Python pattern later does not reach it.

### `_goe.Game`

Binds `goe::agent::game`:

| Python | C++ |
|---|---|
| `Game(config)` | `game(config)` |
| `new_episode(seed=None)` | `newEpisode` (GIL released) |
| `make_action(index)` | `makeAction` (GIL released) |
| `get_state()` | `getState`, as a dict: `vision`, `player`, `inventory`, `inventory_counts` (numpy `float32` arrays that own their memory, shaped as in `state`), `centre` (tuple), `tick`, `score` |
| `is_episode_finished()`, `is_player_dead()`, `is_truncated()` | the same |
| `action_taken`, `episode_tick`, `avatars_lost`, `score`, `seed` | read-only properties |
| `step_events`, `episode_events` | `{event name: count}` dicts |
| `set_chunk_pattern(chunk, pattern)`, `set_default_pattern(pattern)`, `clear_chunk_patterns()` | the pattern setters (`pattern` may be `None`) |
| `chunk_at(x, y)` | `chunkAt` |
| `generate_chunk(chunk, pattern)` | `generateChunk` |
| `action_count`, `cell_feature_names`, `player_feature_names`, `item_feature_names`, `inventory_section_names` | the shape of things |

Releasing the GIL in `new_episode` and `make_action` lets other Python threads run while the game
ticks; the game itself still runs on the calling thread.

## `goe.Game` (`game.py`)

The class to use. Keyword options instead of a config object, actions by name, and a dataclass
for the state.

```python
game = goe.Game(vision_radius=8, cell_features=["type", "in_sight"], reward_weights=goe.SHAPED_REWARD)
game.new_episode(seed=1)
while not game.is_episode_finished():
    s = game.get_state()
    reward = game.make_action("MOVE_RIGHT")
game.close()
```

### Constructor

`Game(data_dir=None, vision_radius=8, circle=True, follow_player=True, fixed_centre=(0, 0),
cell_features=None, player_features=None, inventory_sections=None, inventory_slots=5,
item_features=None, ticks_per_step=8, episode_ticks=0, allow_give_up=False,
reward_weights=None, chunk_patterns=None, default_pattern=None)`

Each option is the `config` field of the same name (see
[agent-interface.md](agent-interface.md#goeagentconfig)). Python-side additions:

- `data_dir=None` uses `default_data_dir()`.
- `None` for a feature or section list means all of them (the C++ default).
- `reward_weights` is checked against `EVENTS` first; an unknown event raises `ValueError` listing
  the known ones. `None` keeps the C++ default, the score.
- `chunk_patterns` maps `(chunk x, chunk y)` to a `ChunkPattern` or the path of a JSON pattern;
  `default_pattern` takes either too.

### Methods

| Method | What it does |
|---|---|
| `new_episode(seed=None)` | A new world; the seed is masked to 32 bits. |
| `make_action(action)` | One step. `action` is an index into `available_actions`, a name (`"MOVE_UP"`), or a sequence with one number per action (ViZDoom's buttons; the largest one is played; a wrong length raises `ValueError`). Returns the reward. |
| `get_state()` | A `GameState`. |
| `is_episode_finished()`, `is_player_dead()`, `is_truncated()` | As in C++. |
| `close()` | Lets go of the world, so another `Game` can start in this process. `Game` is also a context manager. |
| `set_chunk_pattern(chunk, pattern)` | The chunk's own pattern (a `ChunkPattern`, a path, or `None` to remove it), for chunks made from now on. |
| `set_default_pattern(pattern)` | The pattern of every chunk without its own. |
| `clear_chunk_patterns()` | Random mazes again. |
| `chunk_at(x, y)` | The `(x, y)` of the chunk holding a cell, the cell counted from the middle of the start area (as `GameState.centre` and the player's `x` and `y`). |
| `generate_chunk(chunk, pattern=None)` | Within an episode: with a pattern, sets it as the chunk's own. Returns `True` when the chunk is in memory, `False` when it is not or no episode has started. It does not build a missing chunk or rebuild one already built; see the note in [agent-interface.md](agent-interface.md#chunk-patterns). |

### Properties

`action_taken`, `step_events`, `episode_events`, `avatars_lost`, `episode_tick`, `score`, `seed`
as in C++; `available_actions` (the action names this game accepts), `cell_features`,
`player_features`, `item_features`, `inventory_sections` (the names in observation order);
`vision_radius` and `inventory_slots` as given.

### Module-level names in `game.py`

| Name | What it is |
|---|---|
| `ACTIONS` | `list(_goe.ACTIONS)` |
| `EVENTS` | `list(_goe.EVENTS)` |
| `EVENT_MEANINGS` | `{event: what it counts}` |
| `SHAPED_REWARD` | a starting point for `reward_weights`: collect +5, apple +20, use +2, open +10, teleport +5, kill +10, mine +5, hurt -0.2, death -50 |
| `default_data_dir()` | `$GOE_DATA_DIR`, else `_goe.DEFAULT_DATA_DIR` when it holds `data/skins.json`, else `""` (the working directory) |

## `goe.GameState` (`game.py`)

A dataclass made from `_goe.Game.get_state()`:

| Field | Type | Holds |
|---|---|---|
| `vision` | `float32 (channels, rows, columns)` | channel c is `cell_features[c]`; the cell at (row, column) is `centre + (column - radius, row - radius)`, y growing downwards |
| `player` | `float32 (player_features,)` | the active avatar |
| `inventory` | `float32 (sections, slots, item_features)` | an empty slot has type -1 |
| `inventory_counts` | `float32 (sections,)` | how many items each section holds |
| `centre` | `(x, y)` | the vision's centre, from the middle of the start area |
| `tick` | `int` | ticks since the episode started (50 a second) |
| `score` | `int` | the active avatar's total points |

`game_variables` is `player` under ViZDoom's name.

## `goe.GoeEnv` (`env.py`)

A Gymnasium environment around `Game`. With gymnasium installed it is a `gymnasium.Env` with
spaces; without it, a plain class with the same `reset` and `step`.

| Member | What it does |
|---|---|
| `GoeEnv(render_mode=None, **game_options)` | Every keyword goes to `Game`. `render_mode` may be `"ansi"`. |
| `action_space` | `Discrete(len(game.available_actions))` |
| `observation_space` | `Dict` of `Box(-inf, inf, float32)`: `vision`, `player`, `inventory`, `inventory_counts`, shaped from the game's feature lists |
| `reset(seed=None, options=None)` | A new world. With a seed, the same world for the same seed; without one, a world seed drawn from the environment's `np_random` (seeded by the first seeded reset). Returns `(observation, info)`. |
| `step(action)` | `make_action(int(action))`; returns `(observation, reward, terminated, truncated, info)`. `terminated`: the last avatar is gone. `truncated`: `episode_ticks` passed (and not terminated). |
| `render()` | With `"ansi"` and a `type` cell feature: the vision as text, one glyph per element type (`GLYPHS`: `#` wall, `@` player, `k` key, `D` door, `A` golden apple, `M` monster, `?` anything else). |
| `close()` | Closes the game. |
| `game` | The `Game` underneath, for anything the environment does not expose (patterns, events). |

`info` holds `score`, `tick`, `centre`, `seed`, `action_taken`, `avatars_lost` and `events` (the
step's event counts).

## `goe.ChunkPattern` (`pattern.py`)

A fixed layout of elements for one chunk, built instead of a random maze. The Python class
wraps `_goe.ChunkPattern` (its `_native`) and speaks numpy.

| Member | What it does |
|---|---|
| `ChunkPattern(types, subtypes=None)` | From 2-D arrays (rows, columns): element types (`ELEMENT_TYPES` values, -1 for nothing, 0 for the floor itself with the subtype as its look) and subtypes (`None`: all 0). Raises `ValueError` for a non 2-D or mismatched shape, a pattern larger than `CHUNK_SIZE`, or a type a pattern may not place (missiles, unknown numbers). |
| `ChunkPattern.from_rows(rows, legend)` | From strings of equal length and `{character: entry}`, an entry being `None`, a type (number or name) or `(type, subtype)`. Characters missing from the legend, unknown names and multi-character keys raise `ValueError`. |
| `ChunkPattern.from_json(text)`, `ChunkPattern.load(path)` | `{"legend": {...}, "rows": [...]}`; see `agent/patterns/rooms.json`. |
| `width`, `height` | in cells |
| `types`, `subtypes` | `int64 (rows, columns)` arrays |
| `==`, `repr` | compare, `ChunkPattern(8 x 8)` |

`ELEMENT_TYPES` is `{"floor": 0, "rubbish": 2, "wall": 4, "monster": 6, "drone": 7,
"brick_cluster": 8, "key": 51, "door": 52, "puppet_master": 77, "camera": 78, "player": 100,
"gun": 200, "bazooka": 204, "bunker": 250, "teleporter": 400, "bouba": 555, "kiki": 556,
"bomb": 602, "landmine": 603, "golden_apple": 900}`. `as_pattern(p)` turns a `ChunkPattern`, a
path or `None` into a `ChunkPattern` or `None` (anything else raises `TypeError`); `Game` uses it
for every pattern argument.

How a pattern is laid on a chunk (tiling, the start player, chunk walls) is described with
`goe::chunkPattern` in [world.md](world.md#goechunkpattern-chunkpatternh).

## Tests

`agent/python/tests` (pytest, 16 tests): `test_env.py` covers the default observation, choosing
its composition, the fixed centre, refusing unknown names, one game per process, actions by
index, name and buttons, same seed same game, truncation, the Gymnasium environment, and rewards
from events; `test_pattern.py` covers making patterns, refusing bad ones, the agent seeing a
pattern, one pattern for every chunk, changing and clearing patterns, and the return values of
`generate_chunk`.
