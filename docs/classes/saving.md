# Saving, loading and chunk swap: `gameSerializer`

`gameSerializer` (`gameSerializer.h`) writes the whole game world to one binary file and reads it
back, and moves far chunks of the endless world to disk and back. The sequence diagram is
[09-save-load-sequence](../diagrams/09-save-load-sequence.svg).

## What a save holds

The format version (`formatVersion`, 7; older saves still load), the world seed (from 6), the
best score of avatars already lost (from 7), every board in `chamber::allChambers` (each chunk
with its cells and fog of war, the board's name, colour and live elements), every element
reachable from them (stacked, collected, held weapons...) with its stats and inventory, the game
clock, the instance id counter, the random generator's state, and the static registries (active
and visited players, golden apples, teleporters, view points, elements being disposed). Chunks on
disk are carried too.

Elements reference each other by instance id, and ids are kept. Plain floor and wall tiles, most
of the cells, are written compactly (type, subtype, facing, direction) and get fresh ids on load.

## Members

| Member | What it does |
|---|---|
| `saveGame(file)` | Writes to a temporary file, then renames it over the save. |
| `loadGame(file)` | Reads everything first; only a good file replaces the running world. |
| `canLoad(file)` | Whether the file looks like a save this build can read (magic and version), without loading it; the title screen offers Continue on it. |
| `replaceSave(file)` | A new game takes the old save's place at once, so Continue never brings the old game back; removes the old save when the new one cannot be written. |
| `removeSave(file)` | Deletes the save of a lost game. |
| `clearWorld()` | Empties the world (boards, players, apples, teleporters...) for a load or a new game. |
| `swapOutChunk(world, chunk)` | Writes a far chunk and what its elements carry to `chunk_x_y.bin` in the board's swap folder and drops it from memory, parking its apples and teleporters. Refuses a chunk holding an avatar, an element still being disposed, or the view's owner. |
| `swapInChunk(world, chunk)` | Reads a swapped chunk back, as it was left, and re-links references by id. |
| `createByType(type, subtype, board)` | A new element of a type number; the level generator, chunk patterns and loading share it. Throws for an unknown type. |

Every call runs on the game thread between ticks (`swapInChunk` may also run inside one) and takes
`chamber::worldMutex` itself.
