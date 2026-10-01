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
#ifndef MUSICCHIPS_H
#define MUSICCHIPS_H

#include "musicPersonality.h"
#include "musicSynth.h"
#include <array>
#include <string_view>

/**
 * @brief The four ways the performer's band can sound.
 *
 * - adlib: the free synthesizer, two detuned oscillators and a filter, like an FM card (the
 *   performer's own instruments);
 * - sid: the C64 with two SIDs for stereo, six channels, a pulse lead with sweeping width, a
 *   resonant filter, chords as fast arpeggios and drums made of a noise click and a falling pitch;
 * - pokey: the Atari 65XE with two POKEYs for stereo, eight raw square channels with 16 volume
 *   steps at 50 frames a second, pitches on 8 bit dividers (high notes go a little out of
 *   tune), a buzzy bass and noise drums;
 * - gameboy: four channels, two pulses with narrow duty cycles, the 32 step wave channel for
 *   the bass, metallic 7 bit noise for the hats and the channels panned left, centre or right.
 *
 * None is bit exact; each gives the feeling of the chip. The personality still shapes the
 * details (duty cycles, filter, vibrato, pulse sweep), so two performers on one chip differ.
 */
namespace goe::musician {

enum class chipStyle : std::uint8_t { adlib = 0, sid = 1, pokey = 2, gameboy = 3 };
inline constexpr int chipStyleCount = 4;

/// "AdLib", "SID", "POKEY", "Game Boy"
std::string_view nameOf(chipStyle s);
/// the style of a name, ignoring case and spaces ("gameboy" is the Game Boy); adlib when none matches
chipStyle styleNamed(std::string_view name);

/// everything the synthesizer needs to sound like one chip
struct bandSound
{
    instrument lead, pad, bass;
    std::array<instrument, drumCount> kit{};
    chipModel chip;
    /// chords are played as fast arpeggios on one voice, not as held notes
    bool arpeggioChords = false;
};

/// the band of a performer on a chip; the same performer and chip always sound the same
bandSound soundFor(chipStyle s, const performerPersonality &who);

/// gives the synthesizer the band's instruments, drums and chip
void dress(synthesizer &band, const bandSound &sound);

} // namespace goe::musician

#endif // MUSICCHIPS_H
