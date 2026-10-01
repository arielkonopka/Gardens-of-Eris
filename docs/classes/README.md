# Classes of Gardens of Eris

What every class of the game does, grouped by module. The diagrams that go with these pages are in
[docs/diagrams](../diagrams/README.md). When you add or change a class, change its page too.

| Page | What it covers |
|---|---|
| [Agent interface (C++)](agent-interface.md) | `goe::agent::game`, `config`, `state`, `action`, `section`, `event`, `feature`: the game without a window, for programs to play |
| [Python bindings](python-package.md) | the `goe` package: `_goe` (nanobind), `Game`, `GameState`, `GoeEnv`, `ChunkPattern` |
| [Basics](basics.md) | `bElemTypes`, `GoEConstants`, `coords`, `myUtility::Coords`, `dir::direction`, `controlItem` |
| [Elements: the base class](elements-core.md) | `bElem`, `bElemAttr`, `bElemStats`, `inventory`, `elementFactory`, `motion`, `elementView`, `goe::sound` |
| [Elements: the kinds](elements.md) | floor, wall, door, key, teleporter, guns, missiles, apples, bombs, landmines, player, monster, drone, camera, bunker, kiki and bouba |
| [Controllers](puppet-masters.md) | `puppetMasterFR` and its kinds, `goe::sight` |
| [The world](world.md) | `chamber`, `chamber::fence`, `worldBuilder`, `randomLevelGenerator`, `chamberArea`, `goe::chunkPattern` |
| [Rules](rules.md) | `difficulty`, `goe::music::byDifficulty`, `goe::music::cues`, `goe::rng`, `gameClock`, `goe::events`, `randomWordGen` |
| [Saving](saving.md) | `gameSerializer`: saves, loads and chunk swap |
| [The application](application.md) | `main.cpp`, `presenter`, `titleScreen`, `titleMenu`, `menuPad`, `bindings`, `inputManager`, `gameSettings`, `autopilot`, `hallOfFame`, `storyScroller`, `crashLog` |
| [Seeing and hearing](media.md) | `viewPoint`, `fogLayer`, `fogMask`, Allegro handles, `videoManager`, `videoDriver`, `configManager`, `soundManager`, `soundSpace`, `performerStream`, the adaptive musician (`goe::musician`) |

## Where each class lives

