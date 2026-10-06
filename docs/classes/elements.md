# Elements: the kinds

Every kind is a subclass of `bElem` ([elements-core.md](elements-core.md)) and returns its own
`bElemTypes` number from `getType()` ([basics.md](basics.md#belemtypes-namespace-commonsh)). Its
default qualities (steppable, collectible, energy...) come from `skins.json` by type and subtype;
the class adds behaviour. The README's element table describes them from the player's side.

```
bElem
 ├─ floorElement, wall, rubbish, brickCluster, door, key, teleport, bunker
 ├─ plainGun ── bazooka
 ├─ plainMissile
 ├─ player, monster, patrollingDrone, securityCamera, kiki
 ├─ puppetMasterFR ── (see puppet-masters.md)
 └─ explosives (virtual base)
      ├─ simpleBomb ── landmine
      ├─ goldenApple
      ├─ bazookaMissile
      └─ bouba
```

## The maze

### `floorElement` (`floorElement.h`)
The floor every cell starts with; type 0. Its subtype is its look. A cell left empty by
`chamber::lift` gets a fresh floor.

### `wall` (`wall.h`)
A wall; type 4. Overrides `stepOnElement` and `removeElement` so walls stay put. Blocks sight
(`goe::sight::opaque`).

### `brickCluster` (`brickCluster.h`)
Type 8. Cannot be killed, can be pushed, dragged and blown up. The level generator also uses it to
block a passage instead of a door. Blocks sight.

### `door` (`door.h`)
Type 52. `interact` opens or closes an unlocked door; a locked one opens for a key of the door's
subtype from the interacting collector's inventory (`inventory::getKey`); doors of an even subtype
use the key up, doors of an odd subtype leave it. Opening reports `goe::events::kind::open`.
`stepOnAction` and `destroy` keep the door's look in line with its state. A closed door blocks
sight.

### `key` (`key.h`)
Type 51. A collectible; its subtype is the door colour it opens.

### `teleport` (`teleport.h`)
Type 400. Interacting with it, or standing in it, sends the element to the teleporter at the other
end, onto a steppable cell next to it.

- Every teleporter is in a registry (`allTeleporters`, guarded by `registryMutex`). Subtype 0
  teleporters are global and pair with any other global one; local ones pair within their subtype
  (one subtype per 5 x 5 chunk area). The pair is made at random on first use (`linkWith`); the
  chosen one becomes the receiver.
- `registrationBatch`: while one lives on a thread, new teleporters wait and join the registry
  together when it ends, so a teleporter never links into a half-built chunk.
- `park(elements)`, `unpark(elements)`, `parkedCount()`: when a chunk goes to disk its
  teleporters leave the registry but stay pickable by id and place; using a link to one reads
  that chunk back first (`partner()`).

### `rubbish` (`rubbish.h`)
Type 2. The pile a dead creature leaves with what it carried (`bElem::disposeElement`). On its
first tick it empties itself into a collector standing on it. Never leaves another pile
(`dropsInventoryOnDeath` is false).

## Items and weapons

### `plainGun` (`plainGun.h`)
Type 200. A weapon: `use(who)` fires a `plainMissile` from `createProjectible` in the holder's
facing. Starts with a random amount of ammo. The gun's energy halves with each shot and recovers
in `mechanics()`, so shooting fast makes weaker shots; the holder's dexterity sets a random cut on
the missile's energy. An empty gun is thrown away and the next gun takes over (see `inventory`).

### `bazooka` (`bazooka.h`)
Type 204. A `plainGun` whose `createProjectible` fires a `bazookaMissile`.

### `plainMissile` (`plainMissile.h`)
Type 201. Flies straight at `_plainMissileSpeed`, hurts what it hits by its energy and scores for
its shooter (`setStatsOwner`, `goe::events::blame`).

### `goldenApple` (`goldenApple.h`)
Type 900, an `explosives`. The thing the player is looking for, collected as a token. Shot or
hurt, it breaks: a broken apple is a usable that heals its holder and drains its own energy, and
explodes in the inventory when it runs out. The static list of apples still out in the world
(`getApple(num)`, `getAppleNumber()`) is guarded by a mutex; `park` and `unpark` take apples in
swapped-out chunks off the list and back while they still count.

## Explosives

### `explosives` (`explosives.h`)
The virtual base of everything that blows up. `explode(blastRadius)` spreads the blast cell by
cell (`traverser`, a cellular automaton) and hurts or destroys what it reaches, blamed on whoever
set it off.

### `simpleBomb` (`simpleBomb.h`)
Type 602. `hurt`, `kill` or `destroy` set it off; it explodes after `fuse()` ticks (15).

### `landmine` (`landmine.h`)
Type 603, a `simpleBomb` that looks almost like floor. Anything stepping on it sets it off
(`stepOnAction`) and it explodes on the next tick (`fuse()` 1), taking what stands on it along.
Deeper chunks get more of them (`difficulty::landmineCopies`).

### `bazookaMissile` (`bazookaMissile.h`)
Type 205. Flies up to `_bazookaMaxSteps` cells and explodes when it hits something or runs out.

### `bouba` (`bouba.h`)
Type 555. One cell of a kiki's death ray; every `_radioActivitySpeed` ticks it hurts what stands
in it by `difficulty::beamDamage`. The kiki clears its boubas when the beam is blocked.

## Creatures and contraptions

### `player` (`player.h`)
Type 100. The avatar. `mechanics()` reads the control item from `inputManager` and walks, shoots,
interacts, drags, cycles items, uses, drops or gives up (see `controlItem` in
[basics.md](basics.md#controlitem-struct-commonsh)). It moves the sound listener and the view.

- `getActivePlayer()`: the avatar being played; `nullptr` is game over.
- Unused avatars found in the maze are activated by interacting with them; when the active one
  dies the next activated one takes over (`visitedPlayers`, `countVisitedPlayers()`). Waking one
  reports `goe::events::kind::activate`.
- `bestScore()`: the best score of every avatar of this game, lost ones included (kept in saves).
- `backgroundScope`: while one lives on a thread, players made there never become the active
  player, so a spare avatar in a new chunk does not take over.
- `getViewRadius()` grows with the steps taken.

### `monster` (`monster.h`)
Type 6. Walks straight and turns when blocked (`checkNeigh`, `steppableNeigh`). Collects if its
skin allows (`canCollect` in `skins.json`); some have no inventory on purpose.

### `patrollingDrone` (`patrollingDrone.h`)
Type 7. Does nothing until it gets a controller. A collector interacting with a subtype 0 drone
hands over a puppet master from its inventory (the player's view then also follows the drone);
`attachController(controller)` does it directly. From then on `mechanics()` calls the
controller's `drive()` (see [puppet-masters.md](puppet-masters.md)). `getBrainModule()` returns
the controller.

### `securityCamera` (`securityCamera.h`)
Type 78. On its first tick spawns `difficulty::guardianCount` drones driven by
`puppetMasterGuardian`. When it sees the active player within `difficulty::cameraSight` with
nothing opaque between (`goe::sight`), it records where (`getAlertPosition`) and counts the
sighting (`getAlertNumber`), and the guardians go there. `leash` (55) is how far guardians may go.

### `bunker` (`bunker.h`)
Type 250. A fixed turret with its own gun. Looks along its four lines up to
`difficulty::bunkerRange` and fires at the player (`findLongestShot`), resting
`difficulty::bunkerRest` between shots. Hums while active.

### `kiki` (`kiki.h`)
Type 556. A death-ray emitter: lays a beam of `bouba` cells in its direction until something solid
stands in it (`beamBlocked`).
