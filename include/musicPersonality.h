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
#ifndef MUSICPERSONALITY_H
#define MUSICPERSONALITY_H

#include "musicSynth.h"
#include <cstdint>

/**
 * @brief Who the performer is: how they play and how their instrument sounds.
 *
 * Made once from a seed and kept for the whole session. Five latent traits are drawn first
 * and every playing and sound parameter is derived from them, so related parameters move
 * together and no seed gives a cacophonous mix (see generate()).
 */
namespace goe::musician {

/// the few deep traits everything else is derived from, each 0..1
struct latentTraits
{
    float energy = 0.5f;
    float complexity = 0.5f;
    float adventurousness = 0.5f;
    float expressiveness = 0.5f;
    float conservatism = 0.5f;
};

/// a scale, as semitones above its root
enum class mode : std::uint8_t { ionian, dorian, aeolian, mixolydian, harmonicMinor };

struct performerPersonality
{
    latentTraits traits;

    // how they play; every value is 0..1
    float energy = 0.5f;                  ///< loudness and activity
    float rhythmicComplexity = 0.3f;      ///< how far from plain patterns the rhythm goes
    float harmonicAdventurousness = 0.3f; ///< how much the harmony wanders from the plain chords
    float melodicRange = 0.5f;            ///< how wide the melody moves
    float repetition = 0.5f;              ///< how often old material comes back
    float anticipation = 0.2f;            ///< playing strong beats a little early
    float syncopation = 0.2f;             ///< accents off the beat
    float articulation = 0.5f;            ///< 0 short and detached .. 1 long and joined
    float dynamics = 0.5f;                ///< the range between soft and loud notes
    float phraseVariation = 0.4f;         ///< how much a returning motif changes
    float registerBias = 0.5f;            ///< 0 low .. 1 high
    float dissonance = 0.2f;              ///< taste for coloured chords and outside notes
    float silence = 0.3f;                 ///< willingness to leave space
    float timingDrift = 0.3f;             ///< how loosely they keep to the grid
    float stereoMotion = 0.3f;            ///< how much the sound moves between the speakers

    // where they feel at home
    int keyRoot = 50;           ///< MIDI note of the tonic in the chord register
    mode homeMode = mode::dorian;
    float baseTempo = 84.0f;    ///< beats per minute at no tension

    // their instrument
    instrument lead, pad, bass;

    /// the same seed always makes the same performer
    static performerPersonality generate(std::uint64_t seed);
    /// the sum of the traits that make music busy; generate() keeps it under tuning::busynessBudget
    float busyness() const;
    /// every value in its range and the busy traits within budget
    bool withinLimits() const;
};

/// the semitones of a mode's seven degrees
const int *scaleOf(mode m);

} // namespace goe::musician

#endif // MUSICPERSONALITY_H
