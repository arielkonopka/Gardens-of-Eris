# Controllers: puppet masters, roaming and line of sight

A puppet master is an element (type 77) that, once handed to a `patrollingDrone`, decides how the
drone moves. The same drone behaves differently depending on which controller drives it. The kind
of controller is the element's subtype, so the level generator, save files and `skins.json` all
use the one puppet master type.

```
puppetMasterFR (patrol, kind 0)
 ├─ puppetMasterCollector     (kind 1)
 ├─ puppetMasterHunter        (kind 2)
 ├─ puppetMasterWallFollower  (kind 3)
 ├─ puppetMasterGuardian      (kind 4, only with a security camera)
 └─ puppetMasterHound         (kind 5, only sent after a camping player)
```

The controlled drone calls `drive()` from its own `mechanics()`, so a controller runs wherever
its body is and is never a live element itself. When it takes over a drone it says "controller
enabled" in a robot voice, each kind in its own language.

## `puppetMasterFR` (`puppetMasterFR.h`)

The base class and the patrol behaviour.

| Member | What it does |
|---|---|
| `enum kind` | `patrol`, `collector`, `hunter`, `wallFollower`, `guardian`, `hound`; kinds below `looseKinds` are placed in the maze for the player to find. |
| `static create(board, subtype)` | Makes the controller class matching the subtype. |
| `virtual onAttach(body)` | Called once when the controller is handed to its body. |
| `virtual drive(body)` | Moves the body one step; the patrol roams the maze keeping the wall on its left (`goe::roam`). |
| `getLastSeen()` | Where this controller last saw the player; `NOCOORDS` when the trail is cold. |
| `collectibleBy(who)` | False for drones: a drone never picks up a loose controller, it walks around it. |

Helpers for subclasses (protected):

| Helper | What it does |
|---|---|
| `bite(body, prey, damage)` | Hurt the prey when it is right next to the body, then rest. |
| `lookout(body, range)` | The active player when they are within range with nothing opaque in between (`goe::sight`); remembers the cell as `lastSeen`. Chasers never read the player's position any other way. |
| `followTrail(body, preyInSight, centre, radius)` | Walk to `lastSeen` around walls, never leaving the circle; false when there is no trail to follow. |
| `pathTowards(body, goal, centre, radius)` | First step of a shortest walk to a cell next to `goal`, within the circle. |
| `towards`, `distance2` | Direction and distance arithmetic. |

To add a behaviour: subclass `puppetMasterFR`, override `drive()` (and `onAttach()` if needed),
add a kind to the enum and to `create()`. A new chaser uses `lookout`, `followTrail` and
`goe::roam::followWall`, never the player's position directly. Nothing that moves stands idle:
with nothing to do it roams with `goe::roam`.

## `puppetMasterCollector` (`puppetMasterCollector.h`)

Drives its body towards collectibles it sees in a straight line (`firstSolidInDirection`), and
roams the maze otherwise.

## `puppetMasterHunter` (`puppetMasterHunter.h`)

Chases the active player around walls while it sees them within `sightRange` (12); when they slip
out of sight it goes to where it saw them last (searching within `trailRadius`, 23), and when they
are not there it patrols the walls.

## `puppetMasterWallFollower` (`puppetMasterWallFollower.h`)

Keeps its right hand on the wall, so its body walks the maze systematically.

## `puppetMasterGuardian` (`puppetMasterGuardian.h`)

Drives a guardian drone of a `securityCamera` (`guard(cam)`, `getCamera()`). When it sees the
player within `difficulty::cameraSight` it fights (`fight`): bites when next to them
(`meleeDamage` 5), shoots along a clear row or column with its own gun, and chases otherwise. When
the player slips away, or the camera reports a new sighting, it checks the spot; with nobody to
find it patrols the walls around the camera. It never leaves the camera's leash (55 cells); when
the camera is gone it keeps to `home`. The state diagram is
[13-guardian-states](../diagrams/13-guardian-states.svg).

## `puppetMasterHound` (`puppetMasterHound.h`)

The anti-camping hunter. `static watch()` runs once a tick: when the active player stays in one
64 x 64 area (`difficulty::areaOf`) longer than `difficulty::houndPatience`, `release` puts a
drone driven by a hound `spawnDistance` (15) cells away. It patrols the walls until it sees the
player (within `searchRadius`, 55), chases and bites (`biteDamage` 5), follows the trail when they
slip away, and dies when they leave the area. One hound at a time; the patience starts over after
each.

## `goe::roam` (namespace, `roam.h`)

How creatures move about the maze when nothing else calls them. Monsters and every controller use
it, so an idle creature never stands still: it roams the maze along its walls.

- `hand`: `right` or `left`, the side a creature keeps the wall on.
- `followWall(body, side, centre, radius)`: one move of the wall follower rule. Around a corner
  when the wall at its side just ended, else straight, towards the wall, away from it, and back
  only in a dead end. One move in 23 it lets go of a corner, so it does not circle a pillar
  forever. When `radius > 0`, cells outside the circle around `centre` count as walls (a
  guardian's leash). Always does something: a step, or a turn when walled in.
- `turn(body, d)`, `step(body, d)`: face a direction and wait; move one cell and wait.
- `leftOf`, `rightOf`, `behind`: direction arithmetic.

## `goe::sight` (namespace, `lineOfSight.h`)

What elements can see; every watcher (cameras, guardians, hunters, hounds) looks through it.

- `opaque(e)`: walls, brick clusters, bunkers, teleporters, closed doors, and cells the board
  does not have. Floors, items and creatures do not block.
- `clear(board, from, to)`: nothing opaque on the straight line between two cells (Bresenham),
  the end cells excluded; a diagonal step does not squeeze between two opaque cells that touch at
  their corners.
