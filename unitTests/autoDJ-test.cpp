/*
 * The DJ: listening to songs, mapping them to the difficulty, and planning beat-matched mixes.
 */
#include "autoDJ.h"
#include "musicAnalysis.h"
#include <gtest/gtest.h>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <random>
#include <sndfile.h>

namespace fs = std::filesystem;

namespace {
constexpr int rate = 11025;

/// a minute of noise with a drum hit on every beat, the first of every bar the loudest
std::vector<float> beatTrack(double bpm, double firstBeat, int seconds = 60, float noise = 0.02f)
{
    std::mt19937 g(5);
    std::normal_distribution<float> n(0.0f, noise);
    std::vector<float> s((std::size_t) rate * seconds);
    for (auto &x : s)
        x = n(g);
    int k = 0;
    for (double t = firstBeat; t < seconds; t += 60.0 / bpm, k++) {
        const auto at = (std::size_t) (t * rate);
        const float amp = k % 4 == 0 ? 0.9f : 0.5f;
        for (std::size_t j = 0; j < 300 && at + j < s.size(); j++)
            s[at + j] += amp * std::exp(-(float) j / 60.0f) * std::sin((float) j * 0.3f);
    }
    return s;
}

goe::dj::trackInfo beatInfo(float bpm, double firstDownbeat, double seconds = 180.0)
{
    goe::dj::trackInfo i;
    i.analysed = true;
    i.bpm = bpm;
    i.beatStrength = 0.8f;
    i.firstBeat = i.firstDownbeat = i.entry = firstDownbeat;
    i.seconds = seconds;
    return i;
}

fs::path scratchFile(const std::string &ext)
{
    return fs::temp_directory_path() / ("goe-dj-test-" + std::to_string(std::random_device{}()) + ext);
}
} // namespace

TEST(AutoDJTests, HearsTheTempoBeatAndDownbeatOfADrumTrack)
{
    for (double bpm : {96.0, 128.0, 150.0, 174.0}) {
        const double beat = 60.0 / bpm;
        const auto info = goe::dj::analyse(beatTrack(bpm, 0.31 + beat), rate); // a beat before the first downbeat
        EXPECT_TRUE(info.hasBeat()) << bpm;
        EXPECT_NEAR(info.bpm, bpm, 0.5) << bpm;
        EXPECT_NEAR(std::fmod(info.firstBeat, beat), std::fmod(0.31 + beat, beat), 0.02) << bpm;
        // the loud hit comes a beat later than the first one, so the downbeat is there
        EXPECT_NEAR(info.firstDownbeat, 0.31 + beat, 0.02) << bpm;
        EXPECT_NEAR(info.seconds, 60.0, 0.01);
    }
}

TEST(AutoDJTests, ComesInPastAQuietIntroOnADownbeat)
{
    auto track = beatTrack(120.0, 0.25);
    for (std::size_t i = 0; i < (std::size_t) rate * 10; i++)
        track[i] *= 0.05f; // ten quiet seconds
    const auto info = goe::dj::analyse(track, rate);
    ASSERT_TRUE(info.hasBeat());
    EXPECT_GE(info.entry, 10.0 - 0.02);
    EXPECT_LT(info.entry, 12.0);
    const double bars = (info.entry - info.firstDownbeat) / (4.0 * info.beatSeconds());
    EXPECT_NEAR(bars, std::round(bars), 1e-6);
    // without an intro it comes in at once
    EXPECT_NEAR(goe::dj::analyse(beatTrack(120.0, 0.25), rate).entry, 0.25, 0.02);
}

TEST(AutoDJTests, NoiseHasNoBeatAndSilenceIsQuiet)
{
    std::mt19937 g(7);
    std::normal_distribution<float> n(0.0f, 0.2f);
    std::vector<float> noise((std::size_t) rate * 180); // as long as a song
    for (auto &x : noise)
        x = n(g);
    EXPECT_FALSE(goe::dj::analyse(noise, rate).hasBeat());
    const std::vector<float> silence((std::size_t) rate * 5, 0.0f);
    const auto quiet = goe::dj::analyse(silence, rate);
    EXPECT_TRUE(quiet.analysed);
    EXPECT_FALSE(quiet.hasBeat());
    EXPECT_LE(quiet.loudnessDb, -89.0f);
    EXPECT_FALSE(goe::dj::analyse({}, rate).analysed);
}

