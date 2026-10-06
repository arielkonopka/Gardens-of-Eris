# Rules: difficulty, randomness, time, events

The cross-cutting rules every element and the agent library rely on.

## `difficulty` (namespace, `difficulty.h`)

How hard the game is now, and every tuning that follows from it, in one place. The difficulty D
is the player level plus the distance level:

- `playerLevel(shots)`: floor(log5(shots + 1)), the HUD's Dex; computed on integers.
- `distanceLevel(from, to)`: floor(log2(1 + d / 64)) for the longer axis distance d.
- `of(player)`, `current()`: D for a player, or for the active one (0 when there is none).
- `areaOf(cell)`: the 64 x 64 area a cell is in (the Hound's camping check).
- `chunkDepth(chunk)`: a chunk's maze difficulty, 0 to 4, from its distance to the start chunk.

Tunings, built from 5 and 23 (the Law of Fives): `bunkerRange(d)`, `bunkerRest(base, d)`,
`cameraSight(d)`, `guardianCount(d)`, `beamDamage(d)`, `mazeHoles(depth)`, `landmineCopies(depth)`,
`houndPatience(d)`, `musicCrossfadeSeconds`, `musicHoldSeconds`, `musicDangerHoldSeconds` (10 s),
`musicianLevel(d)` (23 per step, clamped to the performer's 0..256), `musicAlertTicks` (10 s),
`musicDangerTicks` (5 s), `musicDangerDistance` (3 cells). Constants:
`distanceUnit` (64, one chunk), `ticksPerSecond` (50), `five`, `twentyThree`.

## `goe::music::byDifficulty` (`difficultyMusic.h`)

Which song plays for the current D and danger. `choose(d, threatened, songs, now, roll)` takes the
skin's songs as `songSlot`s (`level`, `danger`). Under threat a danger song starts at once and plays
for at least `musicDangerHoldSeconds`; otherwise the songs of `levelFor(d, songs)` (the highest
level not above D, or the lowest one) play, `roll` picking among them, and once a song plays D may
change it only after `musicHoldSeconds`, so walking back and forth over a distance step does not
flip the music. `mix(now)` is how far the crossfade into the new song has come (0 to 1 over
`musicCrossfadeSeconds`). The sound thread asks it every round.

## `goe::music::cues` (namespace, `musicCues.h`)

The game's side of the music-control interface. Cameras report `sighted()`, guardians `chased()`
(seeing, chasing or checking where the player was seen) and `endangered()` (within
`musicDangerDistance` and seeing the player, or fighting). `now()` gives the strongest situation
still fresh in game ticks: danger for `musicDangerTicks`, alert for `musicAlertTicks`, else calm.
`reset()` forgets every report (`gameSerializer::clearWorld` calls it). The presenter hands `now()`
to `soundManager::followSituation` every tick; the performer changes its theme with it.

## `goe::rng` (namespace, `randomStreams.h`)

The game's sources of randomness, kept apart so drawing from one never shifts another. Never add
a `std::mt19937` or `std::random_device` of your own in game code; pick a stream.

| Stream | For | Thread | Saved |
|---|---|---|---|
| `gameplay()` | anything that shapes the world | the game thread, or a thread building a chunk | the game's own engine is |
| `saved()` | the running game's engine | game thread | yes |
| `audio()` | music choices | any (locked) | no |
| `cosmetic()` | visual effects | game thread | no |

- `worldSeed()`, `setWorldSeed(s)`, `freshSeed()`: one world seed rebuilds the whole world
  (`--seed`, the agent's `newEpisode(seed)`).
- `placeSeed(x, y, salt)`: a seed for one chunk or wall from the world seed and the place only,
  so the order chunks are built in does not matter. `nextLevelSeed()` does the same for bounded
  levels.
- `generationScope`: while one lives, `gameplay()` on that thread draws from the given engine
  (the chunk's own).
- `below(e, n)`, `pick(e, items)`: a number below n, a random item.

## `gameClock` (`gameClock.h`)

The tick counter every timed state is measured on: `now()`, `advance()`. Atomic, because threads
building chunks read it. The game runs 50 ticks a second; the agent library resets it to 5 at
every new episode.

## `goe::events` (namespace, `gameEvents.h`)

What happens in the game that a watcher may want to count, such as an agent's reward. The game
itself does not listen; with no watcher, `report` does nothing.

- `kind`: `collect` (subject collected by actor), `use` (actor used subject, its usable),
  `open` (actor opened a door), `teleport` (actor sent through a teleporter), `kill` (actor's
  missile or blast killed or destroyed subject), `activate` (actor woke subject, a spare avatar,
  in `player::interact`).
- `observe(fn)`: one watcher at a time (`goe::agent::game` is one); an empty function clears it.
- `report(kind, subject, actor)`: called by the elements; the actor may be null.
- `blame(who)`: while one lives, kills are put down to `who` (the shooter of a missile or the one
  who set off a blast, also through a bomb the shot set off). Blames nest; `blamed()` says whom.
- `isDown(e)`: dying, being destroyed or gone.

## `randomWordGen` (`randomWordGen.h`)

`generateWord(length)`: a made-up word from a list of syllables; `chamber` names its boards with it.
