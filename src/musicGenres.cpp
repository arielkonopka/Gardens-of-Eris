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
#include "musicGenres.h"
#include "musicVocabulary.h"
#include <cctype>
#include <string>

namespace goe::musician {

namespace {
using vocabulary::grooveNamed;

genreRules make(std::string_view name)
{
    genreRules r;
    r.name = name;
    return r;
}

const std::array<genreRules, genreCount> &allRules()
{
    static const auto table = [] {
        std::array<genreRules, genreCount> t{};
        t[(int) genre::mixed] = make("Mixed");
        t[(int) genre::free] = make("Free");

        auto &rave = t[(int) genre::rave] = make("Rave");
        rave.minTempo = 135.0f;
        rave.maxTempo = 150.0f;
        rave.grooves = {(std::int8_t) grooveNamed("rave"), (std::int8_t) grooveNamed("breakbeat"),
                        (std::int8_t) grooveNamed("four on the floor")};
        rave.progressions = {9, 10, 8, 3};
        rave.modes = {mode::aeolian, mode::dorian, mode::harmonicMinor};
        rave.modeCount = 3;
        rave.bass = bassLine::eighths;
        rave.chordRhythm = "x..x..x...x..x..";
        rave.colour = 0.8f;
        rave.leadDensity = 0.15f;
        rave.leadChance = 0.8f;
        rave.drumFloor = 2;

        auto &techno = t[(int) genre::techno] = make("Techno");
        techno.minTempo = 124.0f;
        techno.maxTempo = 134.0f;
        techno.grooves = {(std::int8_t) grooveNamed("techno"), -1, -1};
        techno.progressions = {8, 9, 8, 9};
        techno.modes = {mode::aeolian, mode::dorian, mode::phrygian};
        techno.modeCount = 3;
        techno.bass = bassLine::offbeat;
        techno.chordRhythm = "...x.....x....x.";
        techno.colour = 0.5f;
        techno.leadDensity = -0.1f;
        techno.leadChance = 0.5f;
        techno.lengthScale = 1.5f;
        techno.drumFloor = 2;

        auto &metal = t[(int) genre::metal] = make("Metal");
        metal.minTempo = 100.0f;
        metal.maxTempo = 160.0f;
        metal.grooves = {(std::int8_t) grooveNamed("metal"), (std::int8_t) grooveNamed("half time"),
                         (std::int8_t) grooveNamed("rock")};
        metal.progressions = {13, 10, 9, 13};
        metal.modes = {mode::phrygian, mode::aeolian, mode::harmonicMinor};
        metal.modeCount = 3;
        metal.bass = bassLine::chug;
        metal.shape = chordShape::power;
        metal.colour = 0.3f;
        metal.leadDensity = 0.2f;
        metal.drumFloor = 2;

        auto &disco = t[(int) genre::disco] = make("Disco");
        disco.minTempo = 112.0f;
        disco.maxTempo = 124.0f;
        disco.grooves = {(std::int8_t) grooveNamed("disco"), (std::int8_t) grooveNamed("four on the floor"), -1};
        disco.progressions = {11, 5, 1, 3};
        disco.modes = {mode::dorian, mode::ionian, mode::mixolydian};
        disco.modeCount = 3;
        disco.bass = bassLine::octave;
        disco.shape = chordShape::seventh;
        disco.chordRhythm = "..x...x...x...x.";
        disco.colour = 1.5f;
        disco.leadChance = 0.9f;
        disco.drumFloor = 2;

        auto &psy = t[(int) genre::psytrance] = make("Psytrance");
        psy.minTempo = 140.0f;
        psy.maxTempo = 148.0f;
        psy.grooves = {(std::int8_t) grooveNamed("psytrance"), -1, -1};
        psy.progressions = {8, 13, 8, 9};
        psy.modes = {mode::phrygian, mode::harmonicMinor, mode::aeolian};
        psy.modeCount = 3;
        psy.bass = bassLine::rolling;
        psy.chordRhythm = "x...............";
        psy.colour = 0.5f;
        psy.leadDensity = 0.5f; // the busy sixteenth lines
        psy.leadChance = 0.8f;
        psy.lengthScale = 1.5f;
        psy.drumFloor = 2;

        auto &jazz = t[(int) genre::jazz] = make("Jazz");
        jazz.minTempo = 84.0f;
        jazz.maxTempo = 150.0f;
        jazz.grooves = {(std::int8_t) grooveNamed("jazz swing"), -1, -1};
        jazz.progressions = {11, 5, 1, 7};
        jazz.modes = {mode::dorian, mode::ionian, mode::mixolydian};
        jazz.modeCount = 3;
        jazz.bass = bassLine::walking;
        jazz.shape = chordShape::seventh;
        jazz.chordRhythm = "x.....x...x.....";
        jazz.colour = 2.5f;
        jazz.leadDensity = 0.1f;
        jazz.drumCeiling = 2;

        auto &rock = t[(int) genre::rock] = make("Rock");
        rock.minTempo = 100.0f;
        rock.maxTempo = 140.0f;
        rock.grooves = {(std::int8_t) grooveNamed("rock"), (std::int8_t) grooveNamed("half time"),
                        (std::int8_t) grooveNamed("shuffle")};
        rock.progressions = {12, 0, 3, 2};
        rock.modes = {mode::mixolydian, mode::aeolian, mode::ionian};
        rock.modeCount = 3;
        rock.bass = bassLine::eighths;
        rock.colour = 0.8f;
        rock.drumFloor = 1;
        return t;
    }();
    return table;
}
} // namespace

const genreRules &rulesOf(genre g)
{
    const int i = (int) g;
    return allRules()[(std::size_t) (i <= 0 || i >= genreCount ? (int) genre::free : i)];
}

std::string_view nameOf(genre g)
{
    const int i = (int) g;
    return allRules()[(std::size_t) (i < 0 || i >= genreCount ? 0 : i)].name;
}

bool sameName(std::string_view a, std::string_view b)
{
    auto plain = [](std::string_view text) {
        std::string out;
        for (char ch : text)
            if (ch != ' ')
                out += (char) std::tolower((unsigned char) ch);
        return out;
    };
    return plain(a) == plain(b);
}

genre genreNamed(std::string_view name)
{
    for (int c = 0; c < genreCount; c++)
        if (sameName(nameOf((genre) c), name))
            return (genre) c;
    return genre::mixed;
}

} // namespace goe::musician
