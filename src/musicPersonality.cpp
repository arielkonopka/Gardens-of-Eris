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
#include "musicPersonality.h"
#include <algorithm>
#include <array>

namespace goe::musician {

namespace {
constexpr std::array<std::array<int, 7>, 6> scales{{
    {0, 2, 4, 5, 7, 9, 11},  // ionian
    {0, 2, 3, 5, 7, 9, 10},  // dorian
    {0, 2, 3, 5, 7, 8, 10},  // aeolian
    {0, 2, 4, 5, 7, 9, 10},  // mixolydian
    {0, 2, 3, 5, 7, 8, 11},  // harmonic minor
    {0, 1, 3, 5, 7, 8, 10},  // phrygian
}};

bool unitRange(float v)
{
    return v >= 0.0f && v <= 1.0f;
}

/// the chaotic traits: generate() scales them together when a performer would be too busy
float chaos(const performerPersonality &p)
{
    return p.rhythmicComplexity + p.syncopation + 1.5f * p.dissonance + p.timingDrift + 0.5f * p.harmonicAdventurousness;
}
} // namespace

const int *scaleOf(mode m)
{
    return scales[(std::size_t) m].data();
}

float performerPersonality::busyness() const
{
    return this->energy + chaos(*this);
}

bool performerPersonality::withinLimits() const
{
    const float values[] = {this->energy, this->rhythmicComplexity, this->harmonicAdventurousness, this->melodicRange,
                            this->repetition, this->anticipation, this->syncopation, this->articulation,
                            this->dynamics, this->phraseVariation, this->registerBias, this->dissonance,
                            this->silence, this->timingDrift, this->stereoMotion};
    if (!std::all_of(std::begin(values), std::end(values), unitRange))
        return false;
    if (this->busyness() > tuning::busynessBudget + 1e-4f)
        return false;
    if (this->baseTempo < tuning::minTempo || this->baseTempo > tuning::maxTempo)
        return false;
    for (const instrument *i : {&this->lead, &this->pad, &this->bass})
        if (i->attack < tuning::minAttack || i->release < tuning::minRelease || i->resonance > tuning::maxResonance
            || i->detuneCents > tuning::maxDetuneCents || i->vibratoCents > tuning::maxVibratoCents
            || i->cutoff < tuning::minCutoff || i->cutoff > tuning::maxCutoff || !unitRange(i->sustain))
            return false;
    return true;
}

performerPersonality performerPersonality::generate(std::uint64_t seed)
{
    randomStream r(seed, 0);
    performerPersonality p;
    auto &t = p.traits;
    t.energy = r.centred();
    t.complexity = r.centred();
    t.adventurousness = r.centred();
    t.expressiveness = r.centred();
    t.conservatism = r.centred();
    const float E = t.energy, C = t.complexity, A = t.adventurousness, X = t.expressiveness, K = t.conservatism;
    // a trait with a little of the performer's own whim in it
    auto lean = [&r](float trait, float weight) {
        return std::clamp(weight * trait + (1.0f - weight) * r.unit(), 0.0f, 1.0f);
    };

    p.energy = 0.2f + 0.6f * lean(E, 0.8f);
    p.rhythmicComplexity = (0.1f + 0.6f * lean(C, 0.75f)) * (1.0f - 0.35f * K);
    p.harmonicAdventurousness = (0.1f + 0.6f * lean(A, 0.75f)) * (1.0f - 0.3f * K);
    p.melodicRange = 0.25f + 0.55f * lean(0.5f * E + 0.5f * X, 0.7f);
    p.repetition = 0.35f + 0.45f * lean(K, 0.7f);
    p.anticipation = 0.05f + 0.45f * lean(0.5f * C + 0.5f * X, 0.7f);
    p.syncopation = 0.05f + 0.5f * lean(0.6f * C + 0.4f * E, 0.7f);
    p.articulation = 0.2f + 0.7f * lean(0.6f * X + 0.4f * (1.0f - E), 0.6f);
    p.dynamics = 0.2f + 0.6f * lean(X, 0.75f);
    p.phraseVariation = 0.15f + 0.55f * lean(0.5f * A + 0.5f * (1.0f - K), 0.7f);
    p.registerBias = r.centred();
    p.dissonance = (0.05f + 0.45f * lean(A, 0.7f)) * (1.0f - 0.4f * K);
    p.silence = 0.15f + 0.6f * lean(0.5f * K + 0.5f * (1.0f - E), 0.6f);
    p.timingDrift = (0.1f + 0.6f * lean(X, 0.6f)) * (1.0f - 0.3f * K);
    p.stereoMotion = 0.1f + 0.6f * lean(0.5f * X + 0.5f * A, 0.6f);

    // safety: a performer who is energetic, complex, syncopated, dissonant and loose all at once
    // would be noise, so the chaotic traits shrink together until the sum fits the budget
    const float room = tuning::busynessBudget - p.energy;
    if (const float c = chaos(p); c > room) {
        const float f = room / c;
        p.rhythmicComplexity *= f;
        p.syncopation *= f;
        p.dissonance *= f;
        p.timingDrift *= f;
        p.harmonicAdventurousness *= f;
    }

    p.keyRoot = 45 + r.below(8);
    // conservative performers lean to the plain modes, adventurous ones to dorian and mixolydian
    const float pickMode = std::clamp(0.5f * r.unit() + 0.5f * (A - K + 0.5f), 0.0f, 0.999f);
    static constexpr mode homeModes[] = {mode::aeolian, mode::ionian, mode::dorian, mode::mixolydian};
    p.homeMode = homeModes[(int) (pickMode * 4.0f)];
    p.baseTempo = 68.0f + 36.0f * lean(E, 0.8f);

    // the lead: brightness and richness come from energy and expressiveness
    const float bright = std::clamp(0.4f * E + 0.3f * X + 0.3f * r.unit(), 0.0f, 1.0f);
    auto &lead = p.lead;
    if (A > 0.6f && r.chance(0.5f))
        lead.wave = waveform::pulse;
    else if (bright > 0.55f)
        lead.wave = waveform::saw;
    else if (bright > 0.3f)
        lead.wave = waveform::triangle;
    else
        lead.wave = waveform::sine;
    lead.secondWave = bright > 0.5f ? waveform::triangle : waveform::sine;
    lead.secondMix = 0.15f + 0.45f * lean(X, 0.5f);
    static constexpr float intervals[] = {0.0f, 0.0f, 12.0f, -12.0f, 7.0f};
    lead.secondInterval = intervals[r.below(K > 0.5f ? 4 : 5)];
    lead.detuneCents = 2.0f + 8.0f * lean(X, 0.6f);
    lead.pulseWidth = r.between(0.3f, 0.5f);
    lead.attack = 0.006f + 0.07f * lean(0.5f * p.articulation + 0.5f * (1.0f - E), 0.7f);
    lead.decay = r.between(0.15f, 0.6f);
    lead.sustain = r.between(0.45f, 0.85f);
    lead.release = 0.08f + 0.42f * p.articulation;
    lead.cutoff = 900.0f + 2600.0f * bright;
    lead.resonance = 0.08f + 0.35f * A;
    lead.filterEnvelope = r.between(0.3f, 1.5f);
    lead.keyTracking = r.between(0.5f, 0.8f);
    lead.vibratoRate = r.between(4.5f, 6.3f);
    lead.vibratoCents = 3.0f + 15.0f * X;
    lead.vibratoOnset = r.between(0.12f, 0.45f);
    lead.tremoloRate = r.between(3.0f, 6.0f);
    lead.tremoloDepth = 0.12f * X * r.unit();
    lead.pan = r.between(-0.2f, 0.2f);
    lead.width = 0.1f + 0.5f * p.stereoMotion;
    lead.gain = 0.9f;

    // the chords: soft and slow
    auto &pad = p.pad;
    pad.wave = r.chance(0.5f) ? waveform::sine : waveform::triangle;
    pad.secondWave = r.chance(0.3f + 0.4f * bright) ? waveform::saw : waveform::triangle;
    pad.secondMix = r.between(0.15f, 0.4f);
    pad.secondInterval = r.chance(0.5f) ? 12.0f : 0.0f;
    pad.detuneCents = r.between(4.0f, 12.0f);
    pad.attack = r.between(0.5f, 1.4f);
    pad.decay = 1.0f;
    pad.sustain = 0.8f;
    pad.release = r.between(1.0f, 2.2f);
    pad.cutoff = 500.0f + 1000.0f * bright;
    pad.resonance = r.between(0.05f, 0.2f);
    pad.filterEnvelope = 0.2f;
    pad.keyTracking = 0.3f;
    pad.vibratoCents = 0.0f;
    pad.pan = -lead.pan;
    pad.width = 0.2f + 0.4f * p.stereoMotion;
    pad.gain = 0.32f;

    // the bass: round and short
    auto &bass = p.bass;
    bass.wave = r.chance(0.5f + 0.3f * E) ? waveform::saw : waveform::triangle;
    bass.secondWave = waveform::sine;
    bass.secondMix = 0.3f;
    bass.secondInterval = -12.0f;
    bass.detuneCents = 0.0f;
    bass.attack = r.between(0.006f, 0.02f);
    bass.decay = 0.3f;
    bass.sustain = 0.6f;
    bass.release = 0.12f;
    bass.cutoff = 250.0f + 450.0f * bright;
    bass.resonance = r.between(0.1f, 0.3f);
    bass.filterEnvelope = 0.8f;
    bass.keyTracking = 0.3f;
    bass.vibratoCents = 0.0f;
    bass.pan = 0.0f;
    bass.width = 0.05f;
    bass.gain = 0.7f;
    return p;
}

} // namespace goe::musician
