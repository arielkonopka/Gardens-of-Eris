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
#ifndef MUSICVOCABULARY_H
#define MUSICVOCABULARY_H

#include "musicEvents.h"
#include "musicPersonality.h"
#include <array>
#include <string_view>

/**
 * @brief The musical vocabulary the performer chooses from: rhythm patterns, chord progressions
 * and the themes the game's situation calls for.
 *
 * The performer never draws a note or a time at random from nothing; it picks from these,
 * combines them and changes them a little. Patterns are one bar of sixteenth notes: 'x' plays,
 * '.' rests.
 */
namespace goe::musician::vocabulary {

inline constexpr int stepsPerBar = 16;

/// lead rhythms, from sparse and plain to busy and off the beat
inline constexpr std::array<std::string_view, 14> leadRhythms{
    "x.......x.......",
    "x...........x...",
    "x.......x...x...",
    "x...x...x.......",
    "x...x...x...x...",
    "x.....x.x.......",
    "x.....x.....x...",
    "x...x.x...x.....",
    "x..x..x.x...x...",
    "..x.x...x...x...",
    "x.x.x...x.x.x...",
    "x..x..x..x..x.x.",
    "x.x...x.x.x...x.",
    "x.xxx.x.x.xx..x.",
};

/// bass rhythms, from held notes to a driving ostinato
inline constexpr std::array<std::string_view, 6> bassRhythms{
    "x...............",
    "x.......x.......",
    "x.....x.x.......",
    "x...x...x...x...",
    "x..x..x.x..x..x.",
    "x.x.x.x.x.x.x.x.",
};

/// chord progressions as scale degrees (0 is the tonic), one chord per bar
inline constexpr std::array<std::array<int, 4>, 8> progressions{{
    {0, 0, 3, 4},
    {0, 5, 3, 4},
    {0, 3, 0, 4},
    {0, 4, 5, 3},
    {0, 2, 3, 4},
    {0, 5, 1, 4},
    {0, 3, 5, 4},
    {5, 3, 0, 4},
}};

/// a drummer's groove: one bar of kick, snare and hats; busy hats replace the hats at high intensity
struct groove
{
    std::string_view name;
    std::string_view kick, snare, hats, busyHats;
    float swing = 0.0f; ///< share of a sixteenth the off-beat sixteenths are played late
};

inline constexpr std::array<groove, 6> grooves{{
    {"rock", "x.......x.x.....", "....x.......x...", "x.x.x.x.x.x.x.x.", "xxxxxxxxxxxxxxxx", 0.0f},
    {"four on the floor", "x...x...x...x...", "....x.......x...", "..x...x...x...x.", "x.xxx.xxx.xxx.xx", 0.0f},
    {"half time", "x.........x.....", "........x.......", "x.x.x.x.x.x.x.x.", "x.xxx.xxx.xxx.xx", 0.0f},
    {"breakbeat", "x.........x..x..", "....x..x.x..x...", "x.x.x.x.x.x.x.x.", "xxxxxxxxxxxxxxxx", 0.0f},
    {"electro", "x..x..x...x..x..", "....x.......x..x", "xxx.xxx.xxx.xxx.", "xxxxxxxxxxxxxxxx", 0.0f},
    {"shuffle", "x.....x.x.......", "....x.......x...", "x.x.x.x.x.x.x.x.", "x.xxx.xxx.xxx.xx", 0.3f},
}};

/// the second half of a bar that leads into a new section: s snare, t tom, '.' rest
inline constexpr std::array<std::string_view, 4> fills{
    "s.s.t.t.",
    "s.sst.tt",
    "t.t.s.ss",
    "ssssssss",
};

/// how many notes a pattern plays
constexpr int onsets(std::string_view p)
{
    int n = 0;
    for (char c : p)
        n += c == 'x' ? 1 : 0;
    return n;
}

/// how off the beat a pattern is: 0 every note on a beat .. 1 every note between them
constexpr float syncopation(std::string_view p)
{
    float off = 0;
    int n = 0;
    for (int s = 0; s < (int) p.size(); s++) {
        if (p[(std::size_t) s] != 'x')
            continue;
        n++;
        off += s % 4 == 0 ? 0.0f : (s % 2 == 0 ? 0.5f : 1.0f);
    }
    return n == 0 ? 0.0f : off / (float) n;
}

/// how a theme of the band differs from the performer's home theme
struct theme
{
    situation when = situation::calm;
    int transpose = 0;    ///< semitones from the home key
    mode scale = mode::dorian;
    float tempoLift = 1.0f;
    float drive = 0.0f;   ///< 0 bass follows the energy .. 1 a running ostinato
    float density = 0.0f; ///< added to the wanted note density
    float colour = 1.0f;  ///< scales the chance of chromatic notes and coloured chords
    bool pedal = false;   ///< the bass holds the tonic under every chord
};

/**
 * The main theme for calm play; when a camera or a guardian is onto the player, a related
 * theme in another key with a driving bass; under direct danger, the same a semitone higher
 * on a tonic pedal in harmonic minor. None of them changes the tension: the difficulty alone does that.
 */
inline theme themeFor(situation s, const performerPersonality &p)
{
    theme t;
    t.when = s;
    t.scale = p.homeMode;
    if (s == situation::calm)
        return t;
    t.transpose = p.traits.conservatism > 0.5f ? 5 : -5;
    t.scale = p.homeMode == mode::aeolian ? mode::dorian : mode::aeolian;
    t.tempoLift = 1.06f;
    t.drive = 0.6f;
    t.density = 0.1f;
    t.colour = 1.2f;
    if (s == situation::danger) {
        t.transpose += 1;
        t.scale = mode::harmonicMinor;
        t.tempoLift = 1.12f;
        t.drive = 1.0f;
        t.density = 0.15f;
        t.colour = 1.4f;
        t.pedal = true;
    }
    return t;
}

} // namespace goe::musician::vocabulary

#endif // MUSICVOCABULARY_H
