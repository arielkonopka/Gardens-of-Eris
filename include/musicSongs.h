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
#ifndef MUSICSONGS_H
#define MUSICSONGS_H

#include "musicEvents.h"
#include "musicGenres.h"
#include "musicPersonality.h"
#include "musicTension.h"
#include <array>
#include <cstdint>

/**
 * @brief Turns endless music into a long mixed set of songs.
 *
 * A song has its own key, mode, tempo, groove, swing and favourite chord progressions, and
 * lasts a number of phrases arranged as intro, verses, choruses, a breakdown and an outro.
 * When it ends the next one is either new or an earlier one coming back, so the set
 * interleaves songs the player has heard before with fresh ones. The variety setting decides
 * how short the songs are, how far they move from the performer's home key and tempo, and how
 * often old ones come back. The intensity (the tension, the section, and an alert or a danger)
 * decides how hard the drums play. See docs/adaptive-musician.md, "Songs and the set".
 */
namespace goe::musician {

/// the parts of a song
enum class section : std::uint8_t { intro, verse, chorus, breakdown, outro };

struct song
{
    std::uint32_t id = 0;
    int slot = 0;          ///< where the composer remembers its motifs
    int keyShift = 0;      ///< semitones from the performer's key
    mode scale = mode::dorian;
    float tempo = 84.0f;   ///< beats per minute at no tension, before the player's tempo setting
    int groove = 0;        ///< a vocabulary::grooves index
    float swing = 0.0f;
    int length = 8;        ///< phrases
    float energyLean = 0.0f; ///< added to the performer's energy for this song
    float articulationLean = 0.0f;
    std::array<std::uint8_t, 3> progressions{}; ///< the vocabulary::progressions it favours
    genre style = genre::free; ///< never mixed: a mixed set gives each song a style
};

/// what the next phrase is, in the set and in its song
struct phrasePlan
{
    std::uint32_t songId = 0;
    int slot = 0;
    bool freshSlot = false;  ///< a new song took the slot: the composer forgets its old motifs
    bool songStart = false;
    int phraseInSong = 0;
    section part = section::verse;
    float intensity = 0.0f;  ///< 0..1
    int drums = 1;           ///< 0 none, 1 light, 2 full, 3 driving
    bool fill = false;       ///< the last bar leads into the next section with a fill
    bool lead = true, chords = true, bass = true;
    float densityLift = 0.0f; ///< added to the lead's wanted density
    int groove = 0;
    float swing = 0.0f;
    std::array<std::uint8_t, 3> progressions{0, 1, 2};
    genre style = genre::free;
    // set by the musician from the chip it plays on
    bool arpeggioChords = false;
    int drumChannels = 4;
};

class songbook
{
public:
    explicit songbook(std::uint64_t seed = 0);
    /// the plan of the next phrase; when the song is over, a new one starts or an earlier one comes back.
    /// variety 0..1: 0 long songs near home that often come back, 1 short songs that wander far.
    /// style: the music style chosen; a song of another style ends at once (Mixed: any style)
    phrasePlan next(const performerPersonality &who, const musicalState &tension, situation now, float variety,
                    genre style = genre::free);
    /// the performer as the current song asks: its key, mode and tempo, a little more or less energy
    performerPersonality dressed(const performerPersonality &who) const;
    const song &current() const { return this->songs[(std::size_t) this->playing]; }
    int songsStarted() const { return this->started; }
    int songsReturned() const { return this->returned; }

private:
    song make(const performerPersonality &who, float variety, int slot, genre style);
    section sectionOf(int phrase, const song &s) const;
    std::array<song, tuning::songMemory> songs{};
    int count = 0;     ///< songs remembered
    int playing = 0;   ///< the index of the current song
    int phrase = 0;    ///< the next phrase of the current song
    int oldest = 0;    ///< the slot the next new song takes
    bool freshNext = false;
    bool cameBack = false;
    randomStream choices;
    std::uint32_t nextId = 0;
    int started = 0, returned = 0;
};

} // namespace goe::musician

#endif // MUSICSONGS_H
