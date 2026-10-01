# The adaptive musician

A small autonomous musician that composes and plays the game's music while you play. It has its
own personality, its own instrument, its own memory of what it played, and its own way of reacting
to the game. It does not pick tracks: every note is decided at runtime and every sample is
synthesized at runtime.

Config, **Music**, chooses between **Skin samples** (the songs listed in `skins.json`, played by
difficulty, as before) and **Performer** (this musician). Four more lines shape the performer:

| Config line | Values | `settings.json` | What it does |
|---|---|---|---|
| Music | Skin samples, Performer | `"music": "samples"` or `"performer"` | who plays |
| Performer sound | AdLib, SID, POKEY, Game Boy | `"performerSound": "SID"` | the chip the band sounds like (section 26) |
| Music style | Mixed, Free, Rave, Techno, Metal, Disco, Psytrance, Jazz, Rock | `"musicStyle": "Jazz"` | the kind of music it plays, on the same instruments (section 29) |
| Music variety | 0..100%, default 60% | `"musicVariety": 60` | how often and how far the music changes (section 28) |
| Music tempo | 50..150%, default 100% | `"musicTempo": 100` | the speed, as a share of the composed tempo (section 28) |

Every choice takes effect at once, also during a game.

The fundamental rule of the integration:

> The musician owns the music. The game owns the audio system.

Contents:

