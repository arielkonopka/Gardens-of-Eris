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
#ifndef MUSICTENSION_H
#define MUSICTENSION_H

#include "musicEvents.h"
#include <cstdint>

/**
 * @brief Turns the game's difficulty into a smoothly moving musical tension.
 *
 * The difficulty (0..256) gives a target tension on a curve (targetFor). Each part of the
 * music follows that target at its own pace (tempo quickly, harmony and phrase shape slowly),
 * so a jump in difficulty becomes a transition. A slow mood wanders around the tension, so a
 * difficulty that holds still for minutes does not freeze the music.
 */
namespace goe::musician {

/// the tension each part of the music feels right now, mood included; every value 0..1
struct musicalState
{
    float tempo = 0.0f;
    float density = 0.0f;
    float rhythm = 0.0f;
    float harmony = 0.0f;
    float phrase = 0.0f;
    float timbre = 0.0f;
    float mood = 0.0f; ///< -tuning::moodRange .. +tuning::moodRange, already added to the values above
};

class tensionController
{
public:
    explicit tensionController(std::uint64_t seed = 0);
    /// the target tension of a difficulty: tuning::curveScale * (d / 256) ^ tuning::curvePower
    static float targetFor(int difficulty);
    /// clamped to 0..256; the music follows over the next seconds
    void setDifficulty(int difficulty);
    /// jumps straight to the difficulty's tension, for the very start of the music
    void settle(int difficulty);
    /// moves everything on by this much audio time; any step size gives (nearly) the same result
    void advance(float seconds);
    int difficulty() const { return this->level; }
    float target() const { return this->goal; }
    /// the smoothed tension without the mood, per part
    const musicalState &smoothed() const { return this->raw; }
    /// what the music plays with: smoothed plus mood, kept in 0..1
    musicalState state() const;

private:
    int level = 0;
    float goal = 0.0f;
    musicalState raw;
    randomStream moodStream;
};

} // namespace goe::musician

#endif // MUSICTENSION_H
