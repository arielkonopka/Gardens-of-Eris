# goe: Gardens of Eris for agents

The game without a window, a sound or a keyboard, so a program can play it. The C++ side is
`agent/include/agentGame.h` (the `goe-agent` library); this package binds it with
[nanobind](https://github.com/wjakob/nanobind), the way exRelaxer binds its own library.

Two ways to drive it:

- `goe.Game`, shaped like ViZDoom's `DoomGame` (the interface exRelaxer's Doom experiments use):
  `new_episode`, `get_state`, `make_action`, `is_episode_finished`.
- `goe.GoeEnv`, a Gymnasium environment: `reset`, `step`, `observation_space`, `action_space`
  (a plain class with the same methods when gymnasium is not installed).

## Installing

From a checkout (the package builds the game in the repository around it):

```bash
pip install ./agent/python            # needs a C++23 compiler, CMake, and the game's libraries
pip install "./agent/python[test]"    # with pytest and gymnasium
```

Or in the game's own CMake build, next to the unit tests (nanobind installed in the Python
CMake finds): `cmake -S . -B build -DGOE_BUILD_PYTHON=ON`, then the package is in
`build/agent/python/package` and `ctest` runs its tests as `python-tests`.

The game reads `data/skins.json`. The package looks for it in `$GOE_DATA_DIR`, then in the
`GoEoOL` folder of the checkout it was built from; `Game(data_dir=...)` says it directly.

## Example

```python
import goe

env = goe.GoeEnv(vision_radius=8, episode_ticks=50 * 60 * 5)   # five minutes of game time
obs, info = env.reset(seed=4242)
done = False
while not done:
    obs, reward, terminated, truncated, info = env.step(env.action_space.sample())
    done = terminated or truncated
env.close()
```

As ViZDoom:

```python
game = goe.Game(cell_features=["type", "subtype", "dying", "in_sight"],
                player_features=["energy", "ammo", "score", "view_radius"])
game.new_episode(seed=1)
while not game.is_episode_finished():
    state = game.get_state()          # state.vision, state.player (also state.game_variables), state.inventory
    reward = game.make_action("MOVE_RIGHT")   # an index, a name, or one number per action (the largest wins)
game.close()
```

## What the agent sees

| Part | Shape | Holds |
|------|-------|-------|
| `vision` | (cell features, 2r + 1, 2r + 1) | the top element of every cell in a circle of radius r around the player (`follow_player=False`: around `fixed_centre`) |
| `player` | (player features,) | the active avatar |
| `inventory` | (sections, slots, item features) | weapons, usables, keys, mods, tokens; an empty slot has type -1 |
| `inventory_counts` | (sections,) | how many items each section holds, also those beyond the slots |

Each part's composition is a list of names (`goe.describe_features()` says what each means):

- element features, for cells, items and the player: `type`, `subtype`, `energy`, `max_energy`,
  `ammo`, `max_ammo`, the qualities (`killable`, `steppable`, `collectible`, `locked`, ...), the
  timed states as ticks left (`moving`, `waiting`, `dying`, `teleporting`, ...), `facing` and
  `direction` (0 up, 1 left, 2 down, 3 right, 4 none), `items` carried, `stack` and `below_type`
  (what lies under it);
- cell features: `exists` (the cell is built), `in_sight` (within the player's view radius);
- player features: `x`, `y` (from the middle of the start area), `score`, `shots`, `steps`,
  `collects`, `view_radius`, `dex`, `difficulty`, `avatars` (spares collected);
- item features: `selected` (the active weapon, the usable in hand).

By default every cell and player feature is on, and items show `type`, `subtype`, `energy`,
`ammo`, `max_ammo`, `selected`.

## Actions, steps and rewards

The actions are the game's controls: `NOOP`, `MOVE_*`, `SHOOT_*`, `INTERACT_*`, `DRAG_*`
(`UP`, `DOWN`, `LEFT`, `RIGHT`), `NEXT_ITEM`, `NEXT_GUN`, `USE`, `DROP`, and `GIVE_UP` with
`allow_give_up=True`. A step runs `ticks_per_step` game ticks (default 8, one move; the game runs
50 ticks a second). The action reaches the player once, in the first tick it can act, like a key
pressed once; `game.action_taken` says whether it did.

The reward is the score gained in the step. When a spare avatar takes over from a lost one, that
step's reward is 0 and `avatars_lost` grows. The episode ends (`terminated`) when the last avatar
is gone; the maze never ends, so set `episode_ticks` to cut episodes (`truncated`).

The same seed builds the same world, and the same actions then play the same game. There is one
game per process: the game keeps its world in static state. Run games in parallel in separate
processes, as exRelaxer's searches do with workers.
