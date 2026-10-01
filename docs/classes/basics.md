# Basics: types, coordinates, directions, controls

Small types every other module uses. Most live in `include/commons.h`.

## `bElemTypes` (namespace, `commons.h`)

The number each element kind returns from `bElem::getType()`, also used in `skins.json`, save
files, chunk patterns and the agent's `type` feature.

| Constant | Value | Class |
|---|---|---|
| `_belemType` | -1 | nothing (a bare `bElem`) |
| `_floorType` | 0 | `floorElement` |
| `_rubishType` | 2 | `rubbish` |
| `_wallType` | 4 | `wall` |
| `_monster` | 6 | `monster` |
| `_patrollingDrone` | 7 | `patrollingDrone` |
| `_brickClusterType` | 8 | `brickCluster` |
| `_key` | 51 | `key` |
| `_door` | 52 | `door` |
| `_puppetMasterType` | 77 | `puppetMasterFR` and its kinds |
| `_securityCamera` | 78 | `securityCamera` |
| `_player` | 100 | `player` |
| `_plainGun` | 200 | `plainGun` |
| `_plainMissile` | 201 | `plainMissile` |
| `_bazookaType` | 204 | `bazooka` |
| `_bazookaMissileType` | 205 | `bazookaMissile` |
| `_bunker` | 250 | `bunker` |
| `_teleporter` | 400 | `teleport` |
| `_boubaType` | 555 | `bouba` |
| `_kikiType` | 556 | `kiki` |
| `_stash` | 600 | a stash (counted apart in inventories) |
| `_simpleBombType` | 602 | `simpleBomb` |
| `_landmineType` | 603 | `landmine` |
| `_goldenAppleType` | 900 | `goldenApple` |

## `GoEConstants` (namespace, `commons.h`)

Timing and weapon numbers, built from fives: `_mov_delay` (8 ticks a move), `_mov_delay_push`,
`_bazookaMaxSteps`, `_plainGunAmmo`, `_plainMissileSpeed`, `_bazookaMissileSpeed`,
`_plainGunCharge`, `_teleportationTime`, `_teleportStandTime`, `_maxWaitingTtime`,
`_defaultKillTime`, `_defaultDestroyTime`, `_dividerCloak`, `_interactedTime`,
`_radioActivityPower`, `_radioActivitySpeed`. Tunings that change with the difficulty are not
here but in `difficulty` (see [rules.md](rules.md#difficulty-namespace-difficultyh)).

## `coords` (struct, `commons.h`)

A cell `(x, y)` on a board: x to the right, y downwards, both may be negative on the endless
world. Value type with `==`, `!=`, `+`, `-`, `*`, `/`, `%` (element-wise helpers, not vector
maths), `sum2d()` and `distance()` (Euclidean, computed in double so far cells never overflow).

- `NOCOORDS`: "on no cell", far outside any board and safe to add small offsets to.
- `floorDiv(a, b)`, `floorMod(a, b)`: division rounding down, for negative cells.

## `myUtility::Coords` (class, `Coords.h`)

The older coordinates class with private x, y and an optional z. It converts to and from
`coords` and is still used by a few board methods (`chamber::getElement(myUtility::Coords)`,
`explosives`), `dir2coords(direction)` gives a unit step. New code uses `coords`.

## `dir::direction` (enum class, `commons.h`)

`UP = 0`, `LEFT = 1`, `DOWN = 2`, `RIGHT = 3`, `NODIRECTION = 4`. The namespace also holds
`allDirections`, `allDirectionsOpposite`, `dirToCoords(d)` (UP is `(0, -1)`) and
`getOppositeDirection(d)`. Turning by one is `(d + 1) % 4` to the left.

## `controlItem` (struct, `commons.h`)

One command for the player, `{int type, dir::direction dir}`, made by
`goe::controls::bindings::translate`, by the autopilot or by the agent, and read by
`player::mechanics`:

| type | Command |
|---|---|
| -1 | nothing |
| 0 | walk in `dir` |
| 1 | shoot in `dir` |
| 2 | interact with, or pick up, what is in `dir` |
| 3 | select the next usable |
| 4 | drag in `dir` |
| 5 | select the next kind of gun |
| 6 | give up this avatar |
| 7 | quit without saving |
| 8 | use the usable in hand |
| 9 | drop the usable in hand |
| 10 | save and go back to the title screen |

## Other small types

| Name | What it is |
|---|---|
| `sNeighboorhood` | Steppable flags and types of the eight cells around an element (`bElem::getSteppableNeighborhood`). |
| `coords3d` | Float x, y, z, for sound positions. |
| `pointsType` | Keys of `bElemStats`' points: `TOTAL`, `SHOOT`, `STEPS`, `COLLECTS`. |
| `modType`, `mechanicResult` | Older enums kept for modifiers and mechanics results. |
| `oState` (`bElem.h`) | What `disposeElement` returns: `DISPOSED`, `nullptrREACHED`, `ERROR`. |
| `objectTypes.h` | Empty; a few headers still include it. |
