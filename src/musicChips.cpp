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
#include "musicChips.h"
#include "musicGenres.h"
#include <algorithm>

namespace goe::musician {

namespace {

/// a drum: a short note with no sustain that falls from above its pitch
instrument percussion(waveform wave, float pitch, float drop, float decay, float gain)
{
    instrument d;
    d.wave = wave;
    d.secondMix = 0.0f;
    d.basePitch = pitch;
    d.pitchDrop = drop;
    d.dropTime = 0.03f;
    d.attack = tuning::minAttack;
    d.decay = decay;
    d.sustain = 0.0f;
    d.release = 0.05f;
    d.cutoff = tuning::maxCutoff;
    d.resonance = 0.0f;
    d.filterEnvelope = 0.0f;
    d.keyTracking = 0.0f;
    d.width = 0.0f;
    d.gain = gain;
    return d;
}

/// a chip channel: one raw oscillator, no filter movement, no slow stereo motion
instrument channel(waveform wave, float pulseWidth, float pan, float gain)
{
    instrument i;
    i.wave = wave;
    i.secondMix = 0.0f;
    i.pulseWidth = pulseWidth;
    i.detuneCents = 0.0f;
    i.cutoff = tuning::maxCutoff;
    i.resonance = 0.0f;
    i.filterEnvelope = 0.0f;
    i.keyTracking = 0.0f;
    i.pan = pan;
    i.width = 0.0f;
    i.gain = gain;
    return i;
}

/// how bright the performer's own lead is, 0..1
float brightness(const performerPersonality &who)
{
    return std::clamp((who.lead.cutoff - 900.0f) / 2600.0f, 0.0f, 1.0f);
}

bandSound adlib(const performerPersonality &who)
{
    bandSound b;
    b.lead = who.lead;
    b.pad = who.pad;
    b.bass = who.bass;
    b.chip.channels = {0, 0, 0, 4};
    // drums like an FM card's rhythm mode: a sine kick, a noisy snare with a body, hissing hats
    b.kit[(int) drum::kick] = percussion(waveform::sine, 33.0f, 24.0f, 0.25f, 1.0f);
    auto &snare = b.kit[(int) drum::snare];
    snare = percussion(waveform::noise, 120.0f, 0.0f, 0.15f, 0.55f);
    snare.secondWave = waveform::triangle;
    snare.secondInterval = -64.0f; // a body near 200 Hz under the hiss
    snare.secondMix = 0.6f;
    snare.cutoff = 6000.0f;
    b.kit[(int) drum::hat] = percussion(waveform::noise, 127.0f, 0.0f, 0.04f, 0.3f);
    b.kit[(int) drum::openHat] = percussion(waveform::noise, 127.0f, 0.0f, 0.25f, 0.25f);
    auto &tom = b.kit[(int) drum::tom];
    tom = percussion(waveform::sine, 45.0f, 7.0f, 0.3f, 0.8f);
    tom.dropTime = 0.08f;
    return b;
}

bandSound sid(const performerPersonality &who)
{
    const float bright = brightness(who);
    bandSound b;
    b.chip.channels = {2, 2, 1, 1}; // two SIDs, three channels each
    b.arpeggioChords = true;
    // the lead on the left SID: a pulse whose width sweeps, through the resonant filter
    b.lead = channel(bright > 0.75f ? waveform::saw : waveform::pulse, 0.5f, -0.35f, 0.8f);
    b.lead.pwmRate = 0.4f + 1.2f * who.traits.expressiveness;
    b.lead.pwmDepth = 0.3f;
    b.lead.attack = tuning::minAttack;
    b.lead.decay = 0.3f;
    b.lead.sustain = 0.55f + 0.2f * who.articulation;
    b.lead.release = 0.1f + 0.2f * who.articulation;
    b.lead.cutoff = 1800.0f + 2400.0f * bright;
    b.lead.resonance = 0.45f + 0.2f * who.traits.adventurousness;
    b.lead.filterEnvelope = 0.8f;
    b.lead.keyTracking = 0.6f;
    b.lead.vibratoRate = who.lead.vibratoRate;
    b.lead.vibratoCents = 8.0f + 12.0f * who.traits.expressiveness;
    b.lead.vibratoOnset = 0.25f;
    // the chords on the right SID, as arpeggios of a thin pulse
    b.pad = channel(waveform::pulse, 0.25f, 0.4f, 0.45f);
    b.pad.pwmRate = 0.25f;
    b.pad.pwmDepth = 0.15f;
    b.pad.attack = tuning::minAttack;
    b.pad.decay = 0.6f;
    b.pad.sustain = 0.45f;
    b.pad.release = 0.25f;
    b.pad.cutoff = 2200.0f + 1500.0f * bright;
    b.pad.resonance = 0.3f;
    // a squelchy filtered bass in the middle
    b.bass = channel(who.bass.wave == waveform::saw ? waveform::saw : waveform::pulse, 0.4f, 0.0f, 0.75f);
    b.bass.attack = tuning::minAttack;
    b.bass.decay = 0.2f;
    b.bass.sustain = 0.5f;
    b.bass.release = 0.08f;
    b.bass.cutoff = 450.0f + 300.0f * bright;
    b.bass.resonance = 0.65f;
    b.bass.filterEnvelope = 1.5f;
    b.bass.keyTracking = 0.3f;
    // drums the C64 way: a click of noise, then a pulse falling in pitch
    auto &kick = b.kit[(int) drum::kick];
    kick = percussion(waveform::pulse, 33.0f, 24.0f, 0.18f, 1.0f);
    kick.noiseBurst = 0.006f;
    kick.cutoff = 1500.0f;
    b.kit[(int) drum::snare] = percussion(waveform::noise, 115.0f, 0.0f, 0.12f, 0.55f);
    b.kit[(int) drum::snare].secondWave = waveform::triangle;
    b.kit[(int) drum::snare].secondInterval = -60.0f;
    b.kit[(int) drum::snare].secondMix = 0.5f;
    b.kit[(int) drum::hat] = percussion(waveform::noise, 124.0f, 0.0f, 0.03f, 0.28f);
    b.kit[(int) drum::openHat] = percussion(waveform::noise, 124.0f, 0.0f, 0.2f, 0.22f);
    auto &tom = b.kit[(int) drum::tom];
    tom = percussion(waveform::triangle, 45.0f, 12.0f, 0.25f, 0.8f);
    tom.noiseBurst = 0.008f;
    tom.dropTime = 0.06f;
    return b;
}

bandSound pokey(const performerPersonality &who)
{
    bandSound b;
    b.chip.volumeSteps = 16;
    b.chip.grid = pitchGrid::pokey;
    b.chip.bandLimited = false;
    b.chip.filtered = false;
    b.chip.channels = {2, 2, 2, 2}; // two POKEYs, four channels each
    b.arpeggioChords = true;
    // pure squares, the lead on the left POKEY and the chords on the right
    b.lead = channel(waveform::pulse, 0.5f, -0.5f, 0.7f);
    b.lead.attack = tuning::minAttack;
    b.lead.decay = 0.35f + 0.3f * who.articulation;
    b.lead.sustain = 0.6f;
    b.lead.release = 0.12f;
    b.lead.vibratoRate = who.lead.vibratoRate;
    b.lead.vibratoCents = 6.0f + 10.0f * who.traits.expressiveness;
    b.lead.vibratoOnset = 0.2f;
    b.pad = channel(waveform::pulse, 0.5f, 0.6f, 0.45f);
    b.pad.attack = tuning::minAttack;
    b.pad.decay = 0.6f;
    b.pad.sustain = 0.4f;
    b.pad.release = 0.2f;
    // the buzzy bass, a little to each side so both chips carry it
    b.bass = channel(who.traits.energy > 0.35f ? waveform::poly : waveform::pulse, 0.5f, 0.0f, 0.85f);
    b.bass.attack = tuning::minAttack;
    b.bass.decay = 0.25f;
    b.bass.sustain = 0.6f;
    b.bass.release = 0.08f;
    // noise drums and a square kick, all with the 50 Hz volume steps
    auto &kick = b.kit[(int) drum::kick];
    kick = percussion(waveform::pulse, 36.0f, 24.0f, 0.15f, 0.9f);
    kick.noiseBurst = 0.006f;
    b.kit[(int) drum::snare] = percussion(waveform::noise, 118.0f, 0.0f, 0.12f, 0.6f);
    b.kit[(int) drum::hat] = percussion(waveform::noise, 127.0f, 0.0f, 0.03f, 0.3f);
    b.kit[(int) drum::openHat] = percussion(waveform::noise, 127.0f, 0.0f, 0.15f, 0.27f);
    auto &tom = b.kit[(int) drum::tom];
    tom = percussion(waveform::pulse, 45.0f, 12.0f, 0.2f, 0.6f);
    tom.dropTime = 0.06f;
    return b;
}

bandSound gameboy(const performerPersonality &who)
{
    const float bright = brightness(who);
    bandSound b;
    b.chip.frameRate = 60.0f;
    b.chip.volumeSteps = 16;
    b.chip.grid = pitchGrid::gameboy;
    b.chip.bandLimited = false;
    b.chip.filtered = false;
    b.chip.channels = {1, 1, 1, 1}; // pulse 1, pulse 2, wave, noise
    b.arpeggioChords = true;
    // the duty cycles the chip has: 12.5%, 25%, 50%
    b.lead = channel(waveform::pulse, bright > 0.5f ? 0.125f : 0.25f, 0.0f, 0.55f);
    b.lead.attack = tuning::minAttack;
    b.lead.decay = 0.3f + 0.4f * who.articulation;
    b.lead.sustain = 0.55f;
    b.lead.release = 0.1f;
    b.lead.vibratoRate = who.lead.vibratoRate;
    b.lead.vibratoCents = 8.0f + 12.0f * who.traits.expressiveness;
    b.lead.vibratoOnset = 0.3f;
    b.pad = channel(waveform::pulse, who.traits.conservatism > 0.5f ? 0.5f : 0.125f, 0.8f, 0.32f);
    b.pad.attack = tuning::minAttack;
    b.pad.decay = 0.5f;
    b.pad.sustain = 0.35f;
    b.pad.release = 0.15f;
    b.bass = channel(waveform::wave4, 0.5f, 0.0f, 0.8f);
    b.bass.attack = tuning::minAttack;
    b.bass.decay = 0.3f;
    b.bass.sustain = 0.8f;
    b.bass.release = 0.06f;
    // everything on the one noise channel, a little to the left
    b.kit[(int) drum::kick] = percussion(waveform::noise, 50.0f, 30.0f, 0.1f, 0.9f);
    b.kit[(int) drum::snare] = percussion(waveform::noise, 110.0f, 0.0f, 0.12f, 0.5f);
    b.kit[(int) drum::hat] = percussion(waveform::metal, 120.0f, 0.0f, 0.03f, 0.25f);
    b.kit[(int) drum::openHat] = percussion(waveform::metal, 120.0f, 0.0f, 0.15f, 0.22f);
    b.kit[(int) drum::tom] = percussion(waveform::noise, 70.0f, 12.0f, 0.15f, 0.6f);
    for (auto &d : b.kit)
        d.pan = -0.4f;
    return b;
}
} // namespace

std::string_view nameOf(chipStyle s)
{
    switch (s) {
    case chipStyle::sid: return "SID";
    case chipStyle::pokey: return "POKEY";
    case chipStyle::gameboy: return "Game Boy";
    default: return "AdLib";
    }
}

chipStyle styleNamed(std::string_view name)
{
    for (int c = 0; c < chipStyleCount; c++)
        if (sameName(nameOf((chipStyle) c), name))
            return (chipStyle) c;
    return chipStyle::adlib;
}

bandSound soundFor(chipStyle s, const performerPersonality &who)
{
    switch (s) {
    case chipStyle::sid: return sid(who);
    case chipStyle::pokey: return pokey(who);
    case chipStyle::gameboy: return gameboy(who);
    default: return adlib(who);
    }
}

void dress(synthesizer &band, const bandSound &sound)
{
    band.setChip(sound.chip);
    band.setSound(part::lead, sound.lead);
    band.setSound(part::pad, sound.pad);
    band.setSound(part::bass, sound.bass);
    for (int d = 0; d < drumCount; d++)
        band.setDrum((drum) d, sound.kit[(std::size_t) d]);
}

} // namespace goe::musician
