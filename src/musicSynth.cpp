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
#include "musicSynth.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace goe::musician {

namespace {
constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;
constexpr int sineSize = 2048;

/// one cycle of a sine, with the first sample repeated at the end for interpolation
const std::array<float, sineSize + 1> &sineTable()
{
    static const auto table = [] {
        std::array<float, sineSize + 1> t{};
        for (int c = 0; c <= sineSize; c++)
            t[(std::size_t) c] = (float) std::sin(2.0 * std::numbers::pi * c / sineSize);
        return t;
    }();
    return table;
}

float tableSine(float phase)
{
    const auto &t = sineTable();
    const float at = phase * sineSize;
    const int i = std::clamp((int) at, 0, sineSize - 1);
    const float frac = at - (float) i;
    return t[(std::size_t) i] + (t[(std::size_t) i + 1] - t[(std::size_t) i]) * frac;
}

/// the correction that removes most of the aliasing of a jump in the waveform
float polyBlep(float t, float dt)
{
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

float centsRatio(float cents)
{
    return std::exp2(cents / 1200.0f);
}

float midiToHz(float pitch)
{
    return 440.0f * std::exp2((pitch - 69.0f) / 12.0f);
}

/// a coefficient that brings a value 99% of the way in the given time
float settleCoef(float seconds, float sampleRate)
{
    return std::exp(-4.6f / std::max(1.0f, seconds * sampleRate));
}

float flushDenormal(float x)
{
    return std::fabs(x) < 1e-20f ? 0.0f : x;
}

/// the nearest frequency the chip can make
float onGrid(float hz, pitchGrid grid)
{
    switch (grid) {
    case pitchGrid::pokey: {
        // an 8 bit divider of the 64 kHz clock, or of the 15 kHz clock for low notes: high notes go out of tune
        const float clock = hz >= 250.0f ? 63921.0f : 15699.0f;
        const float n = std::clamp(std::round(clock / (2.0f * hz) - 1.0f), 0.0f, 255.0f);
        return clock / (2.0f * (n + 1.0f));
    }
    case pitchGrid::gameboy: {
        const float x = std::clamp(std::round(2048.0f - 131072.0f / hz), 0.0f, 2047.0f);
        return 131072.0f / (2048.0f - x);
    }
    default:
        return hz;
    }
}

/// -1..1 and back once per cycle
float triangleLfo(float phase)
{
    const float t = phase - std::floor(phase);
    return 4.0f * std::fabs(t - 0.5f) - 1.0f;
}
} // namespace

int chipModel::voices() const
{
    int n = 0;
    for (int c : this->channels) {
        if (c <= 0)
            return 0;
        n += c;
    }
    return n;
}

float oscillator::next(waveform w, float increment, float pulseWidth, bool raw)
{
    const float t = this->phase;
    float out;
    switch (w) {
    case waveform::sine:
        out = tableSine(t);
        break;
    case waveform::triangle:
        // its harmonics fall with the square of their number, so it hardly aliases as it is
        out = 4.0f * std::fabs(t - 0.5f) - 1.0f;
        break;
    case waveform::saw:
        out = 2.0f * t - 1.0f - (raw ? 0.0f : polyBlep(t, increment));
        break;
    case waveform::noise:
    case waveform::metal:
        out = (this->lfsr & 1u) != 0 ? 0.6f : -0.6f;
        break;
    case waveform::poly: {
        // a square whose every fourth cycle is flipped: the buzz of POKEY's distortion, still in tune
        // (the pattern repeats every four cycles, so what it adds lies two octaves under the note)
        static constexpr bool pattern[] = {true, true, true, false};
        out = ((t < 0.5f) == pattern[this->cycle] ? 0.6f : -0.6f);
        break;
    }
    case waveform::wave4: {
        // 32 steps of 16 levels, like the Game Boy's wave channel
        const int at = std::min((int) (t * 32.0f), 31);
        const int level = at < 16 ? at : 31 - at;
        out = (float) level / 7.5f - 1.0f;
        break;
    }
    default: {
        const float pw = std::clamp(pulseWidth, 0.05f, 0.95f);
        // a narrow pulse sits mostly below zero: its average is taken off, so notes start and stop without a thump
        const float centre = 0.7f * (2.0f * pw - 1.0f);
        if (raw) {
            out = (t < pw ? 0.7f : -0.7f) - centre;
            break;
        }
        float shifted = t + 1.0f - pw;
        if (shifted >= 1.0f)
            shifted -= 1.0f;
        out = (t < pw ? 1.0f : -1.0f) + polyBlep(t, increment) - polyBlep(shifted, increment);
        out = out * 0.7f - centre; // as loud as the other waveforms sound
        break;
    }
    }
    this->phase += increment;
    if (this->phase >= 1.0f) {
        this->phase -= 1.0f;
        this->cycle = (std::uint8_t) ((this->cycle + 1) % 4);
        // the noise register moves on once a cycle; metal feeds back after 7 bits, so it repeats soon and rings
        const auto bit = (std::uint16_t) ((this->lfsr ^ (this->lfsr >> 1)) & 1u);
        this->lfsr = (std::uint16_t) ((this->lfsr >> 1) | (bit << 14));
        if (w == waveform::metal)
            this->lfsr = (std::uint16_t) ((this->lfsr & ~(1u << 6)) | (bit << 6));
    }
    return out;
}

void envelope::start(float attack, float decay, float sustain, float sampleRate)
{
    this->at = stage::attack;
    // from wherever the level is now, so a voice taken over mid-note does not jump
    this->attackStep = std::max(1.0f - this->value, 0.0f) / std::max(1.0f, attack * sampleRate);
    this->decayCoef = settleCoef(decay, sampleRate);
    this->sustainLevel = std::clamp(sustain, 0.0f, 1.0f);
    if (this->attackStep <= 0.0f)
        this->at = stage::decay;
}

void envelope::release(float release, float sampleRate)
{
    if (this->at == stage::idle)
        return;
    this->at = stage::release;
    this->releaseCoef = settleCoef(release, sampleRate);
}

float envelope::next()
{
    switch (this->at) {
    case stage::attack:
        this->value += this->attackStep;
        if (this->value >= 1.0f) {
            this->value = 1.0f;
            this->at = stage::decay;
        }
        break;
    case stage::decay:
        this->value = this->sustainLevel + (this->value - this->sustainLevel) * this->decayCoef;
        if (std::fabs(this->value - this->sustainLevel) < 1e-4f) {
            this->value = this->sustainLevel;
            this->at = stage::sustain;
        }
        break;
    case stage::release:
        this->value *= this->releaseCoef;
        if (this->value < 1e-4f) { // -80 dB: the last step to silence cannot be heard
            this->value = 0.0f;
            this->at = stage::idle;
        }
        break;
    default:
        break;
    }
    return this->value;
}

void lowPass::set(float cutoff, float resonance, float sampleRate)
{
    const float fc = std::clamp(cutoff, tuning::minCutoff, std::min(tuning::maxCutoff, sampleRate * tuning::maxFrequencyRatio));
    const float g = std::tan(std::numbers::pi_v<float> * fc / sampleRate);
    const float k = 2.0f - 2.0f * std::clamp(resonance, 0.0f, tuning::maxResonance);
    this->a1 = 1.0f / (1.0f + g * (g + k));
    this->a2 = g * this->a1;
    this->a3 = g * this->a2;
    this->ic1 = flushDenormal(this->ic1);
    this->ic2 = flushDenormal(this->ic2);
}

float lowPass::process(float x)
{
    const float v3 = x - this->ic2;
    const float v1 = this->a1 * this->ic1 + this->a2 * v3;
    const float v2 = this->ic2 + this->a2 * this->ic1 + this->a3 * v3;
    this->ic1 = 2.0f * v1 - this->ic1;
    this->ic2 = 2.0f * v2 - this->ic2;
    return v2;
}

void voice::start(const noteEvent &e, float pitch, const instrument &snd, float sampleRate, std::uint64_t serialNo)
{
    const bool wasSounding = this->active();
    this->sound = snd;
    this->who = e.who;
    this->noteId = e.note;
    this->startSerial = serialNo;
    const float limit = sampleRate * tuning::maxFrequencyRatio;
    this->frequency = std::clamp(midiToHz(std::isfinite(pitch) ? pitch : 60.0f), tuning::minFrequency, limit);
    this->velocity = std::clamp(std::isfinite(e.velocity) ? e.velocity : 0.0f, 0.0f, 1.0f);
    this->age = 0.0f;
    this->frameClock = 0.0f;
    this->arpCount = std::min<int>(e.arpCount, (int) e.arp.size());
    for (int c = 0; c < (int) this->arp.size(); c++)
        this->arp[(std::size_t) c] = (std::int8_t) std::clamp<int>(e.arp[(std::size_t) c], -24, 24);
    if (!wasSounding) {
        // a fresh voice starts from silence; a taken-over one keeps its phase and filter, so it glides
        this->glided = this->frequency;
        this->a.reset();
        this->b.reset(0.25f);
        this->filter.reset();
        this->vibratoPhase = 0.0f;
    }
    this->untilControl = 0;
    this->env.start(std::max(snd.attack, tuning::minAttack), std::max(snd.decay, 0.01f), snd.sustain, sampleRate);
}

void voice::release(float sampleRate, const timbreShift &)
{
    this->env.release(std::max(this->sound.release, tuning::minRelease), sampleRate);
}

void voice::silence()
{
    *this = voice();
}

void voice::updateControls(float sampleRate, const timbreShift &shift, float motion, const chipModel &chip)
{
    const float step = (float) tuning::controlInterval / sampleRate;
    this->age += step;
    const float frameRate = chip.frameRate > 0.0f ? chip.frameRate : 50.0f;
    const int frameBefore = (int) this->frameClock;
    this->frameClock += step * frameRate;
    if (chip.volumeSteps > 0 && ((int) this->frameClock != frameBefore || this->age <= step)) {
        // a chip's software envelope: the loudness moves to the nearest of its levels once a frame
        const float steps = (float) chip.volumeSteps - 1.0f;
        this->steppedTarget = std::round(this->env.level() * this->velocity * steps) / steps;
    }
    // about 3 ms to glide to a new pitch when a sounding voice is taken over
    this->glided += (this->frequency - this->glided) * std::min(1.0f, step / 0.003f);

    this->vibratoPhase += this->sound.vibratoRate * step;
    this->vibratoPhase -= std::floor(this->vibratoPhase);
    const float onset = std::clamp((this->age - this->sound.vibratoOnset) / 0.3f, 0.0f, 1.0f);
    const float vibCents = std::min(this->sound.vibratoCents * shift.vibrato, tuning::maxVibratoCents) * onset
                           * std::sin(twoPi * this->vibratoPhase);
    float semitones = vibCents / 100.0f;
    if (this->arpCount > 0) {
        // the chord's tones in turn, one per frame
        const int k = (int) this->frameClock % (this->arpCount + 1);
        if (k > 0)
            semitones += (float) this->arp[(std::size_t) k - 1];
    }
    if (this->sound.pitchDrop != 0.0f)
        semitones += this->sound.pitchDrop * std::exp(-this->age / std::max(this->sound.dropTime, 0.005f));
    const float limit = sampleRate * tuning::maxFrequencyRatio;
    const float fa = std::clamp(onGrid(this->glided * std::exp2(semitones / 12.0f), chip.grid), tuning::minFrequency, limit);
    this->pulseWidth = this->sound.pulseWidth + this->sound.pwmDepth * triangleLfo(this->age * this->sound.pwmRate);
    const float detune = std::min(this->sound.detuneCents * shift.detune, tuning::maxDetuneCents);
    const float fb = std::clamp(fa * centsRatio(this->sound.secondInterval * 100.0f + detune), tuning::minFrequency, limit);
    this->incrementA = fa / sampleRate;
    this->incrementB = fb / sampleRate;

    this->tremoloPhase += this->sound.tremoloRate * step;
    this->tremoloPhase -= std::floor(this->tremoloPhase);
    this->tremolo = 1.0f - this->sound.tremoloDepth * 0.5f * (1.0f + std::sin(twoPi * this->tremoloPhase));

    const float keyFollow = std::exp2(this->sound.keyTracking * std::log2(fa / 261.63f));
    const float opened = std::exp2(this->sound.filterEnvelope * this->env.level());
    this->filter.set(this->sound.cutoff * shift.brightness * keyFollow * opened, this->sound.resonance, sampleRate);

    const float lean = this->who == part::pad ? -1.0f
                       : this->who == part::bass ? 0.3f
                       : this->who == part::drums ? 0.0f
                                                  : 1.0f;
    const float pan = std::clamp(this->sound.pan + this->sound.width * shift.motion * motion * lean, -1.0f, 1.0f);
    const float angle = (pan + 1.0f) * std::numbers::pi_v<float> * 0.25f;
    this->gainLeft = std::cos(angle);
    this->gainRight = std::sin(angle);
}

void voice::render(float *left, float *right, int n, float sampleRate, const timbreShift &shift, float motion,
                   const chipModel &chip)
{
    const float level = this->velocity * this->sound.gain;
    const float mixB = std::clamp(this->sound.secondMix, 0.0f, 1.0f);
    const float mixA = 1.0f - 0.5f * mixB;
    const bool raw = !chip.bandLimited;
    // a stepped level still moves over a millisecond: the steps are heard, the clicks are not
    const float smooth = 1.0f - std::exp(-1.0f / (0.001f * sampleRate));
    for (int i = 0; i < n; i++) {
        if (this->untilControl <= 0) {
            this->updateControls(sampleRate, shift, motion, chip);
            this->untilControl = tuning::controlInterval;
        }
        this->untilControl--;
        const float e = this->env.next();
        const waveform first = this->age < this->sound.noiseBurst ? waveform::noise : this->sound.wave;
        float s = mixA * this->a.next(first, this->incrementA, this->pulseWidth, raw);
        if (mixB > 0.0f)
            s += mixB * this->b.next(this->sound.secondWave, this->incrementB, this->pulseWidth, raw);
        if (chip.filtered)
            s = this->filter.process(s);
        if (chip.volumeSteps > 0) {
            this->stepped += (this->steppedTarget - this->stepped) * smooth;
            s *= this->stepped * this->sound.gain * this->tremolo;
        } else {
            s *= e * level * this->tremolo;
        }
        left[i] += s * this->gainLeft;
        right[i] += s * this->gainRight;
        if (this->env.now() == envelope::stage::idle)
            break;
    }
}

synthesizer::synthesizer(float sampleRate, int voiceCount)
    : rate(std::clamp(sampleRate, 8000.0f, 192000.0f))
    , voices(std::clamp(voiceCount, 1, tuning::voiceCapacity))
{
    sineTable(); // built here, never on the first note
    this->setChip(chipModel{});
}

void synthesizer::setChip(const chipModel &chosen)
{
    // sounding voices go on fading where they are; the steal rule finds them in their new part's channels
    this->model = chosen;
    int dedicated = 0;
    bool shared = false;
    for (int c : chosen.channels) {
        dedicated += std::max(c, 0);
        shared = shared || c <= 0;
    }
    if (dedicated > this->voices - (shared ? 1 : 0)) {
        // more channels than voices: every part shares them all
        this->ranges.fill({0, this->voices});
        return;
    }
    // the dedicated channels from the top of the pool down, the drums' last; the shared ones below
    int end = this->voices;
    for (int p = partCount - 1; p >= 0; p--) {
        const int c = chosen.channels[(std::size_t) p];
        if (c > 0) {
            this->ranges[(std::size_t) p] = {end - c, end};
            end -= c;
        }
    }
    for (int p = 0; p < partCount; p++)
        if (chosen.channels[(std::size_t) p] <= 0)
            this->ranges[(std::size_t) p] = {0, end};
}

int synthesizer::steal(part who) const
{
    // within the part's channels: 1. a free voice; 2. the quietest one already fading out; 3. the oldest note
    const auto [first, last] = this->ranges[(std::size_t) who];
    for (int c = first; c < last; c++)
        if (!this->pool[(std::size_t) c].active())
            return c;
    int best = -1;
    for (int c = first; c < last; c++) {
        const auto &v = this->pool[(std::size_t) c];
        if (!v.releasing())
            continue;
        if (best < 0 || v.level() < this->pool[(std::size_t) best].level()
            || (v.level() == this->pool[(std::size_t) best].level() && v.serial() < this->pool[(std::size_t) best].serial()))
            best = c;
    }
    if (best >= 0)
        return best;
    best = first;
    for (int c = first + 1; c < last; c++)
        if (this->pool[(std::size_t) c].serial() < this->pool[(std::size_t) best].serial())
            best = c;
    return best;
}

void synthesizer::noteOn(const noteEvent &e)
{
    if ((int) e.who >= partCount)
        return;
    const int at = this->steal(e.who);
    auto &v = this->pool[(std::size_t) at];
    if (v.active())
        this->lastMix.stolen++;
    instrument sound;
    float pitch = e.pitch;
    if (e.who == part::drums) {
        // a drum note names the drum; the drum knows its own pitch
        const int d = std::isfinite(e.pitch) ? std::clamp((int) e.pitch, 0, drumCount - 1) : 0;
        sound = this->kit[(std::size_t) d];
        pitch = sound.basePitch;
    } else {
        sound = this->sounds[(int) e.who];
        sound.attack *= this->shiftNow.attack;
    }
    v.start(e, pitch, sound, this->rate, ++this->serial);
}

void synthesizer::noteOff(const noteEvent &e)
{
    for (int c = 0; c < this->voices; c++) {
        auto &v = this->pool[(std::size_t) c];
        if (v.active() && !v.releasing() && v.note() == e.note) {
            v.release(this->rate, this->shiftNow);
            return;
        }
    }
}

void synthesizer::releaseAll()
{
    for (int c = 0; c < this->voices; c++)
        this->pool[(std::size_t) c].release(this->rate, this->shiftNow);
}

void synthesizer::silenceAll()
{
    for (auto &v : this->pool)
        v.silence();
}

int synthesizer::activeVoices() const
{
    int n = 0;
    for (int c = 0; c < this->voices; c++)
        n += this->pool[(std::size_t) c].active() ? 1 : 0;
    return n;
}

void synthesizer::render(float *left, float *right, int n)
{
    n = std::clamp(n, 0, blockFrames);
    std::fill(left, left + n, 0.0f);
    std::fill(right, right + n, 0.0f);
    // the tension already moves the timbre slowly; this only keeps a sudden setShift from stepping
    const float k = 1.0f - settleCoef(0.5f, this->rate / (float) std::max(n, 1));
    auto glide = [k](float &now, float target) { now += (target - now) * k; };
    glide(this->shiftNow.brightness, this->shiftTarget.brightness);
    glide(this->shiftNow.detune, this->shiftTarget.detune);
    glide(this->shiftNow.vibrato, this->shiftTarget.vibrato);
    glide(this->shiftNow.attack, this->shiftTarget.attack);
    glide(this->shiftNow.motion, this->shiftTarget.motion);
    // one slow sweep of the stereo field every 48 seconds
    this->motionPhase += (float) n / this->rate / 48.0f;
    this->motionPhase -= std::floor(this->motionPhase);
    const float motion = std::sin(twoPi * this->motionPhase);

    int active = 0;
    for (int c = 0; c < this->voices; c++) {
        auto &v = this->pool[(std::size_t) c];
        if (!v.active())
            continue;
        active++;
        v.render(left, right, n, this->rate, this->shiftNow, motion, this->model);
    }
    this->lastMix.activeVoices = std::max(this->lastMix.activeVoices, active);
    for (int i = 0; i < n; i++) {
        float l = left[i] * tuning::headroom, r = right[i] * tuning::headroom;
        if (!std::isfinite(l) || !std::isfinite(r)) {
            // a broken voice must not reach the game's mixer: this block and every voice go silent
            std::fill(left, left + n, 0.0f);
            std::fill(right, right + n, 0.0f);
            this->silenceAll();
            return;
        }
        left[i] = softClip(l);
        right[i] = softClip(r);
        this->lastMix.peak = std::max({this->lastMix.peak, std::fabs(left[i]), std::fabs(right[i])});
    }
}

float softClip(float x)
{
    const float a = std::fabs(x);
    if (a <= tuning::clipKnee)
        return x;
    const float room = 1.0f - tuning::clipKnee;
    const float bent = tuning::clipKnee + room * std::tanh((a - tuning::clipKnee) / room);
    return x < 0.0f ? -bent : bent;
}

} // namespace goe::musician
