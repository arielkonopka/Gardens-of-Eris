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
#include "performerStream.h"
#include <AL/alext.h>
#include <algorithm>
#include <cmath>

performerStream::performerStream(int sampleRate, std::uint64_t seed)
    : rate(sampleRate)
    , frames((std::size_t) bufferFrames * 2)
    , pcm((std::size_t) bufferFrames * 2)
{
    this->performer.initialize({sampleRate, 2}, seed);
    this->performer.setEnabled(false);
    alGetError();
    alGenSources(1, &this->source);
    alGenBuffers(bufferCount, this->buffers.data());
    if (alGetError() != AL_NO_ERROR) {
        // no OpenAL objects: the musician stays silent and nothing else in the game's audio changes
        this->source = 0;
        return;
    }
    // heard where the listener is, like the difficulty music: a stereo stream is never placed in space
    alSourcei(this->source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(this->source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSourcef(this->source, AL_ROLLOFF_FACTOR, 0.0f);
    alSourcei(this->source, AL_LOOPING, AL_FALSE);
    this->floatFormat = alIsExtensionPresent("AL_EXT_FLOAT32") == AL_TRUE;
}

performerStream::~performerStream()
{
    this->performer.shutdown();
    if (this->source == 0)
        return;
    alSourceStop(this->source);
    alSourcei(this->source, AL_BUFFER, 0);
    alDeleteSources(1, &this->source);
    alDeleteBuffers(bufferCount, this->buffers.data());
}

void performerStream::fill(ALuint buffer)
{
    this->performer.composeAhead();
    this->performer.renderAudio(this->frames.data(), (std::uint32_t) bufferFrames);
    const auto bytes = (ALsizei) (bufferFrames * 2);
    if (this->floatFormat) {
        alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32, this->frames.data(), bytes * (ALsizei) sizeof(float), this->rate);
        return;
    }
    std::transform(this->frames.begin(), this->frames.end(), this->pcm.begin(), [](float s) {
        return (short) std::lround(std::clamp(s, -1.0f, 1.0f) * 32767.0f);
    });
    alBufferData(buffer, AL_FORMAT_STEREO16, this->pcm.data(), bytes * (ALsizei) sizeof(short), this->rate);
}

void performerStream::play(bool playOn)
{
    this->on = playOn;
    this->refillsSinceOff = 0;
    this->performer.setEnabled(playOn);
}

void performerStream::pump(int difficulty, goe::musician::situation now, float gain)
{
    if (this->source == 0)
        return;
    this->performer.setDifficulty(difficulty);
    this->performer.setSituation(now);
    alSourcef(this->source, AL_GAIN, std::clamp(gain, 0.0f, 1.0f));
    ALint state = AL_STOPPED;
    alGetSourcei(this->source, AL_SOURCE_STATE, &state);
    if (!this->on && !this->primed)
        return;
    if (!this->on && this->refillsSinceOff > bufferCount) {
        // the musician's fade out has played and only silence is queued: the source can stop
        alSourceStop(this->source);
        alSourcei(this->source, AL_BUFFER, 0);
        this->primed = false;
        return;
    }
    if (!this->primed) {
        alSourceStop(this->source);
        alSourcei(this->source, AL_BUFFER, 0);
        for (ALuint b : this->buffers)
            this->fill(b);
        alSourceQueueBuffers(this->source, bufferCount, this->buffers.data());
        alSourcePlay(this->source);
        this->primed = true;
        return;
    }
    ALint done = 0;
    alGetSourcei(this->source, AL_BUFFERS_PROCESSED, &done);
    while (done-- > 0) {
        ALuint b = 0;
        alSourceUnqueueBuffers(this->source, 1, &b);
        this->fill(b);
        alSourceQueueBuffers(this->source, 1, &b);
        if (!this->on)
            this->refillsSinceOff++;
    }
    if (state != AL_PLAYING) // it ran dry: start again from what is queued
        alSourcePlay(this->source);
}
