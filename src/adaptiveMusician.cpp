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
#include "adaptiveMusician.h"
#include <algorithm>
#include <cmath>

namespace goe::musician {

bool adaptiveMusician::initialize(const audioFormat &f, std::uint64_t seed, int voices)
{
    this->initialized = false;
    if (f.sampleRate < 8000 || f.sampleRate > 192000 || f.channels < 1 || f.channels > 2)
        return false;
    this->format = f;
    this->rate = (float) f.sampleRate;
    this->who = performerPersonality::generate(seed);
    this->feeling = tensionController(seed);
    this->writer = composer(seed);
    this->setlist = songbook(seed);
    this->band = synthesizer(this->rate, voices);
    this->styleNow = (chipStyle) this->styleWanted.load();
    this->sound = soundFor(this->styleNow, this->who);
    dress(this->band, this->sound);
    this->queue.clear();
    this->marks = {};
    this->nextMark = 0;
    this->now = this->composedUntil = this->advancedTo = 0;
    this->playing = situation::calm;
    this->composing = this->settled = false;
    this->phrases = 0;
    this->fade = 0.0f;
    this->gain = std::clamp(this->volumeWanted.load(), 0.0f, 1.0f);
    this->initialized = true;
    return true;
}

void adaptiveMusician::shutdown()
{
    this->initialized = false;
    this->queue.clear();
    this->band.silenceAll();
}

void adaptiveMusician::setDifficulty(int difficulty)
{
    this->difficultyWanted = std::clamp(difficulty, 0, tuning::maxDifficulty);
}

void adaptiveMusician::setSituation(situation s)
{
    this->situationWanted = std::clamp((int) s, 0, situationCount - 1);
}

void adaptiveMusician::setEnabled(bool enabled)
{
    this->enabledWanted = enabled;
}

void adaptiveMusician::setVolume(float volume)
{
    if (std::isfinite(volume))
        this->volumeWanted = std::clamp(volume, 0.0f, 1.0f);
}

void adaptiveMusician::setStyle(chipStyle style)
{
    this->styleWanted = std::clamp((int) style, 0, chipStyleCount - 1);
}

void adaptiveMusician::setVariety(float variety)
{
    if (std::isfinite(variety))
        this->varietyWanted = std::clamp(variety, 0.0f, 1.0f);
}

void adaptiveMusician::setTempoScale(float scale)
{
    if (std::isfinite(scale))
        this->tempoWanted = std::clamp(scale, tuning::minTempoScale, tuning::maxTempoScale);
}

void adaptiveMusician::restyle(chipStyle style)
{
    this->styleNow = style;
    this->sound = soundFor(style, this->who);
    // the old chip's notes fade with their release; the new one starts a phrase right away
    this->queue.cutAt(this->now);
    this->band.releaseAll();
    dress(this->band, this->sound);
    if (this->composing) {
        this->composedUntil = this->now;
        this->marks = {};
    }
}

void adaptiveMusician::pause()
{
    this->pausedWanted = true;
}

void adaptiveMusician::resume()
{
    this->pausedWanted = false;
}

timbreShift adaptiveMusician::shiftFor(const musicalState &s) const
{
    // the instrument itself tightens a little as the tension grows: brighter, wider, quicker
    const float t = s.timbre;
    return {1.0f + 0.35f * t, 1.0f + 0.5f * t, 1.0f + 0.6f * t, 1.0f - 0.3f * t, 1.0f + 0.5f * t};
}

void adaptiveMusician::follow(std::int64_t until)
{
    // in fixed steps, so the mood draws the same numbers however often the audio thread calls
    const std::int64_t quantum = std::max<std::int64_t>(1, (std::int64_t) this->rate / 50);
    while (this->advancedTo + quantum <= until) {
        this->feeling.advance((float) quantum / this->rate);
        this->advancedTo += quantum;
    }
}

void adaptiveMusician::escalate()
{
    // the new situation is heard from the next bar line, not after the rest of the phrase
    const std::int64_t soonest = this->now + (std::int64_t) (0.05f * this->rate);
    for (const auto &m : this->marks) {
        if (m.barLength <= 0 || soonest < m.start || soonest >= m.end)
            continue;
        const std::int64_t bars = (soonest - m.start + m.barLength - 1) / m.barLength;
        const std::int64_t cut = std::min(m.start + bars * m.barLength, m.end);
        if (cut < this->composedUntil) {
            this->queue.cutAt(cut);
            this->composedUntil = cut;
            for (auto &other : this->marks) // phrases after the cut are gone
                if (other.start >= cut)
                    other = {};
        }
        return;
    }
}

void adaptiveMusician::composeAhead()
{
    if (!this->initialized)
        return;
    const int difficulty = this->difficultyWanted;
    if (!this->settled) {
        this->feeling.settle(difficulty); // the first music starts where the game is
        this->settled = true;
    }
    this->feeling.setDifficulty(difficulty);

    if (!this->enabledWanted) {
        if (this->composing) {
            this->queue.clear();
            this->band.releaseAll();
            this->composing = false;
        }
        return;
    }
    if (!this->composing) {
        this->composing = true;
        this->composedUntil = this->now;
        this->marks = {};
    }
    if (const auto style = (chipStyle) this->styleWanted.load(); style != this->styleNow)
        this->restyle(style);
    const auto wanted = (situation) this->situationWanted.load();
    if ((int) wanted > (int) this->playing)
        this->escalate();
    const float variety = this->varietyWanted;
    const float tempoScale = this->tempoWanted;
    const auto lookahead = (std::int64_t) (tuning::lookaheadSeconds * this->rate);
    while (this->composedUntil < this->now + lookahead) {
        this->composedUntil = std::max(this->composedUntil, this->now);
        // the tension as it was when this phrase fell due, whatever the size of the audio buffers
        this->follow(this->composedUntil - lookahead);
        const musicalState state = this->feeling.state();
        this->band.setShift(this->shiftFor(state));
        phrasePlan plan = this->setlist.next(this->who, state, wanted, variety);
        plan.arpeggioChords = this->sound.arpeggioChords;
        const auto [first, last] = this->band.channelsOf(part::drums);
        plan.drumChannels = last - first;
        // the song decides the key, mode and tempo; the player's setting scales the tempo
        performerPersonality player = this->setlist.dressed(this->who);
        player.baseTempo *= tempoScale;
        this->writer.compose(player, state, wanted, this->rate, plan, this->scratch);
        if (this->scratch.length <= 0 || this->queue.room() < this->scratch.count)
            break; // the queue is full: try again next time, the music has a second planned
        for (int c = 0; c < this->scratch.count; c++) {
            noteEvent e = this->scratch.events[(std::size_t) c];
            e.at += this->composedUntil;
            this->queue.push(e);
        }
        this->marks[(std::size_t) this->nextMark] = {this->composedUntil, this->scratch.barLength,
                                                     this->composedUntil + this->scratch.length};
        this->nextMark = (this->nextMark + 1) % (int) this->marks.size();
        this->composedUntil += this->scratch.length;
        this->playing = wanted;
        this->phrases++;
    }
}

void adaptiveMusician::dispatch(const noteEvent &e)
{
    if (e.what == noteEvent::kind::on)
        this->band.noteOn(e);
    else
        this->band.noteOff(e);
}

void adaptiveMusician::renderAudio(float *output, std::uint32_t frames)
{
    if (output == nullptr)
        return;
    const int channels = this->format.channels;
    if (!this->initialized) {
        std::fill(output, output + (std::size_t) frames * (std::size_t) std::max(channels, 1), 0.0f);
        return;
    }
    const bool sounding = this->enabledWanted && !this->pausedWanted;
    const float target = sounding ? 1.0f : 0.0f;
    const float volume = this->volumeWanted;
    const float step = 1.0f / (tuning::fadeSeconds * this->rate);
    std::uint32_t written = 0;
    while (written < frames) {
        float *out = output + (std::size_t) written * (std::size_t) channels;
        const int n = (int) std::min<std::uint32_t>(frames - written, synthesizer::blockFrames);
        if (this->pausedWanted && this->fade <= 0.0f) {
            // paused: silence, and the music waits exactly where it stopped
            std::fill(out, out + (std::size_t) n * (std::size_t) channels, 0.0f);
            written += (std::uint32_t) n;
            continue;
        }
        // notes start and stop at their own sample, wherever that falls in the buffer
        int done = 0;
        while (done < n) {
            while (!this->queue.empty() && this->queue.top().at <= this->now + done)
                this->dispatch(this->queue.pop());
            int until = n;
            if (!this->queue.empty() && this->queue.top().at < this->now + n)
                until = (int) (this->queue.top().at - this->now);
            this->band.render(this->left.data() + done, this->right.data() + done, until - done);
            done = until;
        }
        for (int i = 0; i < n; i++) {
            this->fade = this->fade < target ? std::min(target, this->fade + step) : std::max(target, this->fade - step);
            this->gain += (volume - this->gain) * std::min(1.0f, step);
            const float g = this->fade * this->gain;
            const float l = this->left[(std::size_t) i] * g, r = this->right[(std::size_t) i] * g;
            if (channels == 2) {
                out[2 * i] = l;
                out[2 * i + 1] = r;
            } else {
                out[i] = 0.5f * (l + r);
            }
        }
        this->now += n;
        written += (std::uint32_t) n;
    }
}

} // namespace goe::musician
