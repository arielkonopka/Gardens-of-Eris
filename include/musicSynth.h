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
#ifndef MUSICSYNTH_H
#define MUSICSYNTH_H

#include "musicEvents.h"
#include "musicianTuning.h"
#include <array>
#include <cstdint>

/**
 * @brief The musician's own instrument: a small subtractive synthesizer.
 *
 * Decides HOW the music sounds. Each voice has two oscillators, an ADSR envelope, a
 * low-pass filter, vibrato, tremolo and a place in the stereo field; the synthesizer owns a
 * fixed pool of voices and mixes them with fixed headroom. Nothing here allocates or locks
 * after construction. See docs/adaptive-musician.md, "The synthesizer".
 */
namespace goe::musician {

enum class waveform : std::uint8_t { sine, triangle, saw, pulse };

/// how one part of the band sounds; the performer's personality sets it
struct instrument
{
    waveform wave = waveform::triangle;
    waveform secondWave = waveform::sine;
    float secondMix = 0.0f;      ///< 0..1, how loud the second oscillator is
    float secondInterval = 0.0f; ///< semitones above the first (0, 7, 12, -12)
    float detuneCents = 0.0f;    ///< spread between the oscillators, also spread across left and right
    float pulseWidth = 0.5f;
    float attack = 0.01f, decay = 0.2f, sustain = 0.7f, release = 0.2f; ///< seconds, sustain 0..1
    float cutoff = 2000.0f;      ///< Hz before key tracking and the filter envelope
    float resonance = 0.2f;      ///< 0..tuning::maxResonance
    float filterEnvelope = 0.5f; ///< octaves the cutoff opens on the attack
    float keyTracking = 0.5f;    ///< 1: the cutoff follows the pitch fully
    float vibratoRate = 5.0f, vibratoCents = 0.0f, vibratoOnset = 0.3f;
    float tremoloRate = 4.0f, tremoloDepth = 0.0f;
    float pan = 0.0f;   ///< -1 left .. 1 right
    float width = 0.2f; ///< how far the slow stereo motion takes it from pan
    float gain = 1.0f;
};

/// slow changes the tension makes to every instrument; 1 everywhere means none
struct timbreShift
{
    float brightness = 1.0f; ///< multiplies the cutoff
    float detune = 1.0f;
    float vibrato = 1.0f;
    float attack = 1.0f;     ///< multiplies attack times (below 1: sharper)
    float motion = 1.0f;     ///< multiplies stereo motion
};

/// Sine from a table, the other waveforms band-limited with PolyBLEP so high notes do not alias
class oscillator
{
public:
    void reset(float at = 0.0f) { this->phase = at; }
    /// increment = frequency / sampleRate
    float next(waveform w, float increment, float pulseWidth);

private:
    float phase = 0.0f;
};

/// attack, decay, sustain and release; the level is continuous, so restarting a sounding voice never clicks
class envelope
{
public:
    enum class stage : std::uint8_t { idle, attack, decay, sustain, release };
    void start(float attack, float decay, float sustain, float sampleRate);
    void release(float release, float sampleRate);
    float next();
    stage now() const { return this->at; }
    float level() const { return this->value; }

private:
    stage at = stage::idle;
    float value = 0.0f;
    float attackStep = 0.0f;
    float decayCoef = 0.0f, releaseCoef = 0.0f;
    float sustainLevel = 0.0f;
};

/// a zero-delay-feedback state variable low-pass: stable while the cutoff moves
class lowPass
{
public:
    void reset() { this->ic1 = this->ic2 = 0.0f; }
    void set(float cutoff, float resonance, float sampleRate);
    float process(float x);

private:
    float ic1 = 0.0f, ic2 = 0.0f;
    float a1 = 1.0f, a2 = 0.0f, a3 = 0.0f;
};

/// one sounding note
class voice
{
public:
    void start(std::uint32_t note, part who, float pitch, float velocity, const instrument &sound,
               float sampleRate, std::uint64_t serial);
    void release(float sampleRate, const timbreShift &shift);
    /// adds this voice's next n samples to left and right
    void render(float *left, float *right, int n, float sampleRate, const timbreShift &shift, float motion);
    bool active() const { return this->env.now() != envelope::stage::idle; }
    bool releasing() const { return this->env.now() == envelope::stage::release; }
    float level() const { return this->env.level(); }
    std::uint32_t note() const { return this->noteId; }
    std::uint64_t serial() const { return this->startSerial; }
    part player() const { return this->who; }
    void silence();

private:
    void updateControls(float sampleRate, const timbreShift &shift, float motion);
    instrument sound;
    part who = part::lead;
    std::uint32_t noteId = 0;
    std::uint64_t startSerial = 0;
    float frequency = 440.0f;  ///< where the pitch glides to
    float glided = 440.0f;     ///< the pitch now
    float velocity = 0.0f;
    float age = 0.0f;          ///< seconds since the note started, for the vibrato onset
    float vibratoPhase = 0.0f, tremoloPhase = 0.0f;
    float gainLeft = 0.0f, gainRight = 0.0f;
    float incrementA = 0.0f, incrementB = 0.0f;
    float tremolo = 1.0f;
    int untilControl = 0;
    oscillator a, b;
    envelope env;
    lowPass filter;
};

/// what one render did, for tests
struct mixReport
{
    float peak = 0.0f;
    int activeVoices = 0;
    int stolen = 0;
};

/**
 * @brief The voice pool and the musician's own mixer.
 *
 * At most polyphony() voices sound; a note that finds none free takes one over by a fixed
 * rule (see steal()). The mix has fixed headroom and a soft safety clipper, never per-buffer
 * normalisation, so its loudness does not pump.
 */
class synthesizer
{
public:
    explicit synthesizer(float sampleRate = 44100.0f, int voices = tuning::defaultVoices);
    void setSound(part who, const instrument &sound) { this->sounds[(int) who] = sound; }
    const instrument &soundOf(part who) const { return this->sounds[(int) who]; }
    void setShift(const timbreShift &target) { this->shiftTarget = target; }
    void noteOn(const noteEvent &e);
    void noteOff(const noteEvent &e);
    /// every voice fades out with its release
    void releaseAll();
    /// every voice stops at once (only while the output is faded out)
    void silenceAll();
    /// writes n stereo frames (not added: replaces); n is at most blockFrames
    void render(float *left, float *right, int n);
    int polyphony() const { return this->voices; }
    int activeVoices() const;
    float sampleRate() const { return this->rate; }
    const mixReport &report() const { return this->lastMix; }
    void resetReport() { this->lastMix = {}; }
    static constexpr int blockFrames = 256;

private:
    int steal() const;
    float rate;
    int voices;
    std::array<voice, tuning::voiceCapacity> pool{};
    std::array<instrument, partCount> sounds{};
    timbreShift shiftTarget, shiftNow;
    float motionPhase = 0.0f;
    std::uint64_t serial = 0;
    mixReport lastMix;
};

/// the safety clipper: unchanged up to tuning::clipKnee, then bends smoothly towards +-1
float softClip(float x);

} // namespace goe::musician

#endif // MUSICSYNTH_H
