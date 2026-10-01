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
#ifndef MUSICGENRES_H
#define MUSICGENRES_H

#include "musicPersonality.h"
#include <array>
#include <cstdint>
#include <string_view>

/**
 * @brief Music styles: how the band plays, never how it sounds.
 *
 * A style changes the tempo range, the groove, the bass line, the chords' shape and rhythm, the
 * harmony, the modes and how busy the melody is; the instruments stay those of the chip chosen
 * in Config (musicChips.h). Free is the performer's own way, as before styles existed; Mixed
 * gives every new song a style of its own, so the set moves between them. See
 * docs/adaptive-musician.md, "Music styles".
 */
namespace goe::musician {

enum class genre : std::uint8_t { mixed = 0, free, rave, techno, metal, disco, psytrance, jazz, rock };
inline constexpr int genreCount = 9;

/// how the bass plays a bar
enum class bassLine : std::uint8_t {
    follow,  ///< the performer's own: busier with energy and tension
    offbeat, ///< on the off-beat eighths, between the kicks (techno)
    rolling, ///< three sixteenths after every kick (psytrance)
    octave,  ///< eighths jumping between the root and its octave (disco)
    chug,    ///< short, muted sixteenths on the root (metal)
    walking, ///< quarter notes walking to the next chord's root (jazz)
    eighths  ///< plain eighths on the root, the fifth now and then (rock, rave)
};

/// what a chord is made of
enum class chordShape : std::uint8_t {
    triad,   ///< root, third, fifth
    power,   ///< root, fifth, octave: no third (metal)
    seventh  ///< root, third, fifth, seventh as the plain chord (jazz, disco)
};

struct genreRules
{
    std::string_view name;
    float minTempo = 0.0f, maxTempo = 0.0f;  ///< a song's tempo range; 0: the performer's own
    std::array<std::int8_t, 3> grooves{-1, -1, -1};        ///< vocabulary::grooves it plays; -1 unused
    std::array<std::int8_t, 4> progressions{-1, -1, -1, -1}; ///< vocabulary::progressions; -1 unused
    std::array<mode, 3> modes{mode::aeolian, mode::aeolian, mode::aeolian};
    int modeCount = 0;                     ///< 0: the performer's own mode
    bassLine bass = bassLine::follow;
    chordShape shape = chordShape::triad;
    std::string_view chordRhythm;          ///< short chords on these sixteenths; empty: held for the bar
    float colour = 1.0f;                   ///< scales coloured chords and outside notes (still within the limits)
    float leadDensity = 0.0f;              ///< added to the melody's wanted density
    float leadChance = 1.0f;               ///< the chance a verse has a melody at all
    float lengthScale = 1.0f;              ///< songs last this much longer
    int drumFloor = 0, drumCeiling = 3;    ///< the drum level is kept in this range, breakdowns aside
};

/// the rules of a style; Mixed has none of its own (the songbook picks a style per song), so it gets Free's
const genreRules &rulesOf(genre g);
/// "Mixed", "Free", "Rave", "Techno", "Metal", "Disco", "Psytrance", "Jazz", "Rock"
std::string_view nameOf(genre g);
/// the style of a name, ignoring case and spaces; Mixed when none matches
genre genreNamed(std::string_view name);
/// the same name, whatever the case and the spaces ("gameboy" is "Game Boy")
bool sameName(std::string_view a, std::string_view b);

} // namespace goe::musician

#endif // MUSICGENRES_H
