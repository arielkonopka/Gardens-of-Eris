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
#ifndef PERFORMERSTREAM_H
#define PERFORMERSTREAM_H

#include "adaptiveMusician.h"
#include <AL/al.h>
#include <array>
#include <cstdint>
#include <vector>

/**
 * @brief The integration point between the adaptive musician and the game's OpenAL audio.
 *
 * The game's sound system owns the device, the context and the music volume; this owns one
 * streaming OpenAL source on that context and keeps its queue of buffers filled with the
 * musician's frames. Everything here runs on the sound thread (soundManager::threadLoop).
 * See docs/adaptive-musician.md, "Audio integration boundary".
 */
class performerStream
{
public:
    static constexpr int bufferCount = 4;
    static constexpr int bufferFrames = 2048;

    /// a source and buffers on the current OpenAL context, and a musician at the device's rate
    performerStream(int sampleRate, std::uint64_t seed);
    ~performerStream();
    performerStream(const performerStream &) = delete;
    performerStream &operator=(const performerStream &) = delete;

    /// keeps the source fed: refills every buffer OpenAL has played, restarts after an underrun.
    /// gain is the game's music volume (the music bus); the musician's own gain stays its own.
    void pump(int difficulty, goe::musician::situation now, float gain);
    /// on: plays; off: the musician fades out, then the source stops
    void play(bool on);
    bool playing() const { return this->on; }
    goe::musician::adaptiveMusician &musician() { return this->performer; }

private:
    void fill(ALuint buffer);
    goe::musician::adaptiveMusician performer;
    ALuint source = 0;
    std::array<ALuint, bufferCount> buffers{};
    bool floatFormat = false;
    bool on = false;
    bool primed = false;
    int refillsSinceOff = 0; ///< buffers refilled since play(false), each quieter than the last
    int rate;
    std::vector<float> frames;   ///< interleaved stereo, made once
    std::vector<short> pcm;      ///< the same as 16 bit, when OpenAL takes no floats
};

#endif // PERFORMERSTREAM_H
