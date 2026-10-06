# Elements: the base class and what every element has

Everything on the board is a `bElem`: the floor, walls, the player, a missile in flight, a key in
an inventory. A cell holds a stack of elements (a floor, maybe an item on it, maybe a creature on
top). The kinds of elements are in [elements.md](elements.md), the controllers that drive drones
in [puppet-masters.md](puppet-masters.md).

## `bElem` (`bElem.h`)

The base class of every element. Elements are always owned by `std::shared_ptr`
(`enable_shared_from_this`) and made by `elementFactory::generateAnElement<T>` or
`gameSerializer::createByType`. Not copyable.

### What it owns

- `bElemAttr` (`getAttrs()`): what the element is (subtype, qualities, energy, ammo, inventory).
- `bElemStats` (`getStats()`): what the element is doing (position, timers, points, stack links).
- A weak link to its board (`getBoard()`, `setBoard()`).

### The live elements and the tick

| Member | What it does |
|---|---|
| `static void runLiveElements()` | One game tick: advances `gameClock`, disposes elements that finished dying, being destroyed or teleporting, drops deregistered elements, then runs `mechanics()` of every live element near the active player (`chamber::isActiveNear`, the 5 x 5 chunks around the player) and finally the player's own. The Hound watches for campers here too. |
| `registerLiveElement(who)`, `deregisterLiveElement(id)` | Adds an element to, or queues it out of, its board's `liveElems`: the elements whose `mechanics()` runs each tick. |
| `virtual bool mechanics()` | What the element does by itself each tick. The base version returns false while the element is busy or gone, so subclasses call it first and stop when it says no. |
| `static void tick()`, `static unsigned getCntr()` | `gameClock::advance()` and `gameClock::now()`. |

### Moving and meeting other elements

| Member | What it does |
|---|---|
| `stepOnElement(step)` | Puts this element on top of `step`'s cell (through `chamber::place`). Refuses a collector onto a collectible it could pick up, so nothing is ever placed on one. |
| `removeElement()` | Takes the element out of its cell (`chamber::lift`) and returns it, for collecting or moving. |
| `moveInDirection(d)`, `moveInDirectionSpeed(d, speed)` | One cell in `d`, through `motion::step`: collect, step, push or interact, in that order. |
| `dragInDirection(d)` | One cell in `d`, pulling the movable element behind along (`motion::drag`). |
| `selfAlign()` | Turns the element to a sensible facing after it is placed. |
| `isSteppableDirection(d)`, `getElementInDirection(d)` | The neighbour in a direction (or offset); nullptr where there is no cell. |
| `getSteppableNeighborhood()` | The eight neighbours as `sNeighboorhood`. |
| `stepOnAction(step, who)` | Called on the element being stepped on (`step` true) or left (`false`). Landmines, missiles, teleporters and boubas react here. |

### Items, interaction, damage

| Member | What it does |
|---|---|
| `collect(item)` | Lifts the item into this element's inventory; reports `goe::events::kind::collect`. |
| `dropItem(instanceId)` | Puts an item from the inventory back on the board. |
| `collectOnAction(collected, who)` | Called on the item when it is collected or dropped. |
| `collectibleBy(who)` | False when this collectible must not end up with `who` (a loose controller and a drone); `collect` and `motion` then treat it as an obstacle. |
| `use(item)` | Uses an item (a gun shoots). |
| `interact(who)` | What happens when `who` presses interact against this element (doors open, teleporters send, drones take a controller). The base version sets the interaction timer. |
| `hurt(points)` | Takes energy; at 0 the element is killed. |
| `kill()` | Starts dying (`killed` timer); queues the element in `toDispose`. |
| `destroy()` | Starts being destroyed (`destroyed` timer), as in an explosion; queues it too. |
| `disposeElement()` | Takes the element off the board for good, deregisters it, stops its sounds and leaves what it carried in a rubbish pile when `dropsInventoryOnDeath()` (otherwise the inventory vanishes). Returns `oState`. |
| `dropsInventoryOnDeath()` | True by default; a rubbish pile says false so it never leaves another pile. |

### Kinds and set-up

| Member | What it does |
|---|---|
| `getType()` | The kind, a `bElemTypes` value. Every subclass overrides it. |
| `getAnimPh()` | The animation phase drawn now. |
| `getViewRadius()` | How far the element sees, for the fog and for chasers. |
| `additionalProvisioning(subtype)` | Called once after construction (by `elementFactory`): reads the element's default attributes for its type and subtype from `skins.json` and lets a subclass set up timers, guns or registration. Runs only once (`std::once_flag`). |
| `setStatsOwner(owner)` | Whose points this element earns (a missile scores for its shooter). |

