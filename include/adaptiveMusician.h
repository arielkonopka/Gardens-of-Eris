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
#ifndef ADAPTIVEMUSICIAN_H
#define ADAPTIVEMUSICIAN_H

#include "musicComposer.h"
#include "musicEvents.h"
#include "musicPersonality.h"
#include "musicSynth.h"
#include "musicTension.h"
#include <array>
#include <atomic>
#include <cstdint>

/**
 * @brief A small autonomous musician: it composes and plays music that follows the game.
 *
 * The musician owns the music; the game owns the audio system. It renders stereo frames into
 * a buffer it is given and never touches an audio device. docs/adaptive-musician.md has the
 * full design.
 *
 * Threads:
 * - the control functions (setDifficulty, setSituation, setEnabled, setVolume, pause, resume)
 *   may be called from any thread at any time; they only store atomics;
 * - composeAhead() and renderAudio() belong to the one thread that feeds the audio engine
 *   (the sound thread); renderAudio() never allocates, locks, logs or touches files;
 * - initialize() and shutdown() are called while nothing renders.
 */
namespace goe::musician {

/// what crosses the boundary: interleaved float frames at the engine's rate
struct audioFormat
{
    int sampleRate = 44100; ///< 8000..192000
    int channels = 2;       ///< 1 (left and right mixed) or 2 (left, right)
};

class adaptiveMusician
{
public:
    adaptiveMusician() = default;

    /// a new performer from the seed (the same seed, the same performer); false, and silence
    /// from then on, when the format cannot be served
    bool initialize(const audioFormat &format, std::uint64_t seed, int voices = tuning::defaultVoices);
    /// lets go of the music: from then on renderAudio writes silence until initialize again
    void shutdown();
    bool ready() const { return this->initialized; }

    // --- control, from any thread ---
    /// the game's difficulty, clamped to 0..256; the music follows gradually
    void setDifficulty(int difficulty);
    /// calm, alert (a camera or guardian is onto the player) or danger (a guardian is close):
    /// changes the theme, not the tension. A rise is heard from the next bar, a fall from the next phrase.
    void setSituation(situation s);
    /// off: the musician fades out and stops composing; on again: it starts a new phrase
    void setEnabled(bool enabled);
    /// the musician's own gain, 0..1; the game's music volume is applied after it, by the game
    void setVolume(float volume);
    /// fades out and holds the music where it is; resume fades back in and goes on from there
    void pause();
    void resume();

    // --- the audio thread ---
    /// composes until the music is planned tuning::lookaheadSeconds ahead; call before renderAudio
    void composeAhead();
    /// writes frames of audio (frames * channels floats, -1..1); real-time safe
    void renderAudio(float *output, std::uint32_t frames);

    // --- for tests and tuning; read on the audio thread or while nothing renders ---
    const performerPersonality &personality() const { return this->who; }
    const tensionController &tension() const { return this->feeling; }
    const phraseReport &lastPhrase() const { return this->writer.report(); }
    const synthesizer &synth() const { return this->band; }
    /// samples played since initialize
    std::int64_t position() const { return this->now; }
    int queued() const { return this->queue.size(); }
    /// phrases composed since initialize
    int phrasesComposed() const { return this->phrases; }

private:
    /// where a composed phrase lies, for cutting it at a bar when the situation rises
    struct phraseMark
    {
        std::int64_t start = 0, barLength = 0, end = 0;
    };
    void escalate();
    /// moves the tension on in audio time up to the sample `until`
    void follow(std::int64_t until);
    void dispatch(const noteEvent &e);
    timbreShift shiftFor(const musicalState &s) const;

    std::atomic<int> difficultyWanted{0};
    std::atomic<int> situationWanted{0};
    std::atomic<bool> enabledWanted{true};
    std::atomic<bool> pausedWanted{false};
    std::atomic<float> volumeWanted{1.0f};

    audioFormat format;
    bool initialized = false;
    float rate = 44100.0f;
    performerPersonality who;
    tensionController feeling;
    composer writer;
    synthesizer band;
    eventQueue queue;
    phraseBuffer scratch;
    std::array<phraseMark, 4> marks{};
    int nextMark = 0;
    std::int64_t now = 0;           ///< the next sample to render
    std::int64_t composedUntil = 0; ///< music is planned up to here
    std::int64_t advancedTo = 0;    ///< the tension has followed the music up to here
    situation playing = situation::calm; ///< the situation the latest phrase was written for
    bool composing = false;
    bool settled = false;
    int phrases = 0;
    float fade = 0.0f;   ///< pause and enable fade, 0..1
    float gain = 1.0f;   ///< the smoothed volume
    std::array<float, synthesizer::blockFrames> left{}, right{};
};

} // namespace goe::musician

#endif // ADAPTIVEMUSICIAN_H
