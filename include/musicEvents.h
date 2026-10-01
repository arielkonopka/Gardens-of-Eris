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
#ifndef MUSICEVENTS_H
#define MUSICEVENTS_H

#include "musicianTuning.h"
#include <algorithm>
#include <climits>
#include <array>
#include <cstdint>
#include <random>

/**
 * @brief What the composer hands the synthesizer: notes that start and stop at exact samples.
 *
 * The musician's random streams live here too, so every part draws numbers the same way on
 * every platform (std:: distributions differ between standard libraries).
 */
namespace goe::musician {

/// what the game tells the musician besides the difficulty (musicSituation.h has the game's side)
enum class situation : std::uint8_t { calm = 0, alert = 1, danger = 2 };
inline constexpr int situationCount = 3;

/// the players of the band; the drummer's notes name a drum (see drum), not a pitch
enum class part : std::uint8_t { lead = 0, pad = 1, bass = 2, drums = 3 };
inline constexpr int partCount = 4;

/// the drum kit; a drum note's pitch is one of these
enum class drum : std::uint8_t { kick = 0, snare = 1, hat = 2, openHat = 3, tom = 4 };
inline constexpr int drumCount = 5;

struct noteEvent
{
    enum class kind : std::uint8_t { on, off };
    std::int64_t at = 0;   ///< the sample it happens at, counted from the start of the music
    std::uint32_t order = 0; ///< breaks ties between events of the same sample: first queued, first done
    kind what = kind::on;
    part who = part::lead;
    std::uint32_t note = 0; ///< pairs a note's off with its on
    float pitch = 60.0f;    ///< MIDI note number (for drums: the drum's number)
    float velocity = 0.0f;  ///< 0..1
    /// a chord played as a fast arpeggio on one voice, the way chip music plays chords:
    /// semitones above pitch for the second, third and fourth tone (arpCount of them)
    std::array<std::int8_t, 3> arp{};
    std::uint8_t arpCount = 0;
};

/**
 * @brief Notes waiting for their sample, earliest first, in fixed storage (a binary heap).
 *
 * Pushing and popping never allocate, so the audio thread may pop while rendering.
 */
class eventQueue
{
public:
    bool push(noteEvent e)
    {
        if (this->count >= (int) this->heap.size())
            return false;
        e.order = this->nextOrder++;
        this->heap[(std::size_t) this->count++] = e;
        std::push_heap(this->heap.begin(), this->heap.begin() + this->count, later);
        return true;
    }
    bool empty() const { return this->count == 0; }
    int size() const { return this->count; }
    int room() const { return (int) this->heap.size() - this->count; }
    const noteEvent &top() const { return this->heap[0]; }
    noteEvent pop()
    {
        std::pop_heap(this->heap.begin(), this->heap.begin() + this->count, later);
        return this->heap[(std::size_t) --this->count];
    }
    void clear() { this->count = 0; }
    /**
     * Drops what would start at or after the sample `cut`. A note that started before it, or is
     * already sounding, keeps its end but no later than `cut`. Never allocates.
     */
    void cutAt(std::int64_t cut)
    {
        constexpr std::int64_t dropped = INT64_MAX;
        auto *begin = this->heap.data();
        auto *end = begin + this->count;
        // first mark the ends of notes that will now never start, then keep the rest
        for (auto *e = begin; e != end; ++e)
            if (e->what == noteEvent::kind::off && e->at >= cut
                && std::any_of(begin, end, [e, cut](const noteEvent &o) {
                       return o.what == noteEvent::kind::on && o.note == e->note && o.at >= cut;
                   }))
                e->at = dropped;
        int kept = 0;
        for (auto *e = begin; e != end; ++e) {
            if (e->at < cut) {
                begin[kept++] = *e;
            } else if (e->what == noteEvent::kind::off && e->at != dropped) {
                noteEvent ended = *e;
                ended.at = cut;
                begin[kept++] = ended;
            }
        }
        this->count = kept;
        std::make_heap(this->heap.begin(), this->heap.begin() + this->count, later);
    }

private:
    static bool later(const noteEvent &a, const noteEvent &b)
    {
        return a.at != b.at ? a.at > b.at : a.order > b.order;
    }
    std::array<noteEvent, tuning::eventCapacity> heap{};
    int count = 0;
    std::uint32_t nextOrder = 0;
};

/// one of the musician's random streams: the same numbers for the same seed everywhere
class randomStream
{
public:
    randomStream() = default;
    /// stream k of a seed: different streams of one seed never repeat each other
    randomStream(std::uint64_t seed, std::uint64_t k)
    {
        std::seed_seq s{(std::uint32_t) seed, (std::uint32_t) (seed >> 32), (std::uint32_t) k, 0x5eedu};
        this->e.seed(s);
    }
    /// 0 (included) to 1 (excluded)
    float unit() { return (float) ((this->e() >> 11) * 0x1.0p-53); }
    float between(float lo, float hi) { return lo + (hi - lo) * this->unit(); }
    /// 0 to n - 1
    int below(int n) { return n <= 1 ? 0 : (int) (this->e() % (std::uint64_t) n); }
    bool chance(float p) { return this->unit() < p; }
    /// roughly normal, mean 0 and deviation 1, from twelve uniforms (bounded at +-6)
    float gauss()
    {
        float s = 0;
        for (int c = 0; c < 12; c++)
            s += this->unit();
        return s - 6.0f;
    }
    /// leans towards the middle: the mean of two uniforms
    float centred() { return 0.5f * (this->unit() + this->unit()); }

private:
    std::mt19937_64 e;
};

} // namespace goe::musician

#endif // MUSICEVENTS_H
