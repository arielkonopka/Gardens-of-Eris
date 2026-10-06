// The DJ without the game: listens to songs and shows how it maps them, or plays a mix of them
// into a WAV file, the way the game mixes them (autoDJ.h). Usage:
//   goe-dj song.mp3 ...                              the tempo, beat, loudness and level of each song
//   goe-dj --mix out.wav [seconds=40] song.mp3 ...   each song plays about that long, then mixes into the next
//   (--danger before the songs makes every second mix a danger cut)
#include "autoDJ.h"
#include "musicAnalysis.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sndfile.h>
#include <string>
#include <vector>

namespace {
struct loaded
{
    std::vector<float> stereo; ///< interleaved left, right
    int rate = 44100;
    double frames() const { return (double) this->stereo.size() / 2.0; }
};

bool load(const std::string &path, loaded &out)
{
    SF_INFO info{};
    std::unique_ptr<SNDFILE, int (*)(SNDFILE *)> f(sf_open(path.c_str(), SFM_READ, &info), sf_close);
    if (!f || info.channels < 1)
        return false;
    std::vector<float> raw((std::size_t) info.frames * info.channels);
    const sf_count_t got = sf_readf_float(f.get(), raw.data(), info.frames);
    out.rate = info.samplerate;
    out.stereo.resize((std::size_t) got * 2);
    for (sf_count_t i = 0; i < got; i++) {
        const float l = raw[(std::size_t) (i * info.channels)];
        const float r = info.channels > 1 ? raw[(std::size_t) (i * info.channels + 1)] : l;
        out.stereo[(std::size_t) i * 2] = l;
        out.stereo[(std::size_t) i * 2 + 1] = r;
    }
    return got > 0;
}

/// a song on a deck: its sound, where it is (in its frames) and how loud it is
struct player
{
    const loaded *song = nullptr;
    double at = 0.0;
    float level = 1.0f;
    void add(float &l, float &r, float gain, float pitch)
    {
        const double n = this->song->frames();
        const auto i = (std::size_t) this->at;
        const auto j = (i + 1) % (std::size_t) n;
        const float f = (float) (this->at - (double) i);
        l += gain * this->level * (this->song->stereo[i * 2] * (1 - f) + this->song->stereo[j * 2] * f);
        r += gain * this->level * (this->song->stereo[i * 2 + 1] * (1 - f) + this->song->stereo[j * 2 + 1] * f);
        this->at = std::fmod(this->at + pitch, n);
    }
};
} // namespace

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "usage: goe-dj song ... | goe-dj --mix out.wav [seconds] [--danger] song ...\n";
        return 1;
    }
    std::string out;
    double hold = 40.0;
    bool danger = false;
    int first = 1;
    if (std::strcmp(argv[1], "--mix") == 0) {
        if (argc < 4)
            return 1;
        out = argv[2];
        first = 3;
        char *end = nullptr;
        const double s = std::strtod(argv[3], &end);
        if (end && *end == '\0' && s > 0.0) {
            hold = s;
            first = 4;
        }
    }
    if (first < argc && std::strcmp(argv[first], "--danger") == 0) {
        danger = true;
        first++;
    }
    std::vector<std::string> files(argv + first, argv + argc);
    std::vector<goe::dj::songEntry> songs;
    for (const auto &f : files) {
        const auto info = goe::dj::analyseFile(f);
        if (!info)
            std::cerr << "cannot read " << f << "\n";
        songs.push_back({info.value_or(goe::dj::trackInfo{}), false});
    }
    const auto map = goe::dj::mapSongs(songs);
    for (std::size_t k = 0; k < files.size(); k++) {
        const auto &i = songs[k].info;
        std::printf("%-48.48s %6.1f s %6.2f bpm  beat %.2f  downbeat %6.3f s  %5.1f dB  flux %5.2f  %5.0f Hz  %-8s gain %.2f\n",
                    std::filesystem::path(files[k]).filename().string().c_str(), i.seconds, i.bpm, i.beatStrength,
                    i.firstDownbeat, i.loudnessDb, i.flux, i.brightness,
                    map.slots[k].danger ? "danger" : ("level " + std::to_string(map.slots[k].level)).c_str(),
                    map.gains[k]);
    }
    if (out.empty())
        return 0;

    std::vector<loaded> sound(files.size());
    for (std::size_t k = 0; k < files.size(); k++)
        if (!load(files[k], sound[k]))
            return 2;
    const int rate = sound[0].rate;
    SF_INFO wav{};
    wav.samplerate = rate;
    wav.channels = 2;
    wav.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    std::unique_ptr<SNDFILE, int (*)(SNDFILE *)> o(sf_open(out.c_str(), SFM_WRITE, &wav), sf_close);
    if (!o)
        return 3;
    std::vector<float> block;
    auto emit = [&](float l, float r) {
        block.push_back(std::clamp(l, -1.0f, 1.0f));
        block.push_back(std::clamp(r, -1.0f, 1.0f));
        if (block.size() >= 8192) {
            sf_writef_float(o.get(), block.data(), (sf_count_t) block.size() / 2);
            block.clear();
        }
    };

    // the first song comes in at its first downbeat; then every song plays about `hold` and mixes on
    const double rateRatio = 1.0; // all songs at one rate, as in the game's data
    player playing{&sound[0], songs[0].info.firstDownbeat * rate, map.gains[0]};
    float pitchFrom = 1.0f;
    double sinceIn = 1e9; // seconds since the playing song came in
    double fade = 0.0;
    for (std::size_t next = 1; next <= files.size(); next++) {
        const std::size_t k = next % files.size();
        // the hold, during which the last mix finishes and the speed glides back
        for (double t = 0.0; t < hold; t += 1.0 / rate, sinceIn += 1.0 / rate) {
            float l = 0, r = 0;
            playing.add(l, r, 1.0f, (float) (goe::dj::glide(pitchFrom, sinceIn - fade) * rateRatio));
            emit(l, r);
        }
        if (next == files.size())
            break;
        const float pitchNow = goe::dj::glide(pitchFrom, sinceIn - fade);
        const bool urgent = danger && next % 2 == 0;
        const auto plan = goe::dj::planMix({songs[(next - 1) % files.size()].info, playing.at / rate, pitchNow}, songs[k].info, urgent);
        std::printf("mix %zu -> %zu: wait %.2f s, %s, pitch %.3f, fade %.2f s%s\n", next - 1, k, plan.wait,
                    plan.beatMatched ? "beat-matched" : "not matched", plan.pitch, plan.fade, urgent ? ", danger" : "");
        for (double t = 0.0; t < plan.wait; t += 1.0 / rate) {
            float l = 0, r = 0;
            playing.add(l, r, 1.0f, pitchNow);
            emit(l, r);
        }
        player incoming{&sound[k], plan.cueAt * rate, map.gains[k]};
        for (double t = 0.0; t < plan.fade; t += 1.0 / rate) {
            const auto [g0, g1] = goe::dj::crossfade(t / plan.fade);
            float l = 0, r = 0;
            playing.add(l, r, g0, pitchNow);
            incoming.add(l, r, g1, plan.pitch);
            emit(l, r);
        }
        playing = incoming;
        pitchFrom = plan.pitch;
        fade = 0.0;
        sinceIn = 0.0;
    }
    if (!block.empty())
        sf_writef_float(o.get(), block.data(), (sf_count_t) block.size() / 2);
    return 0;
}
