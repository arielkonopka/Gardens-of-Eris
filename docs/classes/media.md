# Seeing and hearing: view, fog, video, sound, configuration

## `viewPoint` (`viewPoint.h`)

A singleton: what the screen follows and what can be seen. `setOwner(elem)` is the element the
view follows (the player, or a drone the player handed a controller to); `addViewPoint(elem)` adds
another element that uncovers the map around it (a patrol drone acts as an extra camera).
`getViewPoints(start, end)` returns the points (`vpPoint`: x, y, radius) in a region for the fog;
`isPointVisible`, `calculateObscured` answer for one cell.

## The game field views (`fieldView.h`, `flatFieldView.h`, `fieldCamera.h`)

`fieldView` is how the game loop draws the game field: one `draw(fieldFrame)` onto the current
target. A `fieldFrame` carries the active avatar (its board is drawn), the `fieldCamera`, how many
whole cells the field shows, and where on the target it goes. `keyTurn()` says how far the ways
the player presses turn so they follow its screen (0 in 2D; -1, an eighth clockwise, in 3D, where
up the screen is the board's north east). Each way of looking at the field is one implementation:
- `flatFieldView`: the sprites seen from above, still elements first, then the ones on the move,
  the avatar last, all drawn into a scene bitmap and shown through the `fogLayer`.
- `isoFieldView`: the 3D isometric view. Each frame it builds the mesh of the cells the camera
  sees (`goe::iso::meshBuilder`), places it from the corner of a window around the camera so the
  numbers stay small, and draws it in one `al_draw_prim` call into a bitmap with a depth buffer.
  The camera looks from the south west at 35.26 degrees (true isometric), orthographic, `zoom`
  screen pixels per tile pixel. Blocks between the avatar and the camera
  (`goe::iso::hides`) are drawn see-through (`ghostOpacity`) after everything solid: first only
  their depth, then their colour, so only their face nearest the camera blends in. Its shader
  (`data/shaders/isoVertex.glvs`, `isoPixel.glps`, GLSL ES compatible) cuts out the clear pixels
  of standing pictures, lays the floor over the board's colour and mixes in the fog from the same
  `fogLayer` mask and pattern as the 2D view. `ready()` is false when the depth buffer or the
  shader cannot be made, and then the 2D view is used. An element with a 3D model (`"Model"` in
  `skins.json`) is drawn as that model instead of its shape, one `al_draw_vertex_buffer` call
  each, moved to its cell by the shader's `placement` matrix and coloured by `tint`; while it
  dies, explodes or teleports it shows its sprite animation. Wall models laid out by `sides`
  get a segment on each side the camera sees that no other block covers (`joins`) and a lid.
  `reveal(true)` shows every cell without fog, for pictures (`goe-gallery`).

The game loop picks the view from `gameSettings::getView()` each frame; F3 switches it.

## `goe::iso` (`isoMesh.h`)

The 3D field's geometry in world space: one unit is a cell, x east, y south, z up. It knows
nothing about cameras or screens, so the same mesh can be drawn by any camera (one per eye in VR).
`shape` is how an element stands: `ground`, `block`, `standee` or `flat`, from `Shape` in
`skins.json` (`parseShape`) or `defaultShape(type)`; `videoElementDef::shape` holds it.
`meshBuilder(eyeX, eyeY)` collects triangles (`vertex`: position, sheet pixel, light, `backdrop`);
standees turn to face the eye direction and block sides facing away are left out. Faces are lit by
direction (`light`): tops brightest, the sides darker.

## `goe::models`, `goe::models::rig` and `modelLibrary` (`model3d.h`, `rig.h`, `modelLibrary.h`)

The 3D models, read from glTF 2.0 files (`.glb`) with [cgltf](https://github.com/jkuhlmann/cgltf)
(`third_party/cgltf`, MIT). `goe::models` works on the CPU only, so the tests need no display:
`load(file, nodes)` reads every node's triangles (or only the named nodes) into a `mesh` in the
board's axes (glTF's y up becomes z, its front looks south) with the material colour and the
texture's bytes. `placed(mesh, spec)` puts it in its cell: centred, its ground (the file's y = 0)
on the floor, scaled to cells (`Scale`, or the footprint fills the cell), turned, stretched,
lifted. `turned(mesh, quarters)` turns it clockwise seen from above, `quartersToFace(dir)` and
`eighthsToFace(dir)` are how far to face a direction (a diagonal is a quarter turn plus an eighth,
the eighth added in the view's placement matrix), `lightOf(normal)` how bright a face is, and `lid(mesh)` the square over a
wall model's cell painted like its top. `Tip` (`spec::tip`) tips a model over
before anything else. `builtin(name)` makes a shape in code instead of reading a file (`load`
does it for a file named `builtin:blob` or `builtin:star`), set on the floor and filling its cell.
`spec` is one `"Model"` entry; `pick(specs, subtype,
facing)` finds the one for an element now, and `readModelSpecs` (`configManager.h`) reads them.
`fit(mesh, spec)` works out how `placed` moves a mesh (its floor, middle and scale) so the poses of
an animated model all stand on the same spot; `placed(mesh, spec, fitting)` uses it. The file
helpers both readers share (`open`, `wanted`, the texture) are in `goe::models::gltf`
(`gltfFile.h`).

`rig` is an animated model: `rig::load(file, nodes)` reads the meshes with their skeleton (glTF
skins: up to four joints pull each corner; a mesh that is no skin, a sword in a hand, follows its
own node) and the animation clips. `clip(name)` finds a clip, `length(clip)` says how long it runs,
and `pose(clip, seconds)` bends the meshes into that moment (clip -1: as the file binds them), in
the board's axes like `load`, on the CPU. `image()` is the texture every pose shares. A `"Model"`
entry with `Animations` (`spec::moves`, by `move`: idle, walk, death) is animated.

`modelLibrary` (needs the display) makes each model once, the first time the 3D view needs it:
four vertex buffers, one per quarter turn with the light baked into the vertex colours, and a
texture shared by models painted with the same picture. `find(def, subtype, facing)` returns
nullptr when the skin gives no model or the file cannot be read, and the view draws the shape.
An animated model keeps its `rig` and its clips instead; `plays(move)` says whether it has a clip
for a move. `posed(model, move, at, eighths)` gives its corners in a moment (`at`: seconds into
standing about or walking, looped; the part of dying done), turned and lit, made once a frame for
each model, move, moment (30 a second) and turn, and `startFrame()` forgets them. The 3D view
picks the move from the element: dying, moving, or standing about, with three phases so a crowd
does not march in step; an element dying without a death clip shows its sprite animation.

`fieldCamera` is which part of the board is shown. `follow(centre, cells, boardSize)` moves it
`step` (8) pixels a frame towards the window centred on `centre`, so the view glides; a bounded
board keeps the window inside. `topLeft()` is the cell in the corner, `scroll()` the pixels past
it, `leftCells()` / `topCells()` the same as fractions of a cell.

## `hudView` (`hudView.h`)

The stats strip under the game field: avatar, energy, weapon, usable item, keys, apples, score,
Dex and D. `draw(player, dx, dy, width)` paints it and puts it on the current target.

## `spritePainter` and `goe::sprites` (`spritePainter.h`, `spriteFrame.h`)

`goe::sprites::frame(elem, def)` is which cell of the sprite sheet an element shows now: its
subtype, facing and animation phase, or the dying, destroying or teleporting (fading out, fading
in) sequence while one runs. `pixelOf(cell, tileSize, spacing)` is that cell's pixels. Every view
uses them, so the rules live in one place.

`spritePainter::draw(elem, cellPx, layer, withMotion)` blits an element and what it stands on:
`layer::still` leaves moving elements for later (and says so), `layer::moving` draws only those,
`layer::top` only the element itself (the HUD's icons).

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
`eventQueueHandle`, `displayHandle`, `shaderHandle`, `vertexBufferHandle`, built on `destroyWith<Destroy>`. Pass
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
screen, fog bitmap, tile size, the music list and who made it (`musicCredits`), the default animations, the sprites
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
- Music: `setupDifficultyMusic()`, `followDifficulty(d)` and `followSituation(s)` play the music
  list by D, with the danger song while a camera or guardian is onto the player, and a crossfade
  (`goe::music::byDifficulty`); `setupSong`, `pauseSong`, `resumeSong`, `moveSong`,
  `hasSong` handle songs placed on the board.
- Volumes come from `gameSettings`.
- The Performer: when `gameSettings` chooses it, the skins.json songs pause and `streamPerformer()`
  feeds a `performerStream` on the sound thread, outside `snd_mutex`, with `followDifficulty` (scaled
  by `difficulty::musicianLevel`) and `followSituation`, and passes Config's performer sound,
  music variety and music tempo to the musician on every pump. The stream is made the first time it
  is chosen and lives as long as the manager. The device's rate (`ALC_FREQUENCY`) is the musician's rate.
- The DJ (`soundManagerDJ.cpp`): when `gameSettings` chooses it, `playDJMusic()` plays the same
  songs by `goe::dj`'s map and mixes. The first time, `djListener` (a thread) listens to every song
  or takes its analysis from `musicmap.json` in the save folder, then maps them under `snd_mutex`.
  `cueSong` seeks a song and fills its buffers, `songPosition` reads where a song is
  (`framesDone` of the buffers played plus `AL_SAMPLE_OFFSET`). A new song is cued ahead and
  started on its mix point within a sound round, the round's error made up with `AL_SAMPLE_OFFSET`;
  `AL_PITCH` bends its speed. Switching the music in Config silences the old choice first.

## The DJ (`goe::dj`, `musicAnalysis.h`, `autoDJ.h`)

The third way of making music (Config, Music: DJ). No game header is included.

- `analyse(mono, rate)`, `analyseFile(path, stop)` give a `trackInfo`: length, `bpm` (spectral-flux
  autocorrelation with a log-normal tempo prior around 130 bpm, then a fine grid search), the beat
  grid and the downbeat (the strongest of four beats), `beatStrength` (`hasBeat()` from 0.2),
  `entry` (the first downbeat past a quiet intro), loudness, flux and brightness. Files are mixed to
  mono at about 11 kHz.
- `mapSongs(songs)`: ranks the songs by intensity (loudness 30%, flux 35%, brightness 15%, tempo 20%)
  and spreads them over levels 0..`topLevel` (10); keeps the "Play": "danger" songs, or makes the most
  intense song the danger song; `gains` play every song as loud as the average one.
- `planMix(playing, next, urgent)`: a `mixPlan` (wait, cue point, speed, fade). It waits for the next
  bar, or the next eight-bar phrase within 12 s; bends the new song by up to 8% to the playing tempo,
  or double or half of it; fades over four bars (3 to 10 s), two bars when the beats cannot match;
  danger takes the next beat and one bar. `crossfade(x)` is equal-power, `glide` brings the speed
  back to 1 over 16 s.
- `analysisCache`: `musicmap.json`, keyed by file, size and time, versioned by `analysisVersion`.
- `goe-dj song ...` prints the analysis and map; `goe-dj --mix out.wav [seconds] [--danger] song ...`
  renders a mix the way the game mixes.

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
  `setVolume` (its own gain), `setStyle` (the chip), `setGenre` (the music style), `setVariety` (0..1), `setTempoScale`
  (0.5..1.5), `pause`, `resume`; on the audio thread `composeAhead()` and `renderAudio(frames)`
  (real-time safe). Inspectors for tests: `personality`, `tension`, `lastPhrase`, `synth`, `songs`,
  `style`, `musicStyle`, `position`, `queued`, `phrasesComposed`.
- `tensionController` (`musicTension.h`): `targetFor(difficulty)` (0.70 * x^1.222), per-part
  smoothing (`smoothed()`), the mood walk, `state()` with the mood added.
- `performerPersonality` (`musicPersonality.h`): `generate(seed)` from five latent traits; fifteen
  playing traits, key, mode, tempo, three `instrument`s; `busyness()`, `withinLimits()`; `scaleOf(mode)`.
- `songbook` (`musicSongs.h`): `next(personality, state, situation, variety, style)` plans the next phrase
  (`phrasePlan`: the song, its style, its `section`, intensity, drum level, fill, which parts play, groove,
  swing, progressions); starts a new `song` or brings an earlier one back when a song ends;
  `dressed(personality)` is the performer in the song's key, mode, tempo and energy.
- `composer` (`musicComposer.h`): `compose(personality, state, situation, rate, plan, phraseBuffer)`
  writes one phrase of lead, chords (held or arpeggiated), bass and drums; `motif`, `phraseBuffer`,
  `phraseReport` (what the phrase was like, for tests); motif memory per song and theme.
- `vocabulary` (`musicVocabulary.h`): lead and bass rhythms, progressions, drum `grooves` and
  `fills`, `onsets`, `syncopation`, `theme` and `themeFor(situation, personality)`.
- `synthesizer` (`musicSynth.h`): the voice pool, `noteOn`, `noteOff`, `releaseAll`, `render`,
  `setShift(timbreShift)`, `setDrum`, `setChip(chipModel)` and `channelsOf(part)`; `voice`,
  `oscillator`, `envelope`, `lowPass`, `instrument`, `chipModel`, `pitchGrid`, `softClip`.
- `chipStyle`, `bandSound`, `soundFor(style, personality)`, `dress(synthesizer, bandSound)`,
  `nameOf`, `styleNamed` (`musicChips.h`): the AdLib, SID, POKEY and Game Boy sounds.
- `genre`, `genreRules`, `rulesOf(genre)`, `nameOf`, `genreNamed`, `sameName` (`musicGenres.h`): the
  music styles (Mixed, Free, Rave, Techno, Metal, Disco, Psytrance, Jazz, Rock): tempo range, grooves,
  `bassLine`, `chordShape`, chord rhythm, progressions, modes, colour, melody density, drum range.
- `noteEvent` (with `arp` for arpeggiated chords), `eventQueue` (a fixed heap, `cutAt`),
  `randomStream`, `situation`, `part`, `drum` (`musicEvents.h`).
- `tuning` (`musicianTuning.h`): every tunable number.

## `soundSpace` (namespace, `soundSpace.h`)

Where OpenAL hears a sound: the board is seen from above, so a board offset (dx, dy) becomes
(dx, -earHeight, dy): right is right, up the screen is in front, down is behind, and the volume
halves with each doubling of the distance (`referenceDistance` 1 cell). `relative(source,
listener)` computes it.
