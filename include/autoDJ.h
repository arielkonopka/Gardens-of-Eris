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
#ifndef AUTODJ_H
#define AUTODJ_H

#include "difficultyMusic.h"
#include "musicAnalysis.h"
#include <map>
#include <string>
#include <vector>

/**
 * @brief The DJ: the third way of making music (Config, Music: DJ).
 *
 * It listens to every song of the skin's music list once (musicAnalysis.h), then
 * - maps them: the songs are ranked by how intense they sound (loudness, flux, brightness, tempo)
 *   and spread over the difficulty levels 0..topLevel, the calmest first ("Difficulty" in skins.json
 *   is the Skin samples' own map and is not used). The "Play": "danger" songs stay danger songs;
 *   with none, the most intense song becomes the danger song;
 * - levels them: every song is played as loud as the average one (its skins.json gain still applies);
 * - mixes them: a new song starts on a bar of the playing one (on the next eight-bar phrase when it
 *   comes soon), at its own first downbeat past a quiet intro, with its speed pitched to the playing song's tempo when
 *   they are within 8% (or double or half of it), and an equal-power crossfade over a few bars. After
 *   the mix its speed glides back to its own. Danger cuts in on the next beat with a one-bar fade.
 * No game header is included; the sound thread does the playing.
 */
namespace goe::dj {

/// the highest difficulty level the DJ spreads the songs over
inline constexpr int topLevel = 10;
/// the farthest a song's speed is bent to match the playing one, as a fraction
inline constexpr float maxPitchBend = 0.08f;
/// a phrase is eight bars of four beats; the mix waits for it only this long, else takes the next bar
inline constexpr int beatsPerBar = 4;
inline constexpr int barsPerPhrase = 8;
inline constexpr double phraseWaitSeconds = 12.0;
/// a crossfade lasts four bars of the playing song, within these bounds
inline constexpr int mixBars = 4;
inline constexpr double shortestMix = 3.0;
inline constexpr double longestMix = 10.0;
/// without a beat to mix on, a plain crossfade of this length
inline constexpr double plainMix = 5.0;
/// after the mix the new song's speed glides back to its own over this long
inline constexpr double glideSeconds = 16.0;
/// a song's level correction is kept within these gains
inline constexpr float quietestGain = 0.5f;
inline constexpr float loudestGain = 2.0f;

/// one song of the music list as the DJ sees it
struct songEntry
{
    trackInfo info;      ///< not analysed yet (or unreadable): it keeps its place in the list
    bool danger = false; ///< "Play": "danger" from skins.json
};

struct mapping
{
    std::vector<goe::music::songSlot> slots; ///< the level and role of every song, in list order
    std::vector<float> gains;                ///< the factor that plays every song as loud as the average one
};

/// the levels, roles and gains of the songs
mapping mapSongs(const std::vector<songEntry> &songs);

/// a song as it plays: where it is (seconds into the song) and how fast it runs
struct deck
{
    trackInfo info;
    double position = 0.0;
    float pitch = 1.0f;
};

/// how a new song comes in
struct mixPlan
{
    double wait = 0.0;   ///< wall seconds until the new song starts
    double cueAt = 0.0;  ///< where in the new song it starts, in seconds
    float pitch = 1.0f;  ///< the new song's speed during the mix
    double fade = plainMix; ///< wall seconds of the crossfade
    bool beatMatched = false;
};

/// the mix from the playing song into the next one; urgent (danger) takes the next beat and a short fade
mixPlan planMix(const deck &playing, const trackInfo &next, bool urgent);

/// the gains of the outgoing and incoming songs at x (0..1) through an equal-power crossfade
std::pair<float, float> crossfade(double x);

/// the speed of a song that started at pitch from, after `since` seconds of its glide back to 1
float glide(float from, double since);

/// remembered analyses, so the DJ listens to a song only once: a JSON file keyed by file name,
/// a file whose size or time changed is listened to again
class analysisCache
{
public:
    explicit analysisCache(std::string path);
    std::optional<trackInfo> find(const std::string &file) const;
    void put(const std::string &file, const trackInfo &info);
    bool save() const;

private:
    struct entry
    {
        long long size = 0;
        long long time = 0;
        trackInfo info;
    };
    std::string path;
    std::map<std::string, entry> entries;
    static bool stamp(const std::string &file, long long &size, long long &time);
};

} // namespace goe::dj

#endif // AUTODJ_H
