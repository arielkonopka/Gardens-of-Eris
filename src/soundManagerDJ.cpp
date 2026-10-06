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
// The DJ's part of the sound manager (Config, Music: DJ): the deck work on OpenAL sources.
// What to play and how to mix it is decided in autoDJ.h; this file only carries it out.
#include "soundManager.h"
#include "randomStreams.h"
#include <filesystem>

namespace {
/// one round of the sound thread, so a mix point is met within a round
constexpr double roundSeconds = 0.012;
/// the new song is cued this much before its mix point, so a late round can still start it on time
constexpr double cueLead = 0.1;
using djClock = std::chrono::steady_clock;

double secondsBetween(djClock::time_point from, djClock::time_point to)
{
    return std::chrono::duration<double>(to - from).count();
}
} // namespace

int soundManager::djIndexOf(int song) const
{
    for (int k = 0; k < (int) this->difficultySongs.size(); k++)
        if (this->difficultySongs[k] == song)
            return k;
    return -1;
}

const goe::dj::trackInfo &soundManager::djInfo(int song) const
{
    static const goe::dj::trackInfo unknown;
    const int k = this->djIndexOf(song);
    return k >= 0 && k < (int) this->djSongs.size() ? this->djSongs[k].info : unknown;
}

double soundManager::songPosition(int song)
{
    const auto &m = this->registeredMusic[song];
    ALint offset = 0;
    alGetSourcei(m.source, AL_SAMPLE_OFFSET, &offset);
    const double rate = m.musFileinfo.samplerate > 0 ? m.musFileinfo.samplerate : 44100.0;
    const double seconds = (m.framesDone + offset) / rate;
    const double length = (double) m.musFileinfo.frames / rate;
    return length > 0.0 ? std::fmod(seconds, length) : seconds;
}

void soundManager::cueSong(int song, double seconds)
{
    auto &m = this->registeredMusic[song];
    alSourceStop(m.source);
    alSourcei(m.source, AL_BUFFER, 0); // takes every buffer off the stopped source
    const sf_count_t frame
        = std::clamp((sf_count_t) (seconds * m.musFileinfo.samplerate), (sf_count_t) 0, std::max<sf_count_t>(0, m.musFileinfo.frames - 1));
    sf_seek(m.musicFile.get(), frame, SEEK_SET);
    m.queuedFrames.clear();
    m.framesDone = (double) frame;
    for (ALuint buffer : {m.Abuffers[0], m.Abuffers[1], m.Abuffers[2]})
        if (!this->queueNextPiece(m, buffer))
            break;
}

