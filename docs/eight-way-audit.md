# Eight-way movement audit: code that still assumes four ways

Eight-way movement (#316) made `motion::cornerOpen` the one corner rule: a diagonal passes when
at least one of the two cells beside it can be stood on. This page lists every place found
(on `main` at 215b1c6) that still loops over the four sides, or otherwise reasons in four ways,
and says whether that is **intended** (four ways by design, or not about movement) or a
**leftover** that now disagrees with movement. Nothing here is changed yet: the suggested
changes wait for the author's choice.

How it was found: a search for `dir::allDirections`, `dir::sides`, loops `< 4` and `% 4` over
directions, explicit side lists, `leftOf`/`rightOf`/`behind`, `getSteppableNeighborhood`,
`isSteppableDirection`, then reading every movement, sight, shooting, placement and pathing
path: puppet masters (patrol, collector, hunter, wall follower, guardian, Hound), `roam`,
teleporters, the autopilot, rubbish, explosions, kiki/bouba beams, missiles and bunkers.

"Mode 4" means the movement switch of #319 (`--movement 4`), where `cornerOpen` refuses every
diagonal.

## Leftovers that disagree with movement

| file:line | what | verdict | suggested change |
|---|---|---|---|
| `src/plainMissile.cpp:85` | A missile whose next cell (diagonal) holds something killable hurts it without checking the corner. The step branch above it goes through `motion::step` and does stop at a closed corner. | **Leftover, a real bug.** Checked with a scratch test: a missile flying up-left hurt a player behind two walls that touch at the corner (energy 105 → 5). | Return early (the missile dies on the corner) when `!motion::cornerOpen(board, here, dir)`, as `plainGun::use` already does for its point-blank shot. Add a regression test. |
| `src/bazookaMissile.cpp:42` | Same for the bazooka missile: `be->hurt(...)` on the next cell with no corner check. | **Leftover, same bug.** | Same corner check before the hurt. The explosion after it is a 4-connected flood, so that part is fine. |
| `src/player.cpp:186` (control type 2, interact) | Interacting with the cell in the pressed direction (opening a door, waking an avatar, picking up an item) has no corner check. With two keys held, the player can open a door or pick up an item diagonally across a corner closed on both sides. | **Leftover.** It never reaches into a locked room: the room's wall ring is one cell thick, so nothing outside is diagonally next to anything inside. But it does let you reach around closed corners in the open maze. Agents have no diagonal interact action. | Refuse when `!motion::cornerOpen(board, here, dir)`, as walking does. |
| `src/bElem.cpp:165` | A dying element's inventory becomes a rubbish pile on its cell, or on one of the four sides. If all four are blocked, the pile and **everything in it is thrown away**. | Leftover (it predates #316, but a free diagonal cell now counts as "next to"). | After the four sides, try the four diagonals that `cornerOpen` allows. Sides first keeps today's behaviour wherever it works now. |
| `src/bElem.cpp:77` | Dropping an item: the four sides, else the item goes back into the inventory. | Leftover, harmless (the item is kept). | Optional: the same diagonal fallback as above. |
| `src/puppetMasterFR.cpp:77` (`towards`) | Picks a diagonal facing when the way is closer to a diagonal. Used for bites, turning to the prey and the guardian's aim. | Intended in mode 8. **Leftover in mode 4**: the body turns diagonally, then the shot or step is refused. | When `motion::currentMode()` is 4, fall back to the dominant axis. This only matters for mode 4. |
| `src/puppetMasterGuardian.cpp:56` | `inLine` counts diagonals as lines the gun can shoot along. | Intended in mode 8. Leftover in mode 4: the guardian aims diagonally and its shot is refused (the gun still recharges, so it loses a shot). | In mode 4, `inLine` = same row or column only. |
| `src/bunker.cpp:96` (`findLongestShot`) | The starting facing picks the longest of the four side lines. | Leftover, harmless: the bunker looks along all eight lines in `mechanics()` and turns when it sees the player. It only decides where the bunker looks before that. | Loop over `dir::ways` with `cornerOpen`, as `mechanics()` does. |
| `src/teleport.cpp:248` | Arrival cell: the teleporter's own cell or one of its four sides (`allDirections` includes `NODIRECTION`). | Mostly intended (you arrive next to the pad by a side). The pad's own cell being a choice is an old quirk. | Keep the four sides. Diagonals would only add arrival cells that can be reached solely by a diagonal, which `goe-audit` shows never matter for locks. |

## Intended four-way code

| file:line | what | why it is fine |
|---|---|---|
| `src/roam.cpp:69` (`followWall`) | The wall rule walks the four sides; a body coming off a diagonal keeps going across. | User decision of 2026-10-13: wall followers keep to the four sides. A diagonal wall rule would hug corners in a different way and cut through rooms. |
| `src/puppetMasterWallFollower.cpp:27`, `puppetMasterHunter.cpp:31`, `puppetMasterHound.cpp:35,48`, `puppetMasterFR.cpp:150` (patrol), `monster.cpp:160` | Idle patrols use `followWall`. | As above. Chasing is 8-way (`followTrail` → `pathTowards` over `dir::ways` with `cornerOpen`). |
| `src/puppetMasterCollector.cpp:39-48` | Looks all eight ways, turns a quarter at a time unless the target is ahead. | Already 8-way; turning by quarters is the original collector's character. |
| `src/explosives.cpp:248-270` (`traverser`) | The blast spreads as a 4-connected flood within its radius. | A 4-connected flood crosses exactly the corners `cornerOpen` lets through (one side free), and never a corner closed on both sides. So blasts already agree with movement. |
| `src/explosives.cpp:56` | Points for the shooter count the four side neighbours. | Scoring only, unrelated to movement (the blast itself reaches further). The copy at `:132` is inside a commented-out block. |
| `src/kiki.cpp:130`, `src/kiki.cpp:37` | Kiki beams run along the four sides; the bouba fence fills a straight line. | A beam is a straight fence across a corridor. Crossing a one-cell line needs a step onto it in both modes, so diagonals cannot slip past. |
| `src/puppetMasterHound.cpp:89` | The Hound appears on one of the four sides of the square ring `spawnDistance` away. | Placement only: it is the whole square ring (all cells at Chebyshev distance `spawnDistance`). |
| `src/lineOfSight.cpp:45` (`goe::sight::clear`) | Bresenham with its own corner rule: a diagonal step is blind only between two opaque cells. | Already 8-way and the same idea as `cornerOpen`. It uses "opaque" (walls, bricks, bunkers, teleporters, closed doors), not "steppable", so sight passes between two keys where a body would not. Sight is not movement; fine. |
| `src/monster.cpp:64,142`, `src/bunker.cpp:59`, `src/plainGun.cpp:75`, `src/autopilot.cpp:14`, `src/puppetMasterFR.cpp:91,173` | Monster bites, monster vision, bunker eyes, point-blank shots, autopilot, chasers' path search and `touching`. | Already 8-way through `cornerOpen`, so mode 4 restricts them too. |
| `src/chamberArea.cpp:152-170` (`checkIfElementIsFree`) | Reasons about the 8-cell neighbourhood (sides and corners, doors counted as open) so a placed item does not cut a corridor. | Written for 4-way corridors, but under the corner rule 8-way connectivity equals 4-way connectivity, so it is still right. **It is also dead code**: `calculateSurface` is never called from outside itself. Candidate for removal. |
| `src/monster.cpp:164` (`steppableNeigh`) | All 8 neighbours steppable. | Dead code (never called). Candidate for removal. |
| `src/isoFieldView.cpp:197` | Wall segments drawn on the four sides. | Rendering. |
| `src/spriteFrame.cpp:60` | 2D sprites face `dir::across(facing)`. | Rendering: sprites only have four facings. |
| `src/inputManager.cpp:90` | A fresh side press waits 40 ms for a second key. | Input, by design (#316). Skipped in mode 4. |
| `agent/src/agentGame.cpp` | Diagonal actions come after the side actions, before `GIVE_UP`. | By design: the old action numbers stay. |

## Summary

- **Two real leftovers that change gameplay in mode 8:** missiles (plain and bazooka) hurt a
  target diagonally behind a corner closed on both sides. They are small and local, with a
  regression test.
- **Interacting has no corner check** (`player.cpp:186`): doors and items can be reached
  diagonally around a closed corner. This never reaches into a locked room.
- **One item-loss edge case** (rubbish pile with all four sides blocked), which predates #316 but
  is easy to soften with diagonals.
- **Two mode-4-only leftovers** (`towards`, guardian `inLine`): they make mode 4 slightly weaker
  for chasers and guardians than the pre-#316 game, so they matter for the A/B test.
- Everything else is either already 8-way through `cornerOpen`, four-way by design (wall
  following, beams, blasts), not about movement, or dead code.