TEST(AutoDJTests, ReadsAFileAndRemembersItsAnalysis)
{
    const auto wav = scratchFile(".wav");
    const auto json = scratchFile(".json");
    {
        // CD rate stereo, so the file is mixed down and its rate brought down
        const auto mono = beatTrack(120.0, 0.25, 20);
        SF_INFO info{};
        info.samplerate = rate * 4;
        info.channels = 2;
        info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        SNDFILE *f = sf_open(wav.string().c_str(), SFM_WRITE, &info);
        ASSERT_NE(f, nullptr);
        std::vector<float> frames;
        for (float s : mono)
            for (int c = 0; c < 4 * 2; c++)
                frames.push_back(s);
        sf_writef_float(f, frames.data(), (sf_count_t) frames.size() / 2);
        sf_close(f);
    }
    const auto heard = goe::dj::analyseFile(wav.string());
    ASSERT_TRUE(heard.has_value());
    EXPECT_NEAR(heard->bpm, 120.0f, 0.5f);
    EXPECT_NEAR(heard->seconds, 20.0, 0.01);
    EXPECT_FALSE(goe::dj::analyseFile((fs::temp_directory_path() / "goe-dj-no-such.wav").string()).has_value());
    std::stop_source stop;
    stop.request_stop();
    EXPECT_FALSE(goe::dj::analyseFile(wav.string(), stop.get_token()).has_value());

    {
        goe::dj::analysisCache cache(json.string());
        EXPECT_FALSE(cache.find(wav.string()).has_value());
        cache.put(wav.string(), *heard);
        ASSERT_TRUE(cache.save());
    }
    goe::dj::analysisCache again(json.string());
    const auto kept = again.find(wav.string());
    ASSERT_TRUE(kept.has_value());
    EXPECT_NEAR(kept->bpm, heard->bpm, 0.001f);
    EXPECT_NEAR(kept->firstDownbeat, heard->firstDownbeat, 1e-6);
    // a changed file is listened to again
    {
        std::ofstream grow(wav, std::ios::app | std::ios::binary);
        grow << "more";
    }
    EXPECT_FALSE(again.find(wav.string()).has_value());
    fs::remove(wav);
    fs::remove(json);
}

TEST(AutoDJTests, MapsTheCalmestSongsLowAndTheWildestToDanger)
{
    std::vector<goe::dj::songEntry> songs(5);
    // song c gets louder, busier and faster with c, except that song 1 is the wildest
    const float wildness[] = {0, 4, 1, 2, 3};
    for (int c = 0; c < 5; c++) {
        auto &i = songs[c].info;
        i.analysed = true;
        i.loudnessDb = -20.0f + 2.0f * wildness[c];
        i.flux = 10.0f + wildness[c];
        i.brightness = 1000.0f + 100.0f * wildness[c];
        i.bpm = 100.0f + 10.0f * wildness[c];
    }
    const auto m = goe::dj::mapSongs(songs);
    EXPECT_TRUE(m.slots[1].danger); // no danger song marked: the wildest takes the part
    EXPECT_EQ(m.slots[0].level, 0);
    EXPECT_EQ(m.slots[4].level, goe::dj::topLevel);
    EXPECT_LT(m.slots[2].level, m.slots[3].level);
    // as loud as the average song: the quietest is raised, the loudest lowered
    EXPECT_GT(m.gains[0], 1.0f);
    EXPECT_LT(m.gains[1], 1.0f);

    // a marked danger song keeps the part, and the wildest plays at the top level
    songs[0].danger = true;
    const auto marked = goe::dj::mapSongs(songs);
    EXPECT_TRUE(marked.slots[0].danger);
    EXPECT_FALSE(marked.slots[1].danger);
    EXPECT_EQ(marked.slots[1].level, goe::dj::topLevel);
    EXPECT_EQ(marked.slots[2].level, 0);

    // before it has listened, the songs keep their places in the list
    const std::vector<goe::dj::songEntry> unheard(3);
    const auto fresh = goe::dj::mapSongs(unheard);
    EXPECT_EQ(fresh.slots[0].level, 0);
    EXPECT_EQ(fresh.slots[2].level, goe::dj::topLevel);
    EXPECT_FALSE(fresh.slots[0].danger || fresh.slots[1].danger || fresh.slots[2].danger);
    EXPECT_FLOAT_EQ(fresh.gains[1], 1.0f);
}

