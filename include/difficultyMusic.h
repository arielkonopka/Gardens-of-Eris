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
#ifndef DIFFICULTYMUSIC_H
#define DIFFICULTYMUSIC_H

#include "difficulty.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>

namespace goe::music {

/// one song of the skin's music list, as the chooser sees it
struct songSlot
{
    int level = 0;       ///< the lowest D the song plays at ("Difficulty" in skins.json)
    bool danger = false; ///< plays while a camera or a guardian is onto the player ("Play": "danger")
};

/**
 * @brief Which song plays for the current difficulty and danger, and how far the fade into it has come.
 *
 * The sound thread asks it every round. The songs of the highest level not above D take turns;
 * below the lowest level the lowest one plays. A new song is chosen as soon as the first D is known;
 * after that D can move the music only once the song has played for difficulty::musicHoldSeconds.
 * Danger does not wait: a danger song starts at once and plays for at least
 * difficulty::musicDangerHoldSeconds, then the difficulty music comes back.
 */
class byDifficulty
{
public:
    using clock = std::chrono::steady_clock;

    /// the index into songs of the song to play, -1 when there is no music.
    /// roll picks among songs that share a level (or among danger songs).
    int choose(int d, bool threatened, const std::vector<songSlot> &songs, clock::time_point now, std::uint32_t roll)
    {
        const int n = (int) songs.size();
        if (n == 0)
            return this->current = -1;
        const bool playing = this->current >= 0 && this->current < n;
        const auto played = now - this->changed;
        if (threatened) {
            const int wanted = this->pick(songs, roll, [](const songSlot &s) { return s.danger; });
            if (wanted >= 0)
                return this->start(wanted, now);
        }
        if (playing) {
            const auto hold = songs[this->current].danger ? difficulty::musicDangerHoldSeconds
                                                          : difficulty::musicHoldSeconds;
            if (played < std::chrono::seconds(hold))
                return this->current;
        }
        const int level = levelFor(d, songs);
        int wanted = this->pick(songs, roll, [level](const songSlot &s) { return !s.danger && s.level == level; });
        if (wanted < 0) // only danger songs in the list
            wanted = this->pick(songs, roll, [](const songSlot &) { return true; });
        return this->start(wanted, now);
    }

    /// the level that plays at D: the highest one not above D, or the lowest one when D is below them all
    static int levelFor(int d, const std::vector<songSlot> &songs)
    {
        int best = -1;
        int lowest = -1;
        bool any = false;
        for (const auto &s : songs) {
            if (s.danger)
                continue;
            if (!any || s.level < lowest)
                lowest = s.level;
            if (s.level <= d && (best < 0 || s.level > best))
                best = s.level;
            any = true;
        }
        return best >= 0 ? best : lowest;
    }

    /// how loud the chosen song is against the one before it: 0 when it has just started, 1 when the fade is over
    float mix(clock::time_point now) const
    {
        const std::chrono::duration<float> since = now - this->changed;
        return std::clamp(since.count() / (float) difficulty::musicCrossfadeSeconds, 0.0f, 1.0f);
    }

private:
    int current = -1;
    clock::time_point changed{};

    int start(int wanted, clock::time_point now)
    {
        if (wanted != this->current) {
            this->current = wanted;
            this->changed = now;
        }
        return this->current;
    }

    /// the playing song when it fits, else one of the fitting songs chosen by roll; -1 when none fits
    template <typename Fits>
    int pick(const std::vector<songSlot> &songs, std::uint32_t roll, Fits fits) const
    {
        std::vector<int> fitting;
        for (int c = 0; c < (int) songs.size(); c++)
            if (fits(songs[c]))
                fitting.push_back(c);
        if (fitting.empty())
            return -1;
        if (std::ranges::find(fitting, this->current) != fitting.end())
            return this->current;
        return fitting[roll % fitting.size()];
    }
};

} // namespace goe::music

#endif // DIFFICULTYMUSIC_H