void soundManager::playDJMusic()
{
    if (!this->djListener.joinable())
        this->djListener = std::jthread(&soundManager::listenToSongs, this, this->djFiles);
    const auto now = djClock::now();
    const bool threatened = this->situationNow.load() != (int) goe::musician::situation::calm;
    const int pick = this->djChoice.choose(this->difficultyNow, threatened, this->djMap.slots, now, goe::rng::audio());
    if (pick < 0)
        return;
    const int wanted = this->difficultySongs[pick];

    if (this->currentMusic < 0) {
        // nothing plays yet: the first song fades in from its first downbeat
        const auto &info = djInfo(wanted);
        this->cueSong(wanted, info.hasBeat() ? info.firstDownbeat : 0.0);
        alSourcef(this->registeredMusic[wanted].source, AL_PITCH, 1.0f);
        alSourcePlay(this->registeredMusic[wanted].source);
        this->currentMusic = wanted;
        this->djMixStart = now;
        this->djFade = goe::dj::plainMix;
        this->djPitch = 1.0f;
    } else if (wanted == this->currentMusic) {
        this->djPending = -1; // it changed its mind before the mix
    } else if (wanted != this->djPending) {
        if (wanted == this->fadingMusic) { // the song going out is wanted back: it comes in afresh
            alSourcePause(this->registeredMusic[wanted].source);
            this->fadingMusic = -1;
        }
        const double since = secondsBetween(this->djMixStart, now) - this->djFade;
        const goe::dj::deck playing{djInfo(this->currentMusic), this->songPosition(this->currentMusic), goe::dj::glide(this->djPitch, since)};
        const bool urgent = threatened && this->djMap.slots[pick].danger;
        this->djPlan = goe::dj::planMix(playing, djInfo(wanted), urgent);
        this->djPending = wanted;
        this->djMixAt = now + std::chrono::duration_cast<djClock::duration>(std::chrono::duration<double>(this->djPlan.wait));
        // decoded ahead, so the start costs nothing
        this->djCuedAt = std::max(0.0, this->djPlan.cueAt - cueLead);
        this->cueSong(wanted, this->djCuedAt);
    }

    if (this->djPending >= 0 && secondsBetween(now, this->djMixAt) <= roundSeconds) {
        auto &in = this->registeredMusic[this->djPending];
        // a round early or late is made up inside the cued buffers
        const double late = secondsBetween(this->djMixAt, now);
        const double startAt = std::max(this->djCuedAt, this->djPlan.cueAt + late * this->djPlan.pitch);
        const double queued = 2.0; // the three cued buffers hold over two seconds
        if (startAt - this->djCuedAt > queued) {
            this->cueSong(this->djPending, startAt);
        } else {
            alSourcei(in.source, AL_SAMPLE_OFFSET, (ALint) ((startAt - this->djCuedAt) * in.musFileinfo.samplerate));
        }
        alSourcef(in.source, AL_PITCH, this->djPlan.pitch);
        alSourcePlay(in.source);
        std::cout << "DJ: mixing into " << std::filesystem::path(this->gc->music[in.songNo].filename).filename().string()
                  << (this->djPlan.beatMatched ? ", beat-matched at speed " + std::to_string(this->djPlan.pitch) : ", not beat-matched")
                  << ", over " << this->djPlan.fade << " s\n";
        if (this->fadingMusic >= 0)
            alSourcePause(this->registeredMusic[this->fadingMusic].source);
        this->fadingMusic = this->currentMusic;
        this->currentMusic = this->djPending;
        this->djPending = -1;
        this->djMixStart = now;
        this->djFade = this->djPlan.fade;
        this->djPitch = this->djPlan.pitch;
    }

    const double since = secondsBetween(this->djMixStart, now);
    const auto [out, in] = goe::dj::crossfade(this->djFade > 0.0 ? since / this->djFade : 1.0);
    // every song at the list's usual gain, corrected to the average loudness: the DJ levels the songs
    // itself, so a gain set low for one loud song in skins.json would make it too quiet here
    auto level = [this](int song) {
        const int k = this->djIndexOf(song);
        const float loudness = k >= 0 && k < (int) this->djMap.gains.size() ? this->djMap.gains[k] : 1.0f;
        const float own = this->registeredMusic[song].gain;
        return own > 0.0f ? loudness * this->djGain / own : loudness;
    };
    alSourcef(this->registeredMusic[this->currentMusic].source, AL_PITCH, goe::dj::glide(this->djPitch, since - this->djFade));
    this->playSong(this->currentMusic, in * level(this->currentMusic));
    if (this->fadingMusic < 0)
        return;
    if (since >= this->djFade) {
        alSourcePause(this->registeredMusic[this->fadingMusic].source);
        alSourcef(this->registeredMusic[this->fadingMusic].source, AL_PITCH, 1.0f);
        this->fadingMusic = -1;
    } else {
        this->playSong(this->fadingMusic, out * level(this->fadingMusic));
    }
}

void soundManager::listenToSongs(std::stop_token stop, std::vector<std::string> files)
{
    std::string folder = gameSettings::getInstance().getSaveDirectory();
    if (folder.empty())
        folder = ".";
    goe::dj::analysisCache cache((std::filesystem::path(folder) / "musicmap.json").string());
    std::vector<goe::dj::trackInfo> heard(files.size());
    bool learned = false;
    for (std::size_t k = 0; k < files.size(); k++) {
        if (stop.stop_requested())
            return;
        auto info = cache.find(files[k]);
        if (!info) {
            info = goe::dj::analyseFile(files[k], stop);
            if (!info)
                continue; // unreadable (or stopping): it keeps its place in the list
            cache.put(files[k], *info);
            learned = true;
        }
        heard[k] = *info;
    }
    if (learned && !cache.save())
        std::cout << "DJ: could not keep the song analysis in " << folder << "\n";
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    for (std::size_t k = 0; k < heard.size() && k < this->djSongs.size(); k++)
        this->djSongs[k].info = heard[k];
    this->djMap = goe::dj::mapSongs(this->djSongs);
    for (std::size_t k = 0; k < files.size() && k < this->djMap.slots.size(); k++) {
        const auto &i = this->djSongs[k].info;
        std::cout << "DJ: " << std::filesystem::path(files[k]).filename().string() << ": " << i.bpm << " bpm, "
                  << (this->djMap.slots[k].danger ? std::string("danger") : "level " + std::to_string(this->djMap.slots[k].level))
                  << "\n";
    }
}
