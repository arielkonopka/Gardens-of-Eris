/*
 * Copyright (c) 2026, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef MUSICIANTUNING_H
#define MUSICIANTUNING_H

/**
 * @brief Every number the adaptive musician is tuned by, in one place.
 *
 * These are for balancing the musician while developing it, not settings for players. Each
 * group says what it changes; docs/adaptive-musician.md explains the ranges and why.
 */
namespace goe::musician::tuning {

// ---- capacity: everything the musician keeps is bounded by these ----

/// storage for voices; the polyphony actually used is set at initialize() and is never above it
inline constexpr int voiceCapacity = 32;
/// the polyphony used when initialize() is given none
inline constexpr int defaultVoices = 16;
/// notes waiting to start or stop; a phrase is composed only while it fits
inline constexpr int eventCapacity = 1024;
/// note events one phrase may hold (every note is an on and an off)
inline constexpr int phraseEventCapacity = 384;
/// motifs remembered per theme
inline constexpr int phraseMemory = 8;
/// longest motif, in notes
inline constexpr int motifNotes = 32;

// ---- difficulty to tension: target = curveScale * (difficulty / 256) ^ curvePower ----

inline constexpr int maxDifficulty = 256;
/// tension at the highest difficulty: never 1, so the musician always keeps some restraint
inline constexpr float curveScale = 0.70f;
/// 0.70 * 0.5^1.222 = 0.30: difficulty 128 gives tension 0.30
inline constexpr float curvePower = 1.222f;

// ---- how fast each part of the music follows the tension (seconds to cover 63% of a change) ----

inline constexpr float tempoResponse = 4.0f;
inline constexpr float densityResponse = 8.0f;
inline constexpr float rhythmResponse = 10.0f;
inline constexpr float harmonyResponse = 20.0f;
inline constexpr float phraseResponse = 25.0f;
inline constexpr float timbreResponse = 20.0f;

// ---- mood: a slow wander around the tension, so a steady difficulty never sounds frozen ----

/// the furthest the mood moves the tension, either way
inline constexpr float moodRange = 0.08f;
/// how long the mood remembers where it was (seconds)
inline constexpr float moodMemory = 40.0f;
/// how strongly it is pushed each second
inline constexpr float moodRestlessness = 0.012f;

// ---- tempo ----

inline constexpr float minTempo = 60.0f;
inline constexpr float maxTempo = 132.0f;
/// at tension 1 the tempo is this much faster than the performer's own
inline constexpr float tempoTensionLift = 0.12f;

// ---- musical safety limits: tension moves towards them, nothing moves past them ----

/// lead notes per beat
inline constexpr float maxNoteDensity = 3.0f;
/// notes sounding at once across lead, chords and bass
inline constexpr int maxSimultaneousNotes = 8;
/// how far a note may be played off the grid
inline constexpr float maxTimingJitterMs = 18.0f;
/// the widest melodic leap, in semitones
inline constexpr int maxRegisterJump = 9;
/// the share of chords that may be coloured (sevenths, suspensions, pedal points)
inline constexpr float maxDissonance = 0.40f;
/// the share of melody notes that may be outside the key
inline constexpr float maxChromaticRate = 0.15f;
/// a motif may be played unchanged at most this many times in a row
inline constexpr int maxConsecutiveRepetitions = 2;
/// at most this many phrases in a row may bring new material
inline constexpr int maxConsecutiveUnrelated = 2;
/// the sum of the busy traits a personality may have (see performerPersonality::busyness)
inline constexpr float busynessBudget = 2.2f;

// ---- synthesizer ----

inline constexpr float minFrequency = 20.0f;
/// the highest note frequency, as a share of the sample rate
inline constexpr float maxFrequencyRatio = 0.45f;
/// the shortest attack and release, so no note starts or stops with a click
inline constexpr float minAttack = 0.004f;
inline constexpr float minRelease = 0.02f;
/// filter cutoff limits in Hz (also kept under maxFrequencyRatio of the sample rate)
inline constexpr float minCutoff = 80.0f;
inline constexpr float maxCutoff = 12000.0f;
inline constexpr float maxResonance = 0.85f;
/// the widest detune between a voice's two oscillators, in cents
inline constexpr float maxDetuneCents = 14.0f;
/// the deepest vibrato, in cents
inline constexpr float maxVibratoCents = 25.0f;
/// the mix's gain before the safety clipper: room for every voice at once
inline constexpr float headroom = 0.45f;
/// above this the safety clipper bends the signal, so the output never passes 1
inline constexpr float clipKnee = 0.8f;
/// samples between updates of filter and modulation coefficients
inline constexpr int controlInterval = 16;
/// how long pause, resume and volume changes take to fade (seconds)
inline constexpr float fadeSeconds = 0.05f;

// ---- scheduling ----

/// music is composed at least this far ahead of what is being played (seconds)
inline constexpr float lookaheadSeconds = 1.0f;

} // namespace goe::musician::tuning

#endif // MUSICIANTUNING_H
