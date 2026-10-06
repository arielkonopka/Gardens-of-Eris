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
#include "autoDJ.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <numbers>
#include <numeric>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>

namespace goe::dj {

namespace {
/// how much each measure counts in a song's intensity
constexpr float loudnessWeight = 0.30f;
constexpr float fluxWeight = 0.35f;
constexpr float brightnessWeight = 0.15f;
constexpr float tempoWeight = 0.20f;
/// the new song needs a moment to be cued before the mix point, in seconds of the playing song
constexpr double cueMargin = 0.05;

/// each analysed song's rank by a measure, 0 (lowest) to 1 (highest); 0.5 when it cannot be ranked
std::vector<float> ranks(const std::vector<songEntry> &songs, float (*measure)(const trackInfo &))
{
    std::vector<int> order;
    for (int c = 0; c < (int) songs.size(); c++)
        if (songs[c].info.analysed)
            order.push_back(c);
    std::vector<float> out(songs.size(), 0.5f);
    if (order.size() < 2)
        return out;
    std::ranges::stable_sort(order, [&](int a, int b) { return measure(songs[a].info) < measure(songs[b].info); });
    for (std::size_t r = 0; r < order.size(); r++)
        out[order[r]] = (float) r / (float) (order.size() - 1);
    return out;
}
} // namespace

mapping mapSongs(const std::vector<songEntry> &songs)
{
    const int n = (int) songs.size();
    mapping m;
    m.slots.resize(n);
    m.gains.resize(n);
    const auto loud = ranks(songs, [](const trackInfo &i) { return i.loudnessDb; });
    const auto flux = ranks(songs, [](const trackInfo &i) { return i.flux; });
    const auto bright = ranks(songs, [](const trackInfo &i) { return i.brightness; });
    const auto tempo = ranks(songs, [](const trackInfo &i) { return i.bpm; });
    std::vector<float> intensity(n);
    int analysed = 0;
    double loudnessSum = 0.0;
    for (int c = 0; c < n; c++) {
        if (songs[c].info.analysed) {
            intensity[c] = loudnessWeight * loud[c] + fluxWeight * flux[c] + brightnessWeight * bright[c]
                           + tempoWeight * tempo[c];
            analysed++;
            loudnessSum += songs[c].info.loudnessDb;
        } else {
            intensity[c] = n > 1 ? (float) c / (float) (n - 1) : 0.0f; // its place in the list
        }
    }

    // the roles: the marked danger songs, or the most intense free song when none is marked
    std::vector<bool> danger(n);
    for (int c = 0; c < n; c++)
        danger[c] = songs[c].danger;
    if (std::ranges::none_of(danger, [](bool d) { return d; }) && analysed >= 2) {
        int loudest = -1;
        for (int c = 0; c < n; c++)
            if (songs[c].info.analysed && (loudest < 0 || intensity[c] > intensity[loudest]))
                loudest = c;
        if (loudest >= 0)
            danger[loudest] = true;
    }

    // the free songs spread over the levels, the calmest first
    std::vector<int> free;
    for (int c = 0; c < n; c++)
        if (!danger[c])
            free.push_back(c);
    std::ranges::stable_sort(free, [&](int a, int b) { return intensity[a] < intensity[b]; });
    for (std::size_t r = 0; r < free.size(); r++)
        m.slots[free[r]].level = free.size() > 1 ? (int) std::lround((double) r * topLevel / (double) (free.size() - 1)) : 0;
    for (int c = 0; c < n; c++)
        m.slots[c].danger = danger[c];

    // every song as loud as the average one
    const double meanDb = analysed > 0 ? loudnessSum / analysed : 0.0;
    for (int c = 0; c < n; c++) {
        float level = 1.0f;
        if (songs[c].info.analysed)
            level = std::clamp((float) std::pow(10.0, (meanDb - songs[c].info.loudnessDb) / 20.0), quietestGain, loudestGain);
        m.gains[c] = level;
    }
    return m;
}

mixPlan planMix(const deck &playing, const trackInfo &next, bool urgent)
{
    mixPlan p;
    p.cueAt = next.entry; // past a quiet intro, on a downbeat when it has a beat
    const trackInfo &out = playing.info;
    if (!out.hasBeat() || !next.hasBeat()) {
        p.fade = urgent ? shortestMix : plainMix;
        return p;
    }
    const float pitch = playing.pitch > 0.0f ? playing.pitch : 1.0f;
    const double beat = out.beatSeconds();
    const double pos = out.seconds > 0.0 ? std::fmod(std::max(0.0, playing.position), out.seconds) : playing.position;
    // the next boundary of `beats` beats on the playing song's downbeat grid
    auto boundary = [&](int beats) {
        const double unit = beats * beat;
        return out.firstDownbeat + std::ceil((pos + cueMargin * pitch - out.firstDownbeat) / unit) * unit;
    };
    double target = boundary(urgent ? 1 : beatsPerBar);
    if (!urgent) {
        const double phrase = boundary(beatsPerBar * barsPerPhrase);
        if ((phrase - pos) / pitch <= phraseWaitSeconds)
            target = phrase;
    }
    // past the end the song starts again, and its grid with it
    if (out.seconds > 0.0 && target > out.seconds - cueMargin)
        target = out.seconds + out.firstDownbeat;
    p.wait = (target - pos) / pitch;

    // the new song's speed: the playing tempo, or double or half of it, when that is close enough
    const double playingBpm = out.bpm * pitch;
    double best = 0.0;
    for (double times : {0.5, 1.0, 2.0}) {
        const double r = playingBpm / (next.bpm * times);
        if (best == 0.0 || std::abs(r - 1.0) < std::abs(best - 1.0))
            best = r;
    }
    p.beatMatched = std::abs(best - 1.0) <= maxPitchBend;
    p.pitch = p.beatMatched ? (float) best : 1.0f;
    const double bar = beatsPerBar * beat / pitch;
    if (urgent)
        p.fade = std::clamp(bar, 1.0, shortestMix);
    else if (p.beatMatched)
        p.fade = std::clamp(mixBars * bar, shortestMix, longestMix);
    else
        p.fade = std::clamp(2.0 * bar, 2.0, 4.0); // beats that do not match should not overlap long
    return p;
}

std::pair<float, float> crossfade(double x)
{
    const double t = std::clamp(x, 0.0, 1.0) * std::numbers::pi / 2.0;
    return {(float) std::cos(t), (float) std::sin(t)};
}

float glide(float from, double since)
{
    if (since <= 0.0)
        return from;
    const float t = (float) std::min(1.0, since / glideSeconds);
    return from + (1.0f - from) * t;
}

analysisCache::analysisCache(std::string file) : path(std::move(file))
{
    std::ifstream in(this->path, std::ios::binary);
    if (!in)
        return;
    std::stringstream text;
    text << in.rdbuf();
    rapidjson::Document doc;
    doc.Parse(text.str().c_str());
    if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember("version") || !doc["version"].IsInt()
        || doc["version"].GetInt() != analysisVersion || !doc.HasMember("songs") || !doc["songs"].IsObject())
        return;
    for (const auto &m : doc["songs"].GetObject()) {
        const auto &v = m.value;
        if (!v.IsObject())
            continue;
        auto number = [&v](const char *key) { return v.HasMember(key) && v[key].IsNumber() ? v[key].GetDouble() : 0.0; };
        entry e;
        e.size = (long long) number("size");
        e.time = (long long) number("time");
        e.info.analysed = true;
        e.info.seconds = number("seconds");
        e.info.bpm = (float) number("bpm");
        e.info.beatStrength = (float) number("beatStrength");
        e.info.firstBeat = number("firstBeat");
        e.info.firstDownbeat = number("firstDownbeat");
        e.info.entry = number("entry");
        e.info.loudnessDb = (float) number("loudnessDb");
        e.info.flux = (float) number("flux");
        e.info.brightness = (float) number("brightness");
        this->entries[m.name.GetString()] = e;
    }
}