TEST(AutoDJTests, MixesOnTheNextPhraseOrBarWithTheTempoMatched)
{
    // 120 bpm: a beat is 0.5 s, a bar 2 s, a phrase 16 s, from a downbeat at 0.2 s
    const goe::dj::deck playing{beatInfo(120.0f, 0.2), 10.0, 1.0f};
    const auto plan = goe::dj::planMix(playing, beatInfo(125.0f, 1.0), false);
    EXPECT_TRUE(plan.beatMatched);
    EXPECT_NEAR(plan.wait, 16.2 - 10.0, 1e-9); // the phrase at 16.2 s is close enough
    EXPECT_NEAR(plan.cueAt, 1.0, 1e-9);        // in at its own first downbeat
    EXPECT_NEAR(plan.pitch, 120.0f / 125.0f, 1e-5f);
    EXPECT_NEAR(plan.fade, 8.0, 1e-9); // four bars

    // the phrase too far away: the next bar
    const goe::dj::deck early{beatInfo(120.0f, 0.2), 16.5, 1.0f};
    EXPECT_NEAR(goe::dj::planMix(early, beatInfo(120.0f, 0.0), false).wait, 18.2 - 16.5, 1e-9);
    // double or half time is matched too
    EXPECT_NEAR(goe::dj::planMix(playing, beatInfo(62.0f, 0.0), false).pitch, 120.0f / 124.0f, 1e-5f);
    // the playing song still bent: its real tempo counts, and the wait is in wall seconds
    const goe::dj::deck bent{beatInfo(120.0f, 0.2), 10.0, 1.05f};
    const auto fromBent = goe::dj::planMix(bent, beatInfo(126.0f, 0.0), false);
    EXPECT_NEAR(fromBent.pitch, 1.0f, 1e-5f);
    EXPECT_NEAR(fromBent.wait, (16.2 - 10.0) / 1.05, 1e-6);
}

TEST(AutoDJTests, DangerCutsInOnTheNextBeatAndOddSongsCrossfadePlainly)
{
    const goe::dj::deck playing{beatInfo(120.0f, 0.2), 10.0, 1.0f};
    const auto danger = goe::dj::planMix(playing, beatInfo(150.0f, 0.4), true);
    EXPECT_NEAR(danger.wait, 10.2 - 10.0, 1e-9);
    EXPECT_NEAR(danger.fade, 2.0, 1e-9); // a bar
    EXPECT_FALSE(danger.beatMatched);    // 150 is too far from 120 to bend
    EXPECT_FLOAT_EQ(danger.pitch, 1.0f);

    // near the end the song loops, and the mix waits for its first downbeat after the loop
    const goe::dj::deck ending{beatInfo(120.0f, 0.2, 60.0), 59.99, 1.0f};
    EXPECT_NEAR(goe::dj::planMix(ending, beatInfo(120.0f, 0.0), true).wait, 60.0 - 59.99 + 0.2, 1e-9);

    // a song without a beat: straight in, plain crossfade
    goe::dj::trackInfo ambient;
    ambient.analysed = true;
    const auto plain = goe::dj::planMix(playing, ambient, false);
    EXPECT_DOUBLE_EQ(plain.wait, 0.0);
    EXPECT_DOUBLE_EQ(plain.cueAt, 0.0);
    EXPECT_DOUBLE_EQ(plain.fade, goe::dj::plainMix);
}

TEST(AutoDJTests, EqualPowerCrossfadeAndPitchGlide)
{
    auto [out0, in0] = goe::dj::crossfade(0.0);
    EXPECT_FLOAT_EQ(out0, 1.0f);
    EXPECT_NEAR(in0, 0.0f, 1e-6f);
    auto [outHalf, inHalf] = goe::dj::crossfade(0.5);
    EXPECT_NEAR(outHalf * outHalf + inHalf * inHalf, 1.0f, 1e-5f); // the power stays
    auto [out1, in1] = goe::dj::crossfade(2.0);
    EXPECT_NEAR(out1, 0.0f, 1e-6f);
    EXPECT_FLOAT_EQ(in1, 1.0f);

    EXPECT_FLOAT_EQ(goe::dj::glide(0.96f, -1.0), 0.96f); // still mixing
    EXPECT_NEAR(goe::dj::glide(0.96f, goe::dj::glideSeconds / 2), 0.98f, 1e-6f);
    EXPECT_FLOAT_EQ(goe::dj::glide(0.96f, goe::dj::glideSeconds * 2), 1.0f);
}