1. [Architecture](#1-architecture)
2. [The Music Director](#2-the-music-director)
3. [Difficulty to tension](#3-difficulty-to-tension)
4. [Tension smoothing](#4-tension-smoothing)
5. [Performer personality](#5-performer-personality)
6. [Personality generation](#6-personality-generation)
7. [Personality and difficulty](#7-personality-and-difficulty)
8. [Musical vocabulary](#8-musical-vocabulary)
9. [Rhythm](#9-rhythm)
10. [Harmony](#10-harmony)
11. [Phrase memory and variation](#11-phrase-memory-and-variation)
12. [Mood drift](#12-mood-drift)
13. [The synthesizer](#13-the-synthesizer)
14. [Oscillators](#14-oscillators)
15. [Envelopes](#15-envelopes)
16. [Filter](#16-filter)
17. [Voice management](#17-voice-management)
18. [Randomness and seeds](#18-randomness-and-seeds)
19. [Real-time constraints](#19-real-time-constraints)
20. [Musical safety constraints](#20-musical-safety-constraints)
21. [Configuration parameters](#21-configuration-parameters)
22. [Testing](#22-testing)
23. [Audio integration boundary](#23-audio-integration-boundary)
24. [The game's danger: cameras and guardians](#24-the-games-danger-cameras-and-guardians)
25. [Listening without the game](#25-listening-without-the-game)
26. [Chip sounds](#26-chip-sounds)
27. [Drums](#27-drums)
28. [Songs, the set, variety and tempo](#28-songs-the-set-variety-and-tempo)
29. [Music styles](#29-music-styles)

## 1. Architecture

```text
                     GAME
   difficulty D            cameras and guardians
        |                           |
 difficulty::musicianLevel   goe::music::cues::now()
        |  (0..256)                 |  (calm, alert, danger)
        +-------------+-------------+
                      |  atomics: the music control state
                      v
     +----------------------------------------+
     | adaptiveMusician  (the Music Director)  |
     |                                         |
     |  tensionController   difficulty -> tension, smoothing, mood
     |          |                              |
     |          v                              |
     |  songbook: which song, which section, how hard the drums play
     |          |                              |
     |  performer state = personality dressed by the song + tension + theme
     |          |                              |
     |    +-----+---------------+              |
     |    |                     |              |
     |    v                     v              |
     |  composer             synthesizer       |
     |  (WHAT is played:     (HOW it sounds:   |
     |   rhythm, harmony,     oscillators,     |
     |   melody, drums,       envelopes,       |
     |   phrases, memory)     filter, voices,  |
     |                        a chip model)    |
     |    |                     ^              |
     |    | note events         |              |
     |    +-----> eventQueue ---+              |
     |                          |              |
     +--------------------------|--------------+
                                | stereo float frames
                                v
     performerStream  ->  OpenAL source  ->  game's mixer  ->  device
```

| Layer | Class | Header | Decides |
|---|---|---|---|
| Music Director | `goe::musician::adaptiveMusician` | `include/adaptiveMusician.h` | when to compose, what the game wants, the lifecycle, the musician's own mix |
| Tension controller | `goe::musician::tensionController` | `include/musicTension.h` | how tense the music is, part by part |
| Performer | `goe::musician::performerPersonality` | `include/musicPersonality.h` | who is playing |
| Song planner | `goe::musician::songbook`, `song`, `phrasePlan` | `include/musicSongs.h` | which song plays, its section, the drums' intensity |
| Music generator | `goe::musician::composer`, `vocabulary` | `include/musicComposer.h`, `include/musicVocabulary.h` | WHAT is played |
| Synthesizer | `goe::musician::synthesizer`, `voice`, `oscillator`, `envelope`, `lowPass`, `chipModel` | `include/musicSynth.h` | HOW it sounds |
| Chip sounds | `goe::musician::chipStyle`, `bandSound`, `soundFor`, `dress` | `include/musicChips.h` | which chip the band sounds like |
| Events | `noteEvent`, `eventQueue`, `randomStream` | `include/musicEvents.h` | the language between the generator and the synthesizer |
| Tuning | `goe::musician::tuning` | `include/musicianTuning.h` | every number the musician is balanced with |
| Integration | `performerStream` | `include/performerStream.h` | feeding the game's OpenAL |
| Game signals | `goe::music::cues` | `include/musicCues.h` | the danger the player is in |

The musician headers do not include any game header: the musician can be built, tested and reused
on its own. Only `performerStream` (OpenAL) and `musicCues` (game clock, difficulty rules) know the game.

Diagrams: [musician components](diagrams/15-musician-components.svg) and
[audio thread sequence](diagrams/16-musician-audio-sequence.svg).

## 2. The Music Director

`adaptiveMusician` is the only class the game talks to. It:

- receives the difficulty (`setDifficulty`, 0..256), the situation (`setSituation`), on and off
  (`setEnabled`), the musician's own gain (`setVolume`), and `pause` / `resume`, from any thread,
  into atomics;
- on the audio thread, in `composeAhead()`, moves the tension along in audio time, asks the
  composer for the next phrase whenever less than `tuning::lookaheadSeconds` of music is planned,
  and queues the phrase's note events at absolute sample times;
- in `renderAudio()`, starts and stops notes at their exact samples and lets the synthesizer
  render between them; then applies the fade (pause, enable) and the musician's gain;
- when the situation rises (calm to alert, alert to danger) cuts the planned music at the next
  bar line (`eventQueue::cutAt`), so the new theme is heard within a bar instead of after the
  rest of a ten second phrase. A falling situation waits for the next phrase: relief may take its time.

Rationale: composition happens a phrase at a time, ahead of the audio, so the audio path only
renders. A phrase is never changed after it is composed because of the difficulty (section 36 of
the brief): a big jump in difficulty is heard from the next phrase on, and even then only as far
as the smoothed tension has moved.

## 3. Difficulty to tension

```text
target tension = curveScale * (difficulty / 256) ^ curvePower
               = 0.70 * x ^ 1.222
difficulty   0 -> 0.00
difficulty 128 -> 0.30
difficulty 256 -> 0.70
```

The curve is a power curve: it rises slowly at first, so the easy part of the game stays calm for
longer, and it ends at 0.70, not 1, so the hardest game still leaves the musician restraint. There
are no bands: every integer maps to a slightly different target, and neighbouring integers differ by
less than 0.01 (tested).

The game's difficulty D (`include/difficulty.h`) is small and grows by logarithms (a few steps
across a whole game). `difficulty::musicianLevel(D) = min(23 * D, 256)` spreads it over the
musician's range: D = 11 is near the top.

## 4. Tension smoothing

The target is followed separately by each part of the music, each with its own time constant
(seconds to cover 63% of a change):

| Part | Response | What it drives |
|---|---|---|
| tempo | 4 s | beats per minute |
| density | 8 s | lead notes per beat, bass activity |
| rhythm | 10 s | syncopation, anticipation, timing drift |
| harmony | 20 s | coloured chords, chromatic notes, pedal points, deceptive cadences |
| phrase | 25 s | how much returning motifs change, short phrases, register, silence contrast |
| timbre | 20 s | brightness, detune, vibrato, attack, stereo motion |

`now += (target - now) * (1 - exp(-dt / response))` is exact for any step size, so the result does
not depend on the frame rate or buffer size (tested: a thousand 10 ms steps equal two 5 s steps).
The musician advances it in fixed 20 ms steps of audio time, up to the moment each phrase falls
due, so the same seed and difficulty give the same music whatever the audio buffer size.

A jump from 70 to 180 therefore becomes a slow walk: the tempo is there after some seconds, the
harmony after half a minute.

## 5. Performer personality

`performerPersonality` has fifteen playing traits, each 0..1:

| Trait | Effect |
|---|---|
| energy | note density, loudness, bass activity, tempo |
| rhythmicComplexity | how far rhythms stray from plain patterns |
| harmonicAdventurousness | coloured chords even at low tension |
| melodicRange | how wide the melody moves, how often it leaps |
| repetition | how often old motifs return |
| anticipation | strong beats played a sixteenth early |
| syncopation | preference for off-beat patterns |
| articulation | short and detached .. long and joined |
| dynamics | the range of accents and phrase swells |
| phraseVariation | how much a returning motif changes |
| registerBias | low .. high melody |
| dissonance | taste for colour and outside notes |
| silence | willingness to rest, within and between phrases |
| timingDrift | how loosely the grid is kept |
| stereoMotion | how far the instruments wander in the stereo field |

plus a home key (MIDI 45..52), a home mode (aeolian, ionian, dorian or mixolydian), a base tempo
(68..104 BPM) and three instruments (lead, chords, bass) described in section 13.

## 6. Personality generation

Independent random traits would sometimes make a performer who is energetic, complex,
syncopated, dissonant and loose all at once, which is noise. So `generate(seed)` draws five
**latent traits** first (energy, complexity, adventurousness, expressiveness, conservatism), each
the mean of two uniforms so extremes are rare, and derives every playing and sound parameter from
one or two of them plus a little of the performer's own whim:

```text
trait = lowest + span * (weight * latent + (1 - weight) * uniform)
```

Conservatism damps complexity, adventurousness, dissonance and drift. Finally a **busyness budget**
applies: `energy + rhythmicComplexity + syncopation + 1.5 * dissonance + timingDrift +
0.5 * harmonicAdventurousness` may not exceed `tuning::busynessBudget` (2.2); if it does, the
chaotic traits shrink together. Randomness gives variety, never an invalid performer. 3000 seeds
are checked in the tests.

## 7. Personality and difficulty

```text
parameter = personality baseline + tension influence + small performance variation
```

The personality is fixed for the session; the tension moves. For example the wanted lead density is
`0.6 + 1.1 * energy + 0.9 * density tension` notes per beat, and the wanted syncopation is
`0.6 * syncopation + 0.35 * rhythm tension * (0.5 + rhythmicComplexity)`. Two performers at the same
difficulty differ in key, mode, tempo, rhythms, melody and timbre; the same performer at two
difficulties differs in how busy, coloured and restless it is.

## 8. Musical vocabulary

`include/musicVocabulary.h` holds everything the composer chooses from:

- 14 lead rhythms and 6 bass rhythms, one bar of sixteenths each (`x` plays, `.` rests), from sparse
  and on the beat to busy and off it;
- 8 chord progressions as scale degrees, one chord per bar;
- 5 melodic contours (arch, falling, rising, wave, around one note), in the composer;
- 5 modes, in `musicPersonality.cpp`;
- the themes for calm, alert and danger (section 24).

Nothing is drawn from nothing: the generator picks from these, combines them and changes them a
little (`structure -> valid candidates -> personality selection -> bounded variation -> event`).

## 9. Rhythm

For each new motif the composer scores every lead rhythm by how close its note count and its
syncopation are to the wanted values, and picks among the three closest (60/30/10%). The second
bar repeats the first with probability `0.5 + 0.3 * repetition`. Inside a phrase:

- notes may be left out (`silence * 0.08 * (1 + rhythm tension)`, never the first);
- a downbeat may be anticipated by a sixteenth (`anticipation * (0.3 + 0.7 * rhythm tension)`);
- variations swap a bar for a neighbouring rhythm of similar density;
- the bass follows energy and density with its own patterns, and runs an ostinato under the alert
  and danger themes;
- the timing drift (section 18 of the brief) is a slow, bounded random walk of a few milliseconds,
  pulled in at the phrase start, so the pulse stays.

At higher tension the patterns get busier and more syncopated, but they are still patterns on
a sixteenth grid: the pulse is always there.

## 10. Harmony

A phrase takes one progression (two bar phrases keep its first and last chord). Chords are triads
of the current mode, voiced inside one octave so consecutive chords move smoothly. With probability

```text
colour = min(maxDissonance, harmony tension * maxDissonance * (0.4 + dissonance) * theme colour
                            + 0.05 * harmonicAdventurousness)
```

a chord is coloured: a seventh, a suspended fourth that resolves to the third half way through the
bar, an added ninth, or (above harmony tension 0.4) the flat sixth's major chord borrowed from the
parallel minor. Other mechanisms:

- **pedal point**: the bass holds the tonic under changing chords (`colour * 0.3`; always under danger);
- **delayed resolution**: a phrase ending on V is followed by one starting on vi (`colour * 0.5`);
- **chromatic passing notes** between two notes a whole tone apart, and **lower neighbours**
  leaning into a beat, on weak positions only, at most `maxChromaticRate` of the melody;
- the melody lands on chord tones on every beat.

Most of the music stays plainly tonal; a minority is slightly unexpected; a small minority tense.

## 11. Phrase memory and variation

Each theme (calm, alert, danger) remembers its last eight motifs and its main motif (the first it
played). A motif is two bars: a rhythm per bar, the scale steps between its notes, a starting degree
and the notes it leaves out. For each phrase the composer:

1. returns to remembered material with probability `0.55 + 0.35 * repetition - 0.1 * phrase tension`
   (always, after two new ideas in a row: `maxConsecutiveUnrelated`), half the time to the main motif;
2. changes it by `round(4 * (phraseVariation * (0.4 + 0.6 * phrase tension) + 0.2 * random))`
   operations: a neighbouring rhythm, bent steps, a shifted start, a new ending, an inverted half,
   or a note left out or put back. Zero changes is an exact repeat, allowed twice in a row at most
   (`maxConsecutiveRepetitions`);
3. otherwise writes a new motif.

A four bar phrase is the motif followed by an answer: the same motif with one change. The result
is the `A A' A'' B A''' C B'` shape: recognisable, never frozen. The tests run 600 phrases (over an
hour) per performer and check the limits, the balance of old and new material, and that the density
does not drift.

Silence is part of the vocabulary: besides notes left out, the lead rests for a whole phrase now and
then (`silence * (0.10 + 0.20 * phrase tension)`, never twice running, never before the theme has
been heard), while chords and a sparse bass hold the space.

## 12. Mood drift

```text
mood += -mood * (1 - exp(-dt / moodMemory)) + moodRestlessness * sqrt(dt) * gauss
mood  = clamp(mood, -moodRange, +moodRange)          (0.08)
every part's tension = clamp(smoothed + mood, 0, 1)
```

An Ornstein-Uhlenbeck walk: it wanders, it is pulled back to the middle with a 40 second memory,
and its steps scale with the square root of time so it behaves the same at any step size. At a
constant difficulty the music therefore still breathes: some minutes a little more restless, some
a little calmer. The tests run an hour of mood and check its range, its movement and that the
tension under it does not drift.

## 13. The synthesizer

```text
note event -> voice allocation -> oscillator A + oscillator B -> low-pass filter
           -> amplitude envelope -> tremolo -> equal-power pan -> mix -> headroom -> soft clip
```

Three instruments per performer (`instrument` in `musicSynth.h`), as the AdLib sound plays them;
the other chips replace them with their own channels (section 26), and every chip adds a drum kit
(section 27):

| | Lead | Chords | Bass |
|---|---|---|---|
| waveform | sine, triangle, saw or pulse, by brightness and adventurousness | sine or triangle, with a saw or triangle second oscillator | saw or triangle, with a sine an octave below |
| second oscillator | unison, octave up or down, or a fifth; 2..10 cents apart | unison or octave; 4..12 cents | octave below |
| envelope | 6..76 ms attack, longer for legato performers | 0.5..1.4 s attack, 1..2.2 s release | short and round |
| filter | 900..3500 Hz, resonance 0.08..0.43, opens on the attack | 500..1500 Hz | 250..700 Hz |
| vibrato | 4.5..6.3 Hz, 3..18 cents, after 0.12..0.45 s | none | none |
| stereo | near the centre, wandering by stereoMotion | opposite side, wider | centre |

The tension changes the instruments a little (`timbreShift`): at tension 0.7 the cutoff is 25%
higher, detune and stereo motion 35% wider, vibrato 40% deeper and attacks 20% shorter. Never a
"danger sound", only a slightly sharper instrument.

## 14. Oscillators

A phase accumulator per oscillator, `increment = frequency / sampleRate`, so pitch is right at any
rate (tested at 22.05, 44.1, 48 and 96 kHz). The sine comes from a 2048 point table with linear
interpolation, built once before any note. Saw and pulse are band-limited with PolyBLEP, which
removes most aliasing for the cost of a few multiplications; the triangle's harmonics fall fast
enough to be used as it is. Pitch is clamped to 20 Hz .. 0.45 of the sample rate. When a sounding
voice is taken over, its phase is kept and its pitch glides over 3 ms, so the waveform never jumps.

## 15. Envelopes

Attack, decay, sustain, release. The attack is linear and starts from the current level, so a
retriggered or stolen voice never jumps; decay and release are exponential (99% of the way in the
given time), and a voice goes idle below -80 dB. Attack is never under 4 ms and release never under
20 ms (`minAttack`, `minRelease`), which keeps note starts and ends free of clicks (tested for every
waveform).

## 16. Filter

A zero-delay-feedback state variable low-pass (Zavalishin's topology-preserving transform). It is
stable while its cutoff moves, which matters because the cutoff follows the envelope, the pitch
(key tracking) and the tension. Coefficients are updated every 16 samples (`controlInterval`);
cutoff is kept between 80 Hz and min(12 kHz, 0.45 of the sample rate); resonance is at most 0.85.
Filter state is flushed to zero below 1e-20 so it never runs into denormal numbers.

## 17. Voice management

A fixed pool of `voiceCapacity` (32) voices; the polyphony used is set at `initialize` (16 by
default). The chip model (section 26) splits the pool into channels per part, like a sound chip:
a part with channels of its own only ever uses those, the others share what is left. The AdLib
keeps four voices for the drums and shares twelve between lead, chords and bass; the SID has
2 + 2 + 1 + 1, the POKEYs 2 + 2 + 2 + 2, the Game Boy 1 + 1 + 1 + 1. A note that finds no free
voice in its part's channels takes one of them by a fixed rule:

1. a free voice, lowest index first;
2. otherwise the quietest voice already releasing (ties: the oldest);
3. otherwise the oldest note.

Notes are paired with their ends by id, so the end of a stolen note does nothing. The composer's own
limit (`maxSimultaneousNotes`, 8: one lead, up to four chord notes, one bass) keeps stealing rare;
it mostly happens to release tails.

The voices are mixed with a fixed `headroom` (0.45) and then a soft clipper that leaves the signal
untouched below 0.8 and bends it smoothly towards 1 above. There is no per-buffer normalisation, so
nothing pumps. Over 12 performers at the highest difficulty under danger, fewer than one sample in
ten thousand reaches the clipper (tested).

## 18. Randomness and seeds

`adaptiveMusician::initialize(format, seed)`: the same seed makes the same performer and, with the
same inputs, the same music sample for sample (tested). The seed is split into separate streams so
that drawing from one never shifts another:

| Stream | Used for |
|---|---|
| 0 | the personality |
| 1 | phrase choices: rhythms, progressions, motifs, variations, form |
| 2 | event variation: velocities, lengths, timing drift, notes left out, anticipations |
| 3 | the mood |
| 4 | the songbook: keys, modes, tempos, grooves, lengths, which song comes back |

`randomStream` uses `std::mt19937_64` with `std::seed_seq` and its own conversions to floats and
ranges, so the numbers are the same on Linux and Windows (standard library distributions differ).
In the game the seed comes from `goe::rng::audio()`, a fresh one each session, printed as
`Performer seed: ...`; the gameplay random streams are never touched.

## 19. Real-time constraints

`renderAudio` and everything it calls:

- never allocates (tested by counting `operator new` while rendering a minute of music with
  composing, difficulty and situation changes), never locks, never logs, never touches files;
- reads the controls from atomics;
- works on preallocated state: the voice pool, the event queue (a fixed binary heap of 2048 events),
  the phrase buffer (768 events, room for four bars of busy drums), two 256 frame mix buffers;
- does bounded work per sample: at most 16 voices of two oscillators and a filter.

`composeAhead` composes on the same thread but also never allocates: a phrase is at most a few
hundred events written into fixed arrays. It runs about once per phrase, not per sample.

A minute of music at 48 kHz renders in under half a second on an ordinary CPU (over 100 times real
time; the test allows 6 seconds for slow debug builds).

## 20. Musical safety constraints

These hold at every difficulty and for every performer; tension only moves the music towards them.

| Limit | Value | Where it is enforced |
|---|---|---|
| `maxNoteDensity` | 3 lead notes per beat | rhythm choice |
| `maxSimultaneousNotes` | 8 | the composer's parts (1 + 4 + 1 by construction; the drums have their own channels) |
| `maxTimingJitterMs` | 18 ms | timing drift clamp |
| `maxRegisterJump` | 9 semitones | melody octave correction |
| `maxDissonance` | 40% of chords coloured at most | chord colour chance |
| `maxChromaticRate` | 15% of melody notes | chromatic pass cap |
| `maxConsecutiveRepetitions` | 2 | form choice |
| `maxConsecutiveUnrelated` | 2 | form choice |
| `busynessBudget` | 2.2 | personality generation |
| registers | melody tonic between C4 and B4, chords one octave from D3..C#4, bass from G1..F#2 | octave folding |

The tests check every one of them over 200 performers at difficulty 256 in all three themes.

## 21. Configuration parameters

All in `include/musicianTuning.h` (`goe::musician::tuning`). They are for development and
balancing; players only see Config's Music choice and volume.

| Group | Parameter | Default | Valid range |
|---|---|---|---|
| capacity | `voiceCapacity` / `defaultVoices` | 32 / 16 | 1..32 voices |
| | `eventCapacity`, `phraseEventCapacity` | 2048, 768 | must hold one phrase plus a lookahead |
| | `phraseMemory`, `motifNotes` | 8, 32 | 1.., at least the busiest two bar rhythm |
| tension | `curveScale` | 0.70 | 0..1; keep under 1 for restraint |
| | `curvePower` | 1.222 | > 0; 1 is linear |
| response | `tempoResponse` .. `timbreResponse` | 4, 8, 10, 20, 25, 20 s | > 0 |
| mood | `moodRange`, `moodMemory`, `moodRestlessness` | 0.08, 40 s, 0.012 | small; the mood should be felt, not heard |
| tempo | `minTempo`, `maxTempo`, `tempoTensionLift` | 60, 160 BPM, 12% | |
| songs | `songMemory` | 5 songs | 2.. |
| | `longestSong`, `shortestSong` | 23, 5 phrases | at the lowest and highest variety |
| | `songTempoSpread` | 30% | how far a song's tempo moves at the highest variety |
| | `returnAtLowVariety`, `returnAtHighVariety` | 60%, 25% | the chance the next song is an earlier one |
| | `maxSwing` | 0.33 of a sixteenth | |
| | `minTempoScale`, `maxTempoScale` | 0.5, 1.5 | the player's tempo setting |
| | `alertIntensity`, `dangerIntensity` | 0.25, 0.5 | added to the arrangement's intensity |
| safety | see section 20 | | |
| synth | `minFrequency`, `maxFrequencyRatio` | 20 Hz, 0.45 | |
| | `minAttack`, `minRelease` | 4 ms, 20 ms | > 2 ms to stay click free |
| | `minCutoff`, `maxCutoff`, `maxResonance` | 80 Hz, 12 kHz, 0.85 | resonance < 1 |
| | `maxDetuneCents`, `maxVibratoCents` | 14, 25 | |
| | `headroom`, `clipKnee` | 0.45, 0.8 | |
| | `controlInterval` | 16 samples | |
| | `fadeSeconds` | 50 ms | |
| scheduling | `lookaheadSeconds` | 1 s | at least one audio buffer |

The chips' instruments and drum kits are in `src/musicChips.cpp`, the grooves and fills in
`vocabulary::grooves` and `vocabulary::fills`. The theme differences (key change, tempo lift, bass
drive) are in `vocabulary::themeFor`; the game's
side (how long alerts last, the danger distance, the D scaling) in `include/difficulty.h`.

## 22. Testing

`unitTests/musician-test.cpp` runs the musician offline, rendering into buffers, without a sound
card:

| Area | Tests |
|---|---|
| personality | same seed same performer; 3000 seeds within limits; no cacophonous combinations; performers differ in playing and sound |
| tension | the curve (0, 0.30, 0.70, monotone, no step over 0.01); gradual and per-part pace; independent of step size; smooth both ways for 0->256, 256->0, 64->192, 192->64; 127->128 inaudible; an hour of mood bounded, moving and centred |
| composing | the sweep 0, 32 .. 256 raises tempo, density, syncopation, chromaticism and colour progressively, and stays restrained at the top; every safety limit for 200 performers at 256 in all themes; every note that starts stops; 600 phrases at 128 keep repetition and novelty in balance with no drift; situations change the theme, not the tension |
| synthesizer | no clicks at note starts and ends for every waveform; bounded polyphony and quiet, deterministic stealing; pitch right at four sample rates; bad input stays finite and in range; the clipper never passes 1 |
| musician | continuous and deterministic; the same music with 64, 1000 and 4096 frame buffers (events are sample accurate); no clipping and bounded voices for 12 performers at 256 under danger; rendering never allocates; pause holds and fades; disable and enable; the musician's gain; rising danger heard within a bar; invalid formats give silence; a minute renders fast |
| chips | every chip sounds clean, heard and without offset for 8 performers; each part keeps its own channels (Game Boy one each, AdLib four for drums); POKEY and Game Boy round a C7 to their dividers; an arpeggio plays each chord tone for one 20 ms frame on one voice; a stepped volume holds still inside a frame and takes at most 16 levels |
| drums | no drums at level 0, more hits at each level, a kick and a backbeat in every bar; fills lead into the next section; on one drum channel two drums never start together |
| songs | variety 0 keeps one key and few songs, variety 1 makes over three times as many with other keys, tempos and grooves; earlier songs come back at both ends; all sections appear; under danger the drums never drop below full; a returning song comes in at a chorus with its own remembered motifs; the tempo setting slows and speeds the music; a chip change is heard at once without a gap |
| styles | every style keeps its tempo range, grooves and drum floor and ceiling, Free keeps all grooves; Mixed plays at least six styles; choosing a style ends a song of another at once and no other style comes back; each style plays its own bass line (notes per bar); a style never changes the chip, and every style name round-trips |
| game side | `controller-test`: a camera that sees nothing leaves the music calm, a guardian about to hurt the player makes it danger; cues fade after their hold; D maps into 0..256. `titleMenu-test`: the performer sound steps through the four chips both ways, variety and tempo stay in range, the music style cycles both ways, and all four are kept in `settings.json` |

The OpenAL path was checked by running the game headless with OpenAL Soft's wave writer
(`ALSOFT_CONF` with `drivers=wave`) and the Performer chosen: the music streams at the device's
48 kHz with no dropouts.

## 23. Audio integration boundary

```text
GAME (game thread)
 |  difficulty, situation, Config's choice and music volume
 v                                     (atomics, no locks)
ADAPTIVE MUSICIAN (sound thread: composeAhead, renderAudio)
 |  synthesized stereo float frames, 2048 at a time
 v
performerStream: one streaming OpenAL source, 4 queued buffers
 |
GAME AUDIO ENGINE (soundManager, OpenAL: music bus gain, mixing with sound effects)
 |
 v
MASTER OUTPUT (the device OpenAL opened for the game)
```

1. **Who owns the audio device?** `soundManager`. It opens the OpenAL device and context; the
   musician never opens, closes or reinitialises anything.
2. **Who owns the master mixer?** OpenAL, through `soundManager`'s context. The musician's stream
   is one source in it, like the songs; its gain is Config's music volume (the music bus).
3. **Who owns the musician's voices?** The musician: the voice pool, oscillators, envelopes,
   filters, the event queue, the random streams and its internal mix all live in `adaptiveMusician`.
   `performerStream` owns one OpenAL source and four buffers and deletes them before the context goes.
4. **What crosses the boundary?** Interleaved stereo 32 bit float frames, -1..1, at the rate the
   OpenAL device mixes at (`ALC_FREQUENCY`, 48 kHz on most systems; any rate 8..192 kHz works,
   mono is also supported). `performerStream` hands floats to OpenAL when `AL_EXT_FLOAT32` is there,
   16 bit otherwise. Nothing assumes 44.1 kHz or 16 bit.
5. **Which thread calls the renderer?** `soundManager`'s sound thread (`threadLoop`, every 10 ms),
   outside `snd_mutex`, after it has handled the sound effects. The game has no audio callback;
   the sound thread refilling OpenAL's buffer queue plays that role.
6. **Which functions are for the real-time thread?** `composeAhead()` and `renderAudio()`, always on
   the same thread. The read-only inspectors (`personality`, `tension`, `lastPhrase`, `synth`,
   `position`) on that thread too.
7. **How does game state reach the musician?** The presenter calls
   `soundManager::followDifficulty(D)` and `followSituation(goe::music::cues::now())` every tick;
   they store atomics. The sound thread passes them to `adaptiveMusician::setDifficulty` and
   `setSituation`, which store atomics again. No lock is shared between the game and the audio.
8. **How are difficulty updates communicated?** As the integer D, scaled by
   `difficulty::musicianLevel` and clamped to 0..256 at the boundary; the musician smooths it.
9. **Lifecycle.** The stream is made the first time Config chooses the Performer (initialize), plays
   while chosen (`setEnabled(true)`), fades out and stops its source when Skin samples is chosen
   again (`setEnabled(false)`; the songs come back), and is shut down with `soundManager`. `pause()`
   and `resume()` hold and continue the music where it is; the game has no pause screen yet, so
   nothing calls them today. Switching never touches the sound effects or the device.
10. **How are errors isolated?** Invalid formats make `initialize` fail and the musician renders
    silence; non-finite samples silence the block and every voice; pitches, velocities, cutoffs,
    resonance and volume are clamped; voice and event counts are fixed; a missing OpenAL source
    leaves the musician silent and the rest of the audio untouched; a full event queue skips
    composing until there is room.
11. **How is it tested without the device?** Everything above `performerStream` renders into plain
    buffers in `musician-test`; `goe-musician` renders WAV files (section 25).

The musician owns the music. The game owns the audio system.

## 24. The game's danger: cameras and guardians

Besides the difficulty, the music hears how much danger the player is in, through an explicit
control interface (`include/musicCues.h`), never by looking at game objects:

| Situation | Reported by | Lasts |
|---|---|---|
| alert | a security camera seeing the player (`cues::sighted`), a guardian seeing or chasing the player or going to where they were seen (`cues::chased`) | 10 s after the last report |
| danger | a guardian within 3 cells that sees the player, or one fighting them (`cues::endangered`) | 5 s after the last report |
| calm | nothing fresh | |

Reports are counted in game ticks (`gameClock`). A new world forgets them.

The situation changes the **theme**, not the tension (the difficulty alone sets that):

| | Calm | Alert | Danger |
|---|---|---|---|
| key | home | a fourth up (conservative performers) or down | a semitone above alert |
| mode | the performer's | aeolian (dorian if home is aeolian) | harmonic minor |
| tempo | | +6% | +12% |
| bass | by energy | driving ostinato | running eighths on a tonic pedal |
| colour | | x1.2 | x1.4 |
| motif | the main theme | the main theme upside down, then its own | the alert theme's first bar twice, then its own |

Each theme keeps its own memory, so when the danger passes the main theme returns.

## 25. Listening without the game

`goe-musician` (built with the game, `benchmarks/goe-musician.cpp`) renders a performer to a WAV file:

```sh
goe-musician out.wav [seconds=60] [seed=1] [difficulty=128 or from:to] [calm|alert|danger] [rate=44100]
GOE_PHRASES=1 goe-musician sweep.wav 300 7 0:256     # prints every phrase's report
GOE_STYLE=gameboy GOE_VARIETY=0.9 GOE_TEMPO=1.2 goe-musician gb.wav 120 7 30:220
```

`GOE_STYLE` is `adlib`, `sid`, `pokey` or `gameboy`; `GOE_VARIETY` 0..1 and `GOE_TEMPO` 0.5..1.5
are the Config lines as shares. A phrase report line shows the song, its section, the drum level
and whether the phrase ends in a fill.

## 26. Chip sounds

Config's **Performer sound** puts the same performer on another instrument. Each is a `bandSound`
(`include/musicChips.h`): three instruments, a drum kit, and a `chipModel` that tells the
synthesizer how the chip behaves. None is bit exact; each gives the feeling of the chip.

| | AdLib | SID (C64, stereo) | POKEY (Atari 65XE, stereo) | Game Boy |
|---|---|---|---|---|
| channels (lead + chords + bass + drums) | shared 12 + 4 drums | 2 + 2 + 1 + 1, two SIDs | 2 + 2 + 2 + 2, two POKEYs | 1 + 1 + 1 + 1 |
| lead | the performer's own two oscillator voice | a pulse whose width sweeps (PWM), or a saw for bright performers, through a resonant filter | a pure square | a 12.5% or 25% pulse |
| chords | held, up to four voices | fast arpeggios on one voice | fast arpeggios | fast arpeggios on a 50% or 12.5% pulse |
| bass | the performer's own | a squelchy filtered pulse or saw | a buzzy square that flips every fourth cycle (POKEY's distortion), or a plain square | the 32 step, 16 level wave channel |
| drums | sine kick with a pitch drop, noisy snare with a body, hissing hats | a click of noise, then a falling pulse | a square kick and noise, in volume steps | everything on the noise channel, metallic 7 bit noise for the hats |
| pitch | free | free | 8 bit dividers of the 64 kHz clock (15 kHz for low notes): high notes go a little out of tune | 11 bit dividers |
| volume | smooth | smooth (the SID's ADSR) | 16 levels, moving 50 times a second | 16 levels, 60 times a second |
| waveforms | band-limited | band-limited | raw, with the aliasing of a digital chip | raw |
| filter | yes | yes, resonant | none | none |
| stereo | gentle motion | lead left, chords right | lead left, chords right | channels placed left, centre or right |

How the synthesizer does it (`chipModel` in `musicSynth.h`):

- **Arpeggios.** The composer writes a chord as one note with up to three intervals
  (`noteEvent::arp`); the voice plays the next tone of the chord each frame, so one channel
  sounds like a chord, as on every chip without polyphony.
- **Stepped volume.** With `volumeSteps`, a voice's loudness is rounded to one of 16 levels once a
  frame. The level still moves over a millisecond, so the steps are heard and the clicks are not.
- **Pitch grids.** `pitchGrid::pokey` and `pitchGrid::gameboy` round every frequency to what the
  chip's divider can make (tested: a C7 at 2093 Hz plays at 2131 Hz on the POKEY).
- **New waveforms.** `noise` (a 15 bit shift register clocked at the note's frequency), `metal`
  (the same fed back after 7 bits: short and ringing), `poly` (POKEY's buzz, still in tune because
  its pattern repeats every four cycles, two octaves under the note) and `wave4` (the Game Boy's
  wave channel).
- **Pulse width modulation** (`pwmRate`, `pwmDepth`), **pitch drops** for drums (`pitchDrop`,
  `dropTime`) and a **noise burst** at the start of a note (`noiseBurst`), for the C64's drums.
- A pulse's average is taken off, so narrow pulses do not push the mix off centre and notes do not
  thump when they start.

Changing the sound during play drops what was planned, lets the old chip's notes fade with their
release and starts a new phrase on the new chip at once (`adaptiveMusician::restyle`).

## 27. Drums

Every chip has a kit of five drums (`drum` in `musicEvents.h`): kick, snare, closed hat, open hat
(also the crash) and tom. A drum note names its drum; the kit decides its pitch and sound. The
drummer plays one of six grooves (`vocabulary::grooves`): rock, four on the floor, half time,
breakbeat, electro and shuffle (swung). The phrase plan gives a level:

| Level | What plays |
|---|---|
| 0 | no drums |
| 1, light | the kick on the strong beats, the backbeat, the hats on the beats |
| 2, full | the whole groove, an open hat at the end of every second bar |
| 3, driving | busy sixteenth hats, ghost snares, a crash at the top of the phrase |

The last bar before a new section, or a new song, ends in a fill of snares and toms getting
louder (`vocabulary::fills`). On a chip with one drum channel (the SID, the Game Boy) only one drum
starts at a time, in the order kick, snare, crash, ghost, hat. The drums have their own channels,
so they never take a voice from the melody or the chords.

## 28. Songs, the set, variety and tempo

The music is a long mixed set of songs, planned by the `songbook` (`include/musicSongs.h`). A song
has its own key (up to a fifth from the performer's), mode, tempo, groove, swing, three favourite
chord progressions and a little more or less energy and legato. It lasts a number of phrases
arranged as:

```text
intro  verse verse chorus chorus  verse verse chorus chorus  breakdown  chorus ...  outro
```

| Section | Drums | Parts |
|---|---|---|
| intro | light | half of the time the melody waits and the band plays the groove first |
| verse | light | everyone |
| chorus | full, the melody a little denser, the bass running | everyone |
| breakdown | none under the melody, or the drums and bass alone | |
| outro | light, ending in a fill | everyone |

The **intensity** sets the drum level on top of the section: the tension (difficulty), plus 0.25
when a camera or guardian is onto the player, plus 0.5 under danger. Above 0.35 the drums play one
level harder, above 0.6 two. Under danger they never drop below full, whatever the section. None of
it touches the tension: the situation changes how hard the band plays and which theme it plays,
never the difficulty.

When a song ends the next one is new or an earlier one coming back (it then starts at a chorus,
in its own key and tempo, with the motifs it played before: the composer remembers motifs per song).
The songbook remembers five songs; a new one takes the oldest one's place.

**Music variety** (0..100%) decides how much of this happens:

| | Variety 0% | Variety 100% |
|---|---|---|
| song length | about 23 phrases (four minutes) | about 5 phrases (under a minute) |
| key | always the performer's | changes in 90% of new songs |
| mode, tempo, groove | the performer's own mode and tempo, a groove that suits its energy | other modes, tempos up to 30% away (and audibly different from the last song), any groove, sometimes swing |
| earlier songs coming back | 60% of song changes | 25% |

**Music tempo** (50..150%) scales the composed tempo, which is still kept in 60..160 BPM. The
tempo also moves on its own: each song has its own, the tension lifts it by up to 12%, and an alert
or a danger by 6% or 12%.

## 29. Music styles

Config's **Music style** decides what kind of music the band plays. It changes only the music:
tempo, grooves, the bass line, how chords are shaped and played, progressions, modes, how much the
melody plays, and how coloured the harmony is. It never changes the instruments, which stay the chip
chosen in **Performer sound**. The rules live in `include/musicGenres.h` (`genreRules`, one per
style, in `src/musicGenres.cpp`).

| Style | Tempo | Grooves | Bass | Chords | Other |
|---|---|---|---|---|---|
| Mixed (default) | | each new song gets one of the styles below at random | | | a mixed set of styles |
| Free | the performer's | any of the six free grooves | the performer's own, busier with energy and tension | held or arpeggiated triads | as in section 28 |
| Rave | 135..150 | rave, breakbeat, four on the floor | eighths | stabs | busy melody, drums at least full |
| Techno | 124..134 | techno | off-beat eighths | three sparse stabs a bar | phrygian or aeolian, sparse melody, long songs |
| Metal | 100..160 | metal (16th kicks when driving), half time, rock | muted 16th chug | power chords (no third) | phrygian, aeolian, harmonic minor |
| Disco | 112..124 | disco (open hats on the offbeats), four on the floor | octave jumps | sevenths on the offbeats | richer colour |
| Psytrance | 140..148 | psytrance | rolling: three sixteenths after every kick | one stab a bar | busy melody, long songs |
| Jazz | 84..150 | jazz swing | walking quarters with a chromatic approach | sevenths, comped | the most colour, drums never driving |
| Rock | 100..140 | rock, half time, shuffle | eighths with the fifth | held triads | |

The tempo of a song falls in its style's range: near the middle at low variety, anywhere in it at
high variety. **Music tempo** still scales it, within 60..160 BPM. The songbook keeps the style
per song, so a returning song comes back in its own style. Choosing a style during play ends a song
of another style at the next bar, and only songs of the chosen style come back. The safety limits
(section 20) hold for every style: the style's colour and density are added before the limits are
applied.

`goe-musician` takes `GOE_GENRE=jazz` (any style name) and prints the style of each phrase.