bool analysisCache::stamp(const std::string &file, long long &size, long long &time)
{
    std::error_code ec;
    const auto s = std::filesystem::file_size(file, ec);
    if (ec)
        return false;
    const auto t = std::filesystem::last_write_time(file, ec);
    if (ec)
        return false;
    size = (long long) s;
    time = (long long) std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
    return true;
}

std::optional<trackInfo> analysisCache::find(const std::string &file) const
{
    const auto it = this->entries.find(file);
    long long size = 0, time = 0;
    if (it == this->entries.end() || !stamp(file, size, time) || size != it->second.size || time != it->second.time)
        return std::nullopt;
    return it->second.info;
}

void analysisCache::put(const std::string &file, const trackInfo &info)
{
    entry e;
    if (!stamp(file, e.size, e.time))
        return;
    e.info = info;
    this->entries[file] = e;
}

bool analysisCache::save() const
{
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
    w.StartObject();
    w.Key("version");
    w.Int(analysisVersion);
    w.Key("songs");
    w.StartObject();
    for (const auto &[file, e] : this->entries) {
        w.Key(file.c_str());
        w.StartObject();
        w.Key("size");
        w.Int64(e.size);
        w.Key("time");
        w.Int64(e.time);
        w.Key("seconds");
        w.Double(e.info.seconds);
        w.Key("bpm");
        w.Double(e.info.bpm);
        w.Key("beatStrength");
        w.Double(e.info.beatStrength);
        w.Key("firstBeat");
        w.Double(e.info.firstBeat);
        w.Key("firstDownbeat");
        w.Double(e.info.firstDownbeat);
        w.Key("entry");
        w.Double(e.info.entry);
        w.Key("loudnessDb");
        w.Double(e.info.loudnessDb);
        w.Key("flux");
        w.Double(e.info.flux);
        w.Key("brightness");
        w.Double(e.info.brightness);
        w.EndObject();
    }
    w.EndObject();
    w.EndObject();
    const std::string tmp = this->path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc | std::ios::binary);
        if (!out)
            return false;
        out << sb.GetString() << "\n";
        if (!out)
            return false;
    }
    std::error_code ec;
    std::filesystem::rename(tmp, this->path, ec);
    return !ec;
}

} // namespace goe::dj
