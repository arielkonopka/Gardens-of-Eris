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
#ifndef MUSICCOMPOSER_H
#define MUSICCOMPOSER_H

#include "musicEvents.h"
#include "musicPersonality.h"
#include "musicSongs.h"
#include "musicTension.h"
#include "musicVocabulary.h"
#include <array>
#include <cstdint>

/**
 * @brief Decides WHAT the performer plays, one phrase at a time.
 *
 * A phrase is two or four bars of lead melody, chords, bass and drums, written as note events
 * with times counted from the phrase's start. The songbook's plan says which parts play, how
 * hard the drums go and which groove; the composer remembers recent motifs per song and theme
 * and builds each phrase as a repeat, a variation or new material, within the limits of
 * musicianTuning.h. It runs ahead of the audio (adaptiveMusician::composeAhead), never per sample.
 */
namespace goe::musician {

/// a short melodic idea: two bars of rhythm and the scale steps between its notes
struct motif
{
    std::uint32_t id = 0;
    std::uint32_t family = 0; ///< the motif it grew from; a new idea is its own family
    std::array<std::uint8_t, 2> rhythm{}; ///< a vocabulary::leadRhythms index per bar
    std::array<std::int8_t, tuning::motifNotes> steps{}; ///< scale steps before each note (the first from start)
    std::uint32_t rests = 0;  ///< bit k set: note k is left out
    std::int8_t start = 0;    ///< the first note's degree, from the centre of the register
    int notes() const;
};

/// a phrase ready to be queued
struct phraseBuffer
{
    std::array<noteEvent, tuning::phraseEventCapacity> events{};
    int count = 0;
    std::int64_t length = 0;    ///< samples
    std::int64_t barLength = 0; ///< samples
    int bars = 0;
    bool add(const noteEvent &e);
    void clear();
};

/// what the last phrase was like, for tests and tuning
struct phraseReport
{
    enum class relation : std::uint8_t { fresh, repeat, variation };
    relation made = relation::fresh;
    std::uint32_t motifId = 0, family = 0;
    situation theme = situation::calm;
    int transpose = 0;
    int bars = 0;
    bool silent = false;    ///< the lead rested for the whole phrase
    int leadNotes = 0;
    int syncopated = 0;     ///< lead notes off the beat
    int anticipated = 0;    ///< lead notes played a sixteenth before a downbeat
    int chromatic = 0;      ///< lead notes outside the scale
    int chords = 0;
    int colouredChords = 0; ///< sevenths, suspensions, added ninths, borrowed chords, pedal points
    int maxLeap = 0;        ///< semitones
    int maxSimultaneous = 0; ///< lead, chord and bass notes at once (the drums have their own channels)
    std::uint32_t song = 0;
    genre style = genre::free;
    section part = section::verse;
    int drums = 0;          ///< the drum level played, 0..3
    int drumHits = 0;
    bool fill = false;
    float tempo = 0.0f;
    float maxJitterMs = 0.0f;
    float notesPerBeat() const { return bars <= 0 ? 0.0f : (float) leadNotes / (float) (bars * 4); }
};

class composer
{
public:
    explicit composer(std::uint64_t seed = 0);
    /// writes the next phrase into out (cleared first), as the plan asks
    void compose(const performerPersonality &who, const musicalState &tension, situation now, float sampleRate,
                 const phrasePlan &plan, phraseBuffer &out);
    const phraseReport &report() const { return this->last; }
    /// motifs remembered for a theme of the song playing now
    int remembered(situation s) const { return this->banks[(std::size_t) this->slot][(int) s].count; }

private:
    struct memoryBank
    {
        std::array<motif, tuning::phraseMemory> motifs{};
        int count = 0;
        int next = 0;
        motif home;          ///< the theme's main motif, the first one it played
        bool hasHome = false;
    };
    struct leadNote
    {
        int step = 0;       ///< sixteenths from the phrase start (-1 for an anticipated first downbeat)
        int pitch = 60;
        float seconds = 0.2f;
        float velocity = 0.5f;
    };

    motif chooseMotif(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th,
                      phraseReport::relation &made);
    motif fresh(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th);
    motif vary(const motif &m, int changes, const performerPersonality &who, const musicalState &tension,
               const vocabulary::theme &th);
    int pickRhythm(float notesPerBar, float offBeat);
    void remember(situation s, const motif &m);

    int writeChords(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th,
                    const std::array<int, 4> &roots, const phrasePlan &plan, double step, phraseBuffer &out);
    void writeDrums(const performerPersonality &who, const phrasePlan &plan, double step, float rate, phraseBuffer &out);
    /// the sample a sixteenth of the phrase starts at, with the song's swing
    std::int64_t timeOf(int sixteenth, double step) const;
    void writeBass(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th,
                   const std::array<int, 4> &roots, bool pedal, const phrasePlan &plan, double step, float rate,
                   phraseBuffer &out);
    void writeLead(const performerPersonality &who, const musicalState &tension, const vocabulary::theme &th,
                   const motif &m, const std::array<int, 4> &roots, double step, float rate, phraseBuffer &out);

    int pitchOf(int degree, const performerPersonality &who, const vocabulary::theme &th, int octave) const;
    bool inScale(int pitch, const performerPersonality &who, const vocabulary::theme &th) const;
    std::uint32_t note() { return ++this->nextNote; }

    using themeBanks = std::array<memoryBank, situationCount>;
    std::array<themeBanks, tuning::songMemory> banks{}; ///< per song slot, per theme
    int slot = 0;             ///< the song slot playing now
    double swing = 0.0;       ///< the song's swing, a share of a sixteenth
    randomStream phraseChoices;
    randomStream eventChoices;
    int repeats = 0;          ///< unchanged repeats in a row
    int unrelated = 0;        ///< new ideas in a row
    bool lastSilent = false;
    bool deceptive = false;   ///< the next phrase starts away from the tonic
    int lastLead = -1;        ///< the last melody pitch, for leaps across phrases
    float drift = 0.0f;       ///< timing drift now, ms
    std::uint32_t nextNote = 0;
    std::uint32_t nextMotif = 0;
    phraseReport last;
    std::array<leadNote, 4 * vocabulary::stepsPerBar> leadNotes{};
};

} // namespace goe::musician

#endif // MUSICCOMPOSER_H
