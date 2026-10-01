# Seeing and hearing: view, fog, video, sound, configuration

## `viewPoint` (`viewPoint.h`)

A singleton: what the screen follows and what can be seen. `setOwner(elem)` is the element the
view follows (the player, or a drone the player handed a controller to); `addViewPoint(elem)` adds
another element that uncovers the map around it (a patrol drone acts as an extra camera).
`getViewPoints(start, end)` returns the points (`vpPoint`: x, y, radius) in a region for the fog;
`isPointVisible`, `calculateObscured` answer for one cell.

## `fogLayer` (`fogLayer.h`)

Draws the game field under the fog. What nobody sees is covered by the `FogBitmap` of `skins.json`,
tiled over the world so it stays put while the view scrolls, or by `plainColour()` without one.
`setup(width, height, tileSize, patternFile)` needs the display; `draw(scene, points, worldOrigin,
...)` paints the mask and draws through the fog shader. GLSL ES compatible, for a phone port.

## `fogMask` (`fogMask.h`)

How much of the scene is seen, one value per `cellPx` (8) pixels, painted on the CPU
(`paint(points, centreShift)` returns whether it changed) and read by the shader with one filtered
lookup, instead of every pixel looping over every view point on the GPU.

## `goe` handles (`allegroHandles.h`)

Owning smart pointers for Allegro objects: `bitmapHandle`, `fontHandle`, `timerHandle`,
`eventQueueHandle`, `displayHandle`, `shaderHandle`, built on `destroyWith<Destroy>`. Pass
`handle.get()` to C functions; the handle keeps ownership. The project has no raw `new` or
`delete`.

## `videoManager` (`videoManager.h`)

A singleton owning the Allegro display: screen, game field and HUD sizes, the backbuffer, and
shaders by id (`setupShader`, `getShader`, `destroyShader`).

## `videoDriver` and `videoElement::videoElementDef` (`videoDriver.h`, `videoElementDef.h`)

`videoDriver::getVideoElement(type)` returns the sprite set of an element type, loaded from
`skins.json` once: the animation phases by subtype and direction (`defArray`) and the dying,
destroying, teleporting and fading animations.

## `configManager` (`configManager.h`)

A singleton that reads `skins.json` once and exposes it as `gameConfig`: font, sprite sheet, splash
screen, fog bitmap, tile size, the music list, the default animations, the sprites
(`spriteData`, with each subtype's `attributeData` that `bElemAttr` starts from), and the sound
samples by element type, subtype, event type and event (`sampleData`). `configReload()` reads it
again.

## `soundManager` (`soundManager.h`)

A singleton playing sound through OpenAL on its own thread.

- `registerSound(...)`: plays an element's sample for an event, placed where the element is
  relative to the listener (see `soundSpace`). Samples come from `skins.json`; `allowMulti`,
  `modeOfAction` (0 once, 1 looped) and `stacking` decide how repeats behave. Sources come from a
  pool used round-robin.
- `setListenerPosition`, `setListenerChamber`: the player moves the listener.
- `stopSoundsByElementId(id)`: when an element goes, its looping sounds stop.
- Music: `setupDifficultyMusic()` and `followDifficulty(d)` play the music list by D with a
  crossfade (`goe::music::byDifficulty`); `setupSong`, `pauseSong`, `resumeSong`, `moveSong`,
  `hasSong` handle songs placed on the board.
- Volumes come from `gameSettings`.
- The Performer: when `gameSettings` chooses it, the skins.json songs pause and `streamPerformer()`
  feeds a `performerStream` on the sound thread, outside `snd_mutex`, with `followDifficulty` (scaled
  by `difficulty::musicianLevel`) and `followSituation`. The stream is made the first time it is
  chosen and lives as long as the manager. The device's rate (`ALC_FREQUENCY`) is the musician's rate.

## `performerStream` (`performerStream.h`)

The integration point between the adaptive musician and OpenAL: one streaming source and four
buffers of 2048 stereo frames on `soundManager`'s context. `pump(level, situation, gain)` refills
every played buffer (`composeAhead`, then `renderAudio`), restarts the source after an underrun and
sets its gain to the music volume. `play(false)` lets the musician fade out, then stops the source.
Floats go to OpenAL when `AL_EXT_FLOAT32` is there, 16 bit otherwise.

## The adaptive musician (`goe::musician`, `adaptiveMusician.h` and `music*.h`)

Composes and synthesizes music at runtime; the full design is in
[docs/adaptive-musician.md](../adaptive-musician.md). None of these headers includes a game header.

- `adaptiveMusician`: the Music Director and the only class the game uses. `initialize(format,
  seed)`, `shutdown()`; from any thread `setDifficulty` (0..256), `setSituation`, `setEnabled`,
  `setVolume` (its own gain), `pause`, `resume`; on the audio thread `composeAhead()` and
  `renderAudio(frames)` (real-time safe). Inspectors for tests: `personality`, `tension`,
  `lastPhrase`, `synth`, `position`, `queued`, `phrasesComposed`.
- `tensionController` (`musicTension.h`): `targetFor(difficulty)` (0.70 * x^1.222), per-part
  smoothing (`smoothed()`), the mood walk, `state()` with the mood added.
- `performerPersonality` (`musicPersonality.h`): `generate(seed)` from five latent traits; fifteen
  playing traits, key, mode, tempo, three `instrument`s; `busyness()`, `withinLimits()`; `scaleOf(mode)`.
- `composer` (`musicComposer.h`): `compose(personality, state, situation, rate, phraseBuffer)` writes
  one phrase of lead, chords and bass; `motif`, `phraseBuffer`, `phraseReport` (what the phrase was
  like, for tests); per-theme motif memory.
- `vocabulary` (`musicVocabulary.h`): lead and bass rhythms, progressions, `onsets`, `syncopation`,
  `theme` and `themeFor(situation, personality)`.
- `synthesizer` (`musicSynth.h`): the voice pool, `noteOn`, `noteOff`, `releaseAll`, `render`,
  `setShift(timbreShift)`; `voice`, `oscillator`, `envelope`, `lowPass`, `instrument`, `softClip`.
- `noteEvent`, `eventQueue` (a fixed heap, `cutAt`), `randomStream`, `situation`, `part` (`musicEvents.h`).
- `tuning` (`musicianTuning.h`): every tunable number.

## `soundSpace` (namespace, `soundSpace.h`)

Where OpenAL hears a sound: the board is seen from above, so a board offset (dx, dy) becomes
(dx, -earHeight, dy): right is right, up the screen is in front, down is behind, and the volume
halves with each doubling of the distance (`referenceDistance` 1 cell). `relative(source,
listener)` computes it.
