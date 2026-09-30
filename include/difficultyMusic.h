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

namespace goe::music {

/**
 * @brief Which song plays for the current difficulty, and how far the fade into it has come.
 *
 * The sound thread asks it every round. A new song is chosen as soon as the first D is known;
 * after that D can move the music only once the song has played for difficulty::musicHoldSeconds.
 */
class byDifficulty
{
public:
    using clock = std::chrono::steady_clock;

    /// the index of the song to play (difficulty::songFor), -1 when there is no music
    int choose(int d, int songs, clock::time_point now)
    {
        const int wanted = difficulty::songFor(d, songs);
        if (wanted == this->current)
            return this->current;
        if (this->current >= 0 && this->current < songs
            && now - this->changed < std::chrono::seconds(difficulty::musicHoldSeconds))
            return this->current;
        this->current = wanted;
        this->changed = now;
        return this->current;
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
};

} // namespace goe::music

#endif // DIFFICULTYMUSIC_H
