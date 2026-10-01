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
#include "musicTension.h"
#include <algorithm>
#include <cmath>

namespace goe::musician {

tensionController::tensionController(std::uint64_t seed)
    : moodStream(seed, 3)
{
}

float tensionController::targetFor(int difficulty)
{
    const float x = (float) std::clamp(difficulty, 0, tuning::maxDifficulty) / (float) tuning::maxDifficulty;
    return tuning::curveScale * std::pow(x, tuning::curvePower);
}

void tensionController::setDifficulty(int difficulty)
{
    this->level = std::clamp(difficulty, 0, tuning::maxDifficulty);
    this->goal = targetFor(this->level);
}

void tensionController::settle(int difficulty)
{
    this->setDifficulty(difficulty);
    const float m = this->raw.mood;
    this->raw = {this->goal, this->goal, this->goal, this->goal, this->goal, this->goal, m};
}

void tensionController::advance(float seconds)
{
    if (!(seconds > 0.0f))
        return;
    seconds = std::min(seconds, 5.0f);
    // exponential approach: after `response` seconds 63% of the way, whatever the step size
    auto follow = [this, seconds](float &now, float response) {
        now += (this->goal - now) * (1.0f - std::exp(-seconds / response));
    };
    follow(this->raw.tempo, tuning::tempoResponse);
    follow(this->raw.density, tuning::densityResponse);
    follow(this->raw.rhythm, tuning::rhythmResponse);
    follow(this->raw.harmony, tuning::harmonyResponse);
    follow(this->raw.phrase, tuning::phraseResponse);
    follow(this->raw.timbre, tuning::timbreResponse);
    // the mood: a slow random walk pulled back to the middle (Ornstein-Uhlenbeck), within its range
    float &m = this->raw.mood;
    m += -m * (1.0f - std::exp(-seconds / tuning::moodMemory))
         + tuning::moodRestlessness * std::sqrt(seconds) * this->moodStream.gauss();
    m = std::clamp(m, -tuning::moodRange, tuning::moodRange);
}

musicalState tensionController::state() const
{
    auto withMood = [this](float v) { return std::clamp(v + this->raw.mood, 0.0f, 1.0f); };
    musicalState s;
    s.tempo = withMood(this->raw.tempo);
    s.density = withMood(this->raw.density);
    s.rhythm = withMood(this->raw.rhythm);
    s.harmony = withMood(this->raw.harmony);
    s.phrase = withMood(this->raw.phrase);
    s.timbre = withMood(this->raw.timbre);
    s.mood = this->raw.mood;
    return s;
}

} // namespace goe::musician
