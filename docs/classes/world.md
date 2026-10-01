# The world: board, chunks, generation

The game plays on one endless board made of 64 x 64 chunks, built around the player as they walk
and swapped to disk when far away. The diagrams are
[02-world-classes](../diagrams/02-world-classes.svg) and
[07-chunk-generation-activity](../diagrams/07-chunk-generation-activity.svg).

## `chamber` (`chamber.h`)

A board of cells. The game uses one endless board (`makeWorld`); a bounded board
(`makeNewChamber(size)`) has a fixed size and all its cells from the start, for tests, the
benchmark and saves from before the endless world. Either way cells are stored in chunks of
`chunkSize` (64) x 64, so coordinates may be negative. A cell outside a bounded board, or in a
chunk not made yet, holds nothing (`getElement` returns nullptr) and nothing can step there. The
board belongs to the game thread.

### Cells and stacks

| Member | What it does |
|---|---|
| `getElement(cell)`, `topAt(cell)` | The top element of a cell's stack; `topAt` returns a reference without copying the pointer, valid until the cell changes. |
| `setElement(cell, elem)` | Sets the top of a cell. |
| `static place(elem, onto)` | Puts `elem` directly on top of `onto` (sliding in under anything above). |
| `static lift(elem)` | Takes `elem` out of its stack; an emptied cell gets a fresh floor. |
| `visitPosition`, `isVisible`, `setVisible` | The fog of war per cell (0 once seen). |
| `calculateLine(position, d)`, `getLastInLine(pos, d)` | Walks a line of cells (missiles, bunkers). |

Only `place` and `lift` change the stack links of `bElemStats`; neither checks game rules, which
`bElem::stepOnElement` and `bElem::removeElement` do.

### Chunks

| Member | What it does |
|---|---|
| `chunkSize = 64`, `activeChunks = 2` | Chunk side; elements up to 2 chunks from the player run. |
| `static chunkOf(cell)`, `static chunkOrigin(chunk)` | Cell to chunk by floor division; a chunk's first cell. |
| `hasChunk(chunk)`, `addChunk(chunk)` | Whether a chunk exists; make one covered with floor. |
| `chunkKeys()` | The chunks in memory now. |
| `isSwapped(chunk)`, `swappedCount()` | Chunks written to disk by `gameSerializer::swapOutChunk`. |
| `isActiveNear(cell, player)` | Whether an element there runs (always true on a bounded board). |
| `isBounded()` | False for the endless world. |
| `origin` | Where the player started; distances for the difficulty, and the agent's coordinates, count from here. |

### `chamber::fence`

While a fence lives, the board has no cells outside `lo..hi`. A chunk is built inside one, so
nothing placed while building it (a kiki's beam, say) reaches into the next chunks, and a chunk
comes out the same whichever chunks were built before.

### The world registry

`allChambers` owns every board (normally just the endless world); `worldMutex` guards it and is
held while a chunk is generated or the game is saved or loaded. `liveElems` and `toDeregister`
are the board's live elements (see `bElem::runLiveElements`).

## `worldBuilder` (namespace, `worldBuilder.h`)

Grows and shrinks the endless world around the player, on the game thread.

| Function | What it does |
|---|---|
| `buildRadius = 2`, `keepRadius = 3` | Chunks within 2 of the player exist (5 x 5); chunks beyond 3 go to disk (7 x 7 stay). |
| `startNew()` | A new world: the start chunk (the player and `origin`) and every chunk within `buildRadius`. |
| `growAround(world, cell)` | Builds, or reads back from disk, the nearest missing chunk within `buildRadius`; one per call. False when none is missing. |
| `shrinkAround(world, cell)` | Puts the furthest chunk beyond `keepRadius` on disk; one per call. |
| `bringIn(world, cell)` | Reads the cell's chunk back from disk if it went there. |
| `movePlayerTo(world, chunk)` | Builds that far chunk and its neighbours and moves the active player onto free floor near its middle (the title screen demo). |
| `chunksGenerated()` | How many chunks were made new so far; only grows (the story scroller watches it). |
| `setPattern(chunk, pattern)`, `setDefaultPattern(pattern)`, `clearPatterns()`, `patternFor(chunk)` | Fixed patterns for chunks made from now on (see below); they stay until changed, across new games. |

## `randomLevelGenerator` (`randomLevelGenerator.h`)

Builds a maze and fills it with elements: one chunk of the endless world, or a bounded level
(tests and the benchmark).

| Member | What it does |
|---|---|
| `randomLevelGenerator(world, chunk)` | For one chunk of the endless world, seeded from the world seed and the chunk's place (`goe::rng::placeSeed`). |
| `randomLevelGenerator(w, h, levelSeed)` | For a bounded level. |
| `generateChunk(start, pattern)` | Fills the chunk: with a pattern, the pattern's elements only (`fillChunk`); otherwise its west and north walls with their gaps (`buildChunkWalls`), the recursive-division maze (`buildMaze`, `5 - depth` holes per wall) and everything placed in it (`placeEverything`: monsters, bunkers, cameras, keys, guns, apples, landmines, teleporters, doors, and in about every fifth chunk a global teleporter room). The start chunk also gets the player and the world's origin. |
| `generateLevel(holes)` | Fills a bounded level. |
| `static wallGaps(chunk, west)` | Where a chunk's west or north wall is open, from the world seed and the wall's place only, so both neighbours agree whichever is built first. |
| `eng` | The level's own random engine; `gameplay()` draws from it while the chunk is built (`goe::rng::generationScope`). |

Helper types: `elementToPlace` (type, subtype, count, how to place it, surface needed) and
`rectangle`.

## `chamberArea` (`chamberArea.h`)

A rectangle of the maze, split recursively into child areas while the maze is generated: the
spanning tree the README's "Random element placement" describes. Each area owns its children and
knows its free surface; the placer finds areas with enough room
(`findChambersCloseToSurface`), lists their free cells (`findElementsToStepOn`) and removes an
area once it is filled (`removeArea`, which recalculates the surfaces above it).

## `goe::chunkPattern` (`chunkPattern.h`)

A fixed layout of elements for a chunk, built instead of a random maze. Agents use it to train on
a world they choose ([agent-interface.md](agent-interface.md#chunk-patterns)).

- `patternCell`: `{type, subtype}`; type -1 places nothing, type 0 gives the floor itself the
  subtype.
- `chunkPattern(width, height, cells)`: at most 64 on each side, cells row by row
  (`cells[y * width + x]`). Throws `std::invalid_argument` for a bad size or a type a pattern may
  not place (`placeable`: anything the game builds a world from, not missiles).
- `fromRows(rows, legend)`, `fromJson(text)`, `load(file)`: JSON is
  `{"legend": {"#": "wall", "k": ["key", 1], ".": null}, "rows": ["#k.#", ...]}`; a legend entry
  is null, a type (number or name) or `[type, subtype]`.
- `at(x, y)`: the cell at (x, y) of a chunk, the pattern repeated to fill it from the top left.
- `places(type)`, `typeNames()`, `typeByName(name)`.

A patterned chunk has no maze walls and no chunk walls; walls are where the pattern puts them. In
the start chunk the first player in the pattern (row by row) is the one the game starts with;
without one, the player goes on the free floor nearest the middle. Players in other chunks are
spare avatars. Patterned chunks are laid out the same whatever the seed.
