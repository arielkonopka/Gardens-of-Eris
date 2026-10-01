# Diagrams

PlantUML sources (`.puml`) with their rendered SVGs. After changing a source, render again from this folder:

```sh
plantuml -tsvg *.puml
```

Class by class descriptions are in [docs/classes](../classes/README.md).

## Structure

| Diagram | What it shows |
|---|---|
| [Elements](01-elements-classes.svg) ([source](01-elements-classes.puml)) | `bElem` and its kinds, attributes, stats, inventory, puppet masters |
| [World](02-world-classes.svg) ([source](02-world-classes.puml)) | `chamber` and its chunks, `worldBuilder`, `randomLevelGenerator`, `chunkPattern`, `gameSerializer`, difficulty, random streams, teleporters |
| [Application](03-application-classes.svg) ([source](03-application-classes.puml)) | title screen and menu, game controller in the menus (`menuPad`), settings, presenter, input, autopilot, hall of fame, sound, video, fog, stories |
| [Agent](04-agent-classes.svg) ([source](04-agent-classes.puml)) | the headless `goe::agent::game` and the Python package `goe` (`Game`, `GoeEnv`, `ChunkPattern`, `generate_chunk`) |
| [Components](14-components.svg) ([source](14-components.puml)) | the modules and how they depend on each other |

## Behaviour

| Diagram | What it shows |
|---|---|
| [Application lifecycle](05-app-lifecycle-activity.svg) ([source](05-app-lifecycle-activity.puml)) | start, title screen, demo, hall of fame, new game, continue, game over, exit |
| [Game tick](06-game-tick-activity.svg) ([source](06-game-tick-activity.puml)) | one tick of `presenter::presentEverything`, demo checks, saving on Esc, growing the world |
| [Chunk generation](07-chunk-generation-activity.svg) ([source](07-chunk-generation-activity.puml)) | building, swapping in and out, and generating chunks (maze or pattern) |
| [Player step](08-player-move-sequence.svg) ([source](08-player-move-sequence.puml)) | from a key press to moving, collecting, pushing, shooting |
| [Save and load](09-save-load-sequence.svg) ([source](09-save-load-sequence.puml)) | saving, loading and chunk swap files |
| [Agent step](10-agent-step-sequence.svg) ([source](10-agent-step-sequence.puml)) | one `makeAction` of the agent library, from Python down to the ticks |
| [Title screen states](11-title-menu-states.svg) ([source](11-title-menu-states.puml)) | menu screens, demo, hall of fame, game, game over, keys and pad alike |
| [Element lifecycle](12-element-lifecycle-states.svg) ([source](12-element-lifecycle-states.puml)) | created, on the board, moving, collected, dying, destroyed, swapped out, disposed |
| [Camera and guardian](13-guardian-states.svg) ([source](13-guardian-states.puml)) | how a guardian drone patrols, checks, fights and stays on its camera's leash |