## `bElemAttr` (`bElemAttr.h`)

What an element is, read from the `attributes` of its sprite in `skins.json` and changed by
the game.

- Numbers: `subtype`, `energy`, `maxEnergy`, `ammo`, `maxAmmo`.
- Qualities, each with a getter and setter: killable, destroyable, steppable, movable,
  interactive, collectible, can push, can be pushed, can collect, weapon, open, locked; `isMod()`.
- `getInventory()` / `setInventory()`: the element's `inventory`, when it can collect.
- `isSteppable()` is false while the owner is dying, being destroyed or teleporting; it reads the
  owner's stats directly so that this hot check needs no `weak_ptr` lock.

## `bElemStats` (`bElemStats.h`)

What an element is doing, and the links that make up a cell's stack.

- Identity: `getInstanceId()`, a process-wide counter kept across save and load.
- Place: `getMyPosition()` (a collected element is where its collector is), `getMyDirection()`,
  `getFacing()`.
- Timers, all measured on `gameClock`: moved, waiting, killed, destroyed, teleporting
  (`telInProgress`), interacted, fading in and out. Each getter returns the ticks left, or -1.
  `isMoving()`, `isWaiting()`, `isDying()`, `isDestroying()`, `isTeleporting()`, `isFadingIn()`,
  `isFadingOut()`, `isInteracting()`, and `busy()` when any of them runs (the element cannot act).
- Flags: active, collected, disposed, marked, activated mechanics, has a parent (stands on
  something).
- Stack: `getSteppingOn()` (what it stands on) and `getStandingOn()` (what stands on it). Only
  `chamber::place` and `chamber::lift` change them (`friend class chamber`).
- Points: `getPoints(pointsType)` / `setPoints`, and `getStats(pointsType)` / `setStats`.
- `getCollector()`, `getStatsOwner()`: who holds the element, whose points it earns.

## `inventory` (`inventory.h`)

What a collector carries, in five vectors: `weapons`, `usables`, `keys`, `mods`, `tokens`.

| Member | What it does |
|---|---|
| `addToInventory(what)` | Puts an item in its section by its qualities; false when it cannot be carried. Token counts are kept per `tType` (type and subtype, subtype -1 fits all). |
| `getActiveWeapon()`, `nextGun()`, `removeActiveWeapon()`, `countActiveWeaponKind()` | The gun in hand; `nextGun` jumps to the next kind (type and subtype); an empty gun is removed and the next one of the same kind, else any, takes over. |
| `getUsable()`, `nextUsable()` | The usable in hand. |
| `getKey(type, subtype, removeIt)` | A key for a door. |
| `countTokens`, `requestTokens`, `requestToken` | Count, spend or take tokens. |
| `mergeInventory(other)` | Takes everything from another inventory (a rubbish pile emptied into its finder). |
| `changeOwner(who)`, `updateBoard()` | Moves the items to a new owner or board. |
| `retrieveCollectibleFromInventory`, `removeCollectibleFromInventory`, `findInInventory`, `removeToken` | By instance id or position. |
| `clear()`, `isEmpty()` | Let go of everything; nothing held. |
| `runLives()` | Runs the mechanics of items that act while carried (a broken golden apple draining). |

## `elementFactory` (`elementFactory.h`)

`elementFactory::generateAnElement<T>(board, subtype)` makes an element of class `T`, sets its
board and calls `additionalProvisioning(subtype)`. The level generator and most code that makes
elements use it; `gameSerializer::createByType` picks the class from a type number.

## `motion` (namespace, `motion.h`)

How elements move. An element that moves into a cell tries, in order: collect what is there (and
then step onto what it uncovered, in the same move), step onto it, push it one cell further,
interact with it.

- `step(who, d, speed)`: one cell in `d`, the move taking `speed` ticks.
- `drag(who, d, speed)`: one cell in `d`, pulling the movable element behind along.
- `collectAndStep(who, collectible, speed)`: collect, then step if the collect worked.

## `elementView` (namespace, `elementView.h`)

`offset(elem, tileSize)`: how far, in pixels, an element moving between two cells is drawn from
the cell it moved to. The game model knows nothing about pixels; the presenter asks this.

## `goe::sound` (namespace, `elementSound.h`)

`play(elem, eventType, event)` plays an element's sound for an event from `skins.json`, heard
where the element is (a collected one where its collector is). `observe(fn)` lets tests see every
call, since they have no sound device.