| Class or namespace | Header | Page |
|---|---|---|
| `allegroHandles` (`goe::bitmapHandle`...) | `include/allegroHandles.h` | [media](media.md) |
| `bazooka` | `include/bazooka.h` | [elements](elements.md) |
| `bazookaMissile` | `include/bazookaMissile.h` | [elements](elements.md) |
| `bElem` | `include/bElem.h` | [elements-core](elements-core.md) |
| `bElemAttr` | `include/bElemAttr.h` | [elements-core](elements-core.md) |
| `bElemStats` | `include/bElemStats.h` | [elements-core](elements-core.md) |
| `bouba` | `include/bouba.h` | [elements](elements.md) |
| `brickCluster` | `include/brickCluster.h` | [elements](elements.md) |
| `bunker` | `include/bunker.h` | [elements](elements.md) |
| `chamber` | `include/chamber.h` | [world](world.md) |
| `chamberArea` | `include/chamberArea.h` | [world](world.md) |
| `configManager` | `include/configManager.h` | [media](media.md) |
| `coords`, `dir`, `controlItem`, `bElemTypes`, `GoEConstants` | `include/commons.h` | [basics](basics.md) |
| `difficulty` | `include/difficulty.h` | [rules](rules.md) |
| `door` | `include/door.h` | [elements](elements.md) |
| `elementFactory` | `include/elementFactory.h` | [elements-core](elements-core.md) |
| `elementView` | `include/elementView.h` | [elements-core](elements-core.md) |
| `explosives` | `include/explosives.h` | [elements](elements.md) |
| `floorElement` | `include/floorElement.h` | [elements](elements.md) |
| `fogLayer` | `include/fogLayer.h` | [media](media.md) |
| `fogMask` | `include/fogMask.h` | [media](media.md) |
| `gameClock` | `include/gameClock.h` | [rules](rules.md) |
| `gameSerializer` | `include/gameSerializer.h` | [saving](saving.md) |
| `gameSettings` | `include/gameSettings.h` | [application](application.md) |
| `goe::agent::*` | `agent/include/agentGame.h`, `agentFeatures.h` | [agent-interface](agent-interface.md) |
| `goe::autopilot` | `include/autopilot.h` | [application](application.md) |
| `goe::chunkPattern` | `include/chunkPattern.h` | [world](world.md) |
| `goe::controls::bindings` | `include/controlBindings.h` | [application](application.md) |
| `goe::controls::menuPad` | `include/menuPad.h` | [application](application.md) |
| `goe::crashLog` | `include/crashLog.h` | [application](application.md) |
| `goe::events` | `include/gameEvents.h` | [rules](rules.md) |
| `goe::hallOfFame` | `include/hallOfFame.h` | [application](application.md) |
| `goe::music::byDifficulty` | `include/difficultyMusic.h` | [rules](rules.md) |
| `goe::music::cues` | `include/musicCues.h` | [rules](rules.md) |
| `goe::musician::adaptiveMusician` | `include/adaptiveMusician.h` | [media](media.md), [design](../adaptive-musician.md) |
| `goe::musician::composer`, `motif`, `phraseBuffer`, `phraseReport` | `include/musicComposer.h` | [media](media.md) |
| `goe::musician::noteEvent`, `eventQueue`, `randomStream` | `include/musicEvents.h` | [media](media.md) |
| `goe::musician::performerPersonality` | `include/musicPersonality.h` | [media](media.md) |
| `goe::musician::synthesizer`, `voice`, `oscillator`, `envelope`, `lowPass` | `include/musicSynth.h` | [media](media.md) |
| `goe::musician::tensionController` | `include/musicTension.h` | [media](media.md) |
| `goe::musician::tuning` | `include/musicianTuning.h` | [media](media.md) |
| `goe::musician::vocabulary` | `include/musicVocabulary.h` | [media](media.md) |
| `goe::rng` | `include/randomStreams.h` | [rules](rules.md) |
| `goe::sight` | `include/lineOfSight.h` | [puppet-masters](puppet-masters.md) |
| `goe::sound` | `include/elementSound.h` | [elements-core](elements-core.md) |
| `goe::storyScroller` | `include/storyScroller.h` | [application](application.md) |
| `goe` (Python) | `agent/python/goe/` | [python-package](python-package.md) |
| `goldenApple` | `include/goldenApple.h` | [elements](elements.md) |
| `inputManager` | `include/inputManager.h` | [application](application.md) |
| `inventory` | `include/inventory.h` | [elements-core](elements-core.md) |
| `key` | `include/key.h` | [elements](elements.md) |
| `kiki` | `include/kiki.h` | [elements](elements.md) |
| `landmine` | `include/landmine.h` | [elements](elements.md) |
| `monster` | `include/monster.h` | [elements](elements.md) |
| `motion` | `include/motion.h` | [elements-core](elements-core.md) |
| `myUtility::Coords` | `include/Coords.h` | [basics](basics.md) |
| `patrollingDrone` | `include/patrollingDrone.h` | [elements](elements.md) |
| `performerStream` | `include/performerStream.h` | [media](media.md) |
| `plainGun` | `include/plainGun.h` | [elements](elements.md) |
| `plainMissile` | `include/plainMissile.h` | [elements](elements.md) |
| `player` | `include/player.h` | [elements](elements.md) |
| `presenter::presenter` | `include/presenter.h` | [application](application.md) |
| `puppetMasterCollector` | `include/puppetMasterCollector.h` | [puppet-masters](puppet-masters.md) |
| `puppetMasterGuardian` | `include/puppetMasterGuardian.h` | [puppet-masters](puppet-masters.md) |
| `puppetMasterHound` | `include/puppetMasterHound.h` | [puppet-masters](puppet-masters.md) |
| `puppetMasterHunter` | `include/puppetMasterHunter.h` | [puppet-masters](puppet-masters.md) |
| `puppetMasterWallFollower` | `include/puppetMasterWallFollower.h` | [puppet-masters](puppet-masters.md) |
| `puppetMasterFR` | `include/puppetMasterFR.h` | [puppet-masters](puppet-masters.md) |
| `randomLevelGenerator` | `include/randomLevelGenerator.h` | [world](world.md) |
| `randomWordGen` | `include/randomWordGen.h` | [rules](rules.md) |
| `rubbish` | `include/rubbish.h` | [elements](elements.md) |
| `securityCamera` | `include/securityCamera.h` | [elements](elements.md) |
| `simpleBomb` | `include/simpleBomb.h` | [elements](elements.md) |
| `soundManager` | `include/soundManager.h` | [media](media.md) |
| `soundSpace` | `include/soundSpace.h` | [media](media.md) |
| `teleport` | `include/teleport.h` | [elements](elements.md) |
| `titleMenu` | `include/titleMenu.h` | [application](application.md) |
| `titleScreen` | `include/titleScreen.h` | [application](application.md) |
| `videoDriver` | `include/videoDriver.h` | [media](media.md) |
| `videoElement::videoElementDef` | `include/videoElementDef.h` | [media](media.md) |
| `videoManager` | `include/videoManager.h` | [media](media.md) |
| `viewPoint`, `vpPoint` | `include/viewPoint.h` | [media](media.md) |
| `wall` | `include/wall.h` | [elements](elements.md) |
| `worldBuilder` | `include/worldBuilder.h` | [world](world.md) |

`include/elements.h` includes every element header at once; `include/objectTypes.h` is empty.
