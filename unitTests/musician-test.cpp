// The adaptive musician, tested offline: no sound card, the audio is rendered into buffers.
#include "adaptiveMusician.h"
#include "musicComposer.h"
#include "musicCues.h"
#include "musicPersonality.h"
#include "musicSynth.h"
#include "musicTension.h"
#include "gameClock.h"
#include "difficulty.h"
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <new>
#include <numeric>
#include <set>
#include <vector>

// malloc and free stand behind every new and delete here, so they match
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"

// Counts allocations while armed, to prove the audio path never allocates
namespace {
std::atomic<bool> countAllocations{false};
std::atomic<int> allocations{0};
} // namespace

void *operator new(std::size_t n)
{
    if (countAllocations.load(std::memory_order_relaxed))
        allocations.fetch_add(1, std::memory_order_relaxed);
    if (void *p = std::malloc(n == 0 ? 1 : n))
        return p;
    throw std::bad_alloc();
}
void operator delete(void *p) noexcept
{
    std::free(p);
}
void operator delete(void *p, std::size_t) noexcept
{
    std::free(p);
}

using namespace goe::musician;

namespace {
constexpr int rate = 44100;
constexpr int sweep[] = {0, 32, 64, 96, 128, 160, 192, 224, 256};

/// renders seconds of music in blocks of the given size; the musician composes before each block
std::vector<float> render(adaptiveMusician &m, double seconds, std::uint32_t block = 1024, int channels = 2)
{
    const auto total = (std::uint32_t) (seconds * rate);
    std::vector<float> out((std::size_t) total * channels);
    for (std::uint32_t done = 0; done < total; done += block) {
        const std::uint32_t n = std::min(block, total - done);
        m.composeAhead();
        m.renderAudio(out.data() + (std::size_t) done * channels, n);
    }
    return out;
}

/// what a performer plays at one tension, over many phrases
struct playing
{
    double notesPerBeat = 0, syncopated = 0, chromatic = 0, coloured = 0, tempo = 0;
    double score() const { return notesPerBeat / 3.0 + syncopated + 4.0 * chromatic + coloured + tempo / 130.0; }
};

playing compose(std::uint64_t seed, int difficulty, int phrases, situation s = situation::calm)
{
    const auto who = performerPersonality::generate(seed);
    tensionController t(seed);
    t.settle(difficulty);
    composer c(seed);
    phraseBuffer buf;
    playing p;
    double notes = 0, beats = 0, chords = 0;
    for (int k = 0; k < phrases; k++) {
        t.advance(10.0f);
        c.compose(who, t.state(), s, (float) rate, buf);
        const auto &r = c.report();
        beats += r.bars * 4;
        notes += r.leadNotes;
        p.syncopated += r.syncopated;
        p.chromatic += r.chromatic;
        p.coloured += r.colouredChords;
        chords += r.chords;
        p.tempo += r.tempo;
    }
    p.notesPerBeat = notes / beats;
    p.syncopated /= std::max(notes, 1.0);
    p.chromatic /= std::max(notes, 1.0);
    p.coloured /= std::max(chords, 1.0);
    p.tempo /= phrases;
    return p;
}

/// the largest step between neighbouring samples of one channel
float largestStep(const std::vector<float> &v, int channels, std::size_t from = 0, std::size_t to = SIZE_MAX)
{
    float worst = 0;
    to = std::min(to, v.size() / channels);
    for (std::size_t i = std::max<std::size_t>(from, 1); i < to; i++)
        worst = std::max(worst, std::fabs(v[i * channels] - v[(i - 1) * channels]));
    return worst;
}

std::vector<float> renderSynth(synthesizer &s, int frames, std::vector<float> *right = nullptr)
{
    std::vector<float> l((std::size_t) frames), r((std::size_t) frames);
    for (int done = 0; done < frames; done += synthesizer::blockFrames) {
        const int n = std::min(synthesizer::blockFrames, frames - done);
        s.render(l.data() + done, r.data() + done, n);
    }
    if (right)
        *right = r;
    return l;
}

instrument plainSine()
{
    instrument i;
    i.wave = waveform::sine;
    i.secondMix = 0;
    i.vibratoCents = 0;
    i.tremoloDepth = 0;
    i.cutoff = 12000;
    i.filterEnvelope = 0;
    i.keyTracking = 0;
    i.width = 0;
    i.attack = 0.01f;
    i.release = 0.05f;
    return i;
}
} // namespace

// ---- personality ----

TEST(Personality, SameSeedSamePerformer)
{
    const auto a = performerPersonality::generate(42), b = performerPersonality::generate(42);
    EXPECT_EQ(a.energy, b.energy);
    EXPECT_EQ(a.keyRoot, b.keyRoot);
    EXPECT_EQ(a.baseTempo, b.baseTempo);
    EXPECT_EQ(a.lead.cutoff, b.lead.cutoff);
    EXPECT_EQ((int) a.lead.wave, (int) b.lead.wave);
}

TEST(Personality, EveryGeneratedPerformerIsWithinLimits)
{
    for (std::uint64_t seed = 0; seed < 3000; seed++) {
        const auto p = performerPersonality::generate(seed);
        ASSERT_TRUE(p.withinLimits()) << "seed " << seed;
        ASSERT_LE(p.busyness(), tuning::busynessBudget + 1e-4f) << "seed " << seed;
    }
}

TEST(Personality, NoCacophonousCombinations)
{
    // energetic, complex, syncopated, dissonant and loose all at once never happens
    for (std::uint64_t seed = 0; seed < 3000; seed++) {
        const auto p = performerPersonality::generate(seed);
        const int extremes = (p.energy > 0.7f) + (p.rhythmicComplexity > 0.55f) + (p.syncopation > 0.45f)
                             + (p.dissonance > 0.4f) + (p.timingDrift > 0.55f);
        ASSERT_LT(extremes, 4) << "seed " << seed;
    }
}

TEST(Personality, PerformersDifferInPlayingAndSound)
{
    std::vector<performerPersonality> ps;
    for (std::uint64_t seed = 100; seed < 150; seed++)
        ps.push_back(performerPersonality::generate(seed));
    auto playingDistance = [](const performerPersonality &a, const performerPersonality &b) {
        return std::fabs(a.energy - b.energy) + std::fabs(a.rhythmicComplexity - b.rhythmicComplexity)
               + std::fabs(a.repetition - b.repetition) + std::fabs(a.syncopation - b.syncopation)
               + std::fabs(a.articulation - b.articulation) + std::fabs(a.silence - b.silence)
               + std::fabs(a.dissonance - b.dissonance) + std::fabs(a.registerBias - b.registerBias);
    };
    double total = 0;
    int pairs = 0;
    for (std::size_t i = 0; i < ps.size(); i++)
        for (std::size_t j = i + 1; j < ps.size(); j++) {
            const double d = playingDistance(ps[i], ps[j]);
            EXPECT_GT(d, 0.02) << i << " and " << j << " play alike";
            total += d;
            pairs++;
        }
    EXPECT_GT(total / pairs, 0.6);
    std::set<int> waves, keys;
    std::set<int> cutoffs;
    for (const auto &p : ps) {
        waves.insert((int) p.lead.wave);
        keys.insert(p.keyRoot);
        cutoffs.insert((int) (p.lead.cutoff / 250));
    }
    EXPECT_GE(waves.size(), 3u);
    EXPECT_GE(keys.size(), 6u);
    EXPECT_GE(cutoffs.size(), 6u);
}

// ---- tension ----

TEST(Tension, CurveIsNonlinearMonotoneAndRestrained)
{
    EXPECT_NEAR(tensionController::targetFor(0), 0.0f, 1e-6f);
    EXPECT_NEAR(tensionController::targetFor(128), 0.30f, 0.01f);
    EXPECT_NEAR(tensionController::targetFor(256), 0.70f, 0.001f);
    for (int d = 1; d <= 256; d++) {
        EXPECT_GE(tensionController::targetFor(d), tensionController::targetFor(d - 1));
        EXPECT_LT(tensionController::targetFor(d) - tensionController::targetFor(d - 1), 0.01f) << d;
    }
    EXPECT_EQ(tensionController::targetFor(-5), tensionController::targetFor(0));
    EXPECT_EQ(tensionController::targetFor(999), tensionController::targetFor(256));
}

TEST(Tension, FollowsGraduallyAndAtItsOwnPacePerPart)
{
    tensionController t(1);
    t.settle(0);
    t.setDifficulty(256);
    t.advance(0.1f);
    EXPECT_LT(t.smoothed().tempo, 0.05f);
    t.advance(4.0f);
    // tempo is quick, harmony and phrasing slow
    EXPECT_GT(t.smoothed().tempo, t.smoothed().density);
    EXPECT_GT(t.smoothed().density, t.smoothed().harmony);
    EXPECT_GT(t.smoothed().harmony, t.smoothed().phrase - 1e-6f);
    for (int s = 0; s < 120; s++)
        t.advance(1.0f);
    EXPECT_NEAR(t.smoothed().phrase, 0.70f, 0.01f);
}

TEST(Tension, SmoothingDoesNotDependOnStepSize)
{
    tensionController fine(1), coarse(1);
    fine.settle(64);
    coarse.settle(64);
    fine.setDifficulty(192);
    coarse.setDifficulty(192);
    for (int s = 0; s < 1000; s++)
        fine.advance(0.01f);
    coarse.advance(5.0f);
    coarse.advance(5.0f);
    EXPECT_NEAR(fine.smoothed().tempo, coarse.smoothed().tempo, 1e-4f);
    EXPECT_NEAR(fine.smoothed().harmony, coarse.smoothed().harmony, 1e-4f);
    EXPECT_NEAR(fine.smoothed().phrase, coarse.smoothed().phrase, 1e-4f);
}

TEST(Tension, TransitionsAreSmoothBothWays)
{
    for (auto [from, to] : {std::pair{0, 256}, {256, 0}, {64, 192}, {192, 64}}) {
        tensionController t(7);
        t.settle(from);
        t.setDifficulty(to);
        auto last = t.state();
        for (int s = 0; s < 2000; s++) {
            t.advance(0.05f);
            const auto now = t.state();
            for (auto [a, b] : {std::pair{last.tempo, now.tempo}, {last.density, now.density}, {last.harmony, now.harmony},
                                {last.phrase, now.phrase}, {last.timbre, now.timbre}})
                ASSERT_LT(std::fabs(a - b), 0.02f) << from << " -> " << to << " at step " << s;
            last = now;
        }
        EXPECT_NEAR(t.smoothed().harmony, tensionController::targetFor(to), 0.01f);
    }
}

TEST(Tension, OneStepOfDifficultyIsNoAudibleChange)
{
    tensionController t(3);
    t.settle(127);
    const auto before = t.smoothed();
    t.setDifficulty(128);
    t.advance(1.0f);
    EXPECT_LT(std::fabs(t.smoothed().tempo - before.tempo), 0.002f);
    EXPECT_LT(std::fabs(t.smoothed().harmony - before.harmony), 0.002f);
}

TEST(Tension, MoodWandersSlowlyWithinItsRange)
{
    tensionController t(11);
    t.settle(128);
    double sum = 0, sum2 = 0, biggestStep = 0;
    float last = t.state().mood;
    const int steps = 36000; // an hour in tenths of a second
    for (int s = 0; s < steps; s++) {
        t.advance(0.1f);
        const float m = t.state().mood;
        ASSERT_LE(std::fabs(m), tuning::moodRange + 1e-6f);
        biggestStep = std::max(biggestStep, (double) std::fabs(m - last));
        last = m;
        sum += m;
        sum2 += m * m;
    }
    const double mean = sum / steps, deviation = std::sqrt(sum2 / steps - mean * mean);
    EXPECT_GT(deviation, 0.01) << "the mood should move";
    EXPECT_LT(std::fabs(mean), 0.03) << "and come back to the tension";
    EXPECT_LT(biggestStep, 0.02) << "slowly";
    // the tension itself does not drift away while the mood moves
    EXPECT_NEAR(t.smoothed().harmony, tensionController::targetFor(128), 1e-4f);
}

// ---- composing ----

TEST(Composer, DifficultySweepRaisesTensionProgressively)
{
    std::vector<playing> at;
    for (int d : sweep) {
        playing sum;
        const int seeds = 24;
        for (std::uint64_t seed = 1; seed <= seeds; seed++) {
            const auto p = compose(seed, d, 30);
            sum.notesPerBeat += p.notesPerBeat / seeds;
            sum.syncopated += p.syncopated / seeds;
            sum.chromatic += p.chromatic / seeds;
            sum.coloured += p.coloured / seeds;
            sum.tempo += p.tempo / seeds;
        }
        at.push_back(sum);
    }
    for (std::size_t i = 1; i < at.size(); i++) {
        EXPECT_GT(at[i].tempo, at[i - 1].tempo) << sweep[i];
        EXPECT_GT(at[i].score(), at[i - 1].score() - 0.01) << sweep[i];
    }
    const auto &low = at.front(), &high = at.back();
    EXPECT_GT(high.notesPerBeat, low.notesPerBeat * 1.15);
    EXPECT_GT(high.syncopated, low.syncopated);
    EXPECT_GT(high.chromatic, low.chromatic);
    EXPECT_GT(high.coloured, low.coloured);
    // more tense, but restrained: still a moderate tempo and mostly plain harmony and melody
    EXPECT_LT(high.tempo / low.tempo, 1.15);
    EXPECT_LT(high.coloured, tuning::maxDissonance);
    EXPECT_LT(high.chromatic, tuning::maxChromaticRate);
}

TEST(Composer, SafetyLimitsHoldAtMaximumDifficultyForEveryPerformer)
{
    for (std::uint64_t seed = 1; seed <= 200; seed++) {
        const auto who = performerPersonality::generate(seed);
        tensionController t(seed);
        t.settle(256);
        composer c(seed);
        phraseBuffer buf;
        for (int k = 0; k < 20; k++) {
            t.advance(10.0f);
            for (auto s : {situation::calm, situation::alert, situation::danger}) {
                c.compose(who, t.state(), s, (float) rate, buf);
                const auto &r = c.report();
                ASSERT_LE(r.notesPerBeat(), tuning::maxNoteDensity) << seed;
                ASSERT_LE(r.maxSimultaneous, tuning::maxSimultaneousNotes) << seed;
                ASSERT_LE(r.maxLeap, tuning::maxRegisterJump) << seed;
                ASSERT_LE(r.chromatic, (int) std::floor(tuning::maxChromaticRate * r.leadNotes)) << seed;
                ASSERT_LE(r.maxJitterMs, tuning::maxTimingJitterMs) << seed;
                ASSERT_GE(r.tempo, tuning::minTempo);
                ASSERT_LE(r.tempo, tuning::maxTempo);
                ASSERT_GT(buf.length, 0);
                for (int e = 0; e < buf.count; e++) {
                    const auto &ev = buf.events[(std::size_t) e];
                    ASSERT_GE(ev.at, 0);
                    if (ev.what == noteEvent::kind::on) {
                        ASSERT_GE(ev.pitch, 24.0f);
                        ASSERT_LE(ev.pitch, 96.0f);
                        ASSERT_GT(ev.velocity, 0.0f);
                        ASSERT_LE(ev.velocity, 1.0f);
                    }
                }
            }
        }
    }
}

TEST(Composer, EveryNoteThatStartsAlsoStops)
{
    const auto who = performerPersonality::generate(5);
    tensionController t(5);
    t.settle(200);
    composer c(5);
    phraseBuffer buf;
    for (int k = 0; k < 50; k++) {
        c.compose(who, t.state(), (situation) (k % 3), (float) rate, buf);
        std::set<std::uint32_t> open;
        for (int e = 0; e < buf.count; e++) {
            const auto &ev = buf.events[(std::size_t) e];
            if (ev.what == noteEvent::kind::on) {
                open.insert(ev.note);
            } else {
                EXPECT_EQ(open.erase(ev.note), 1u) << "an end without a start";
            }
            if (e > 0) {
                EXPECT_LE(buf.events[(std::size_t) e - 1].at, ev.at) << "events in time order";
            }
        }
        EXPECT_TRUE(open.empty());
    }
}

TEST(Composer, LongRunAtConstantDifficultyStaysCoherent)
{
    for (std::uint64_t seed : {2u, 9u, 31u, 77u}) {
        const auto who = performerPersonality::generate(seed);
        tensionController t(seed);
        t.settle(128);
        composer c(seed);
        phraseBuffer buf;
        int repeatsRun = 0, freshRun = 0, fresh = 0, related = 0, silent = 0;
        bool lastSilent = false;
        double firstHalf = 0, secondHalf = 0;
        std::set<std::uint32_t> families;
        const int phrases = 600; // well over an hour of music
        for (int k = 0; k < phrases; k++) {
            t.advance(10.0f);
            c.compose(who, t.state(), situation::calm, (float) rate, buf);
            const auto &r = c.report();
            (k < phrases / 2 ? firstHalf : secondHalf) += r.notesPerBeat();
            if (r.silent) {
                ASSERT_FALSE(lastSilent) << "two silent phrases in a row";
                silent++;
                lastSilent = true;
                continue;
            }
            lastSilent = false;
            families.insert(r.family);
            repeatsRun = r.made == phraseReport::relation::repeat ? repeatsRun + 1 : 0;
            freshRun = r.made == phraseReport::relation::fresh ? freshRun + 1 : 0;
            ASSERT_LE(repeatsRun, tuning::maxConsecutiveRepetitions) << seed;
            ASSERT_LE(freshRun, tuning::maxConsecutiveUnrelated) << seed;
            (r.made == phraseReport::relation::fresh ? fresh : related)++;
        }
        EXPECT_GT(related, fresh) << "recurring material outweighs new ideas, seed " << seed;
        EXPECT_GT(fresh, phrases / 20) << "but new ideas keep coming, seed " << seed;
        EXPECT_GT(silent, 0) << "silence is part of the music, seed " << seed;
        EXPECT_LT(silent, phrases / 3) << seed;
        EXPECT_GT(families.size(), 10u) << seed;
        EXPECT_NEAR(firstHalf / secondHalf, 1.0, 0.15) << "no runaway density, seed " << seed;
    }
}

TEST(Composer, SituationChangesTheThemeNotTheTension)
{
    const auto who = performerPersonality::generate(21);
    tensionController t(21);
    t.settle(100);
    composer c(21);
    phraseBuffer buf;
    c.compose(who, t.state(), situation::calm, (float) rate, buf);
    const auto calm = c.report();
    c.compose(who, t.state(), situation::alert, (float) rate, buf);
    const auto alert = c.report();
    c.compose(who, t.state(), situation::danger, (float) rate, buf);
    const auto danger = c.report();
    EXPECT_EQ(calm.transpose, 0);
    EXPECT_NE(alert.transpose, 0);
    EXPECT_EQ(danger.transpose, alert.transpose + 1);
    EXPECT_GT(alert.tempo, calm.tempo);
    EXPECT_GT(danger.tempo, alert.tempo);
    // the alert theme grows out of the main theme, the danger theme out of the alert one
    EXPECT_NE(alert.made, phraseReport::relation::fresh);
    EXPECT_NE(danger.made, phraseReport::relation::fresh);
    EXPECT_EQ(c.remembered(situation::calm), 1);
    EXPECT_EQ(c.remembered(situation::alert), 1);
    // back to calm: the main theme returns
    for (int k = 0; k < 6; k++)
        c.compose(who, t.state(), situation::calm, (float) rate, buf);
    EXPECT_LE(c.remembered(situation::calm), 7);
    EXPECT_NEAR(t.target(), tensionController::targetFor(100), 1e-6f);
}

// ---- synthesizer ----

TEST(Synth, NotesStartAndEndWithoutClicks)
{
    for (auto w : {waveform::sine, waveform::triangle, waveform::saw, waveform::pulse}) {
        synthesizer s((float) rate, 4);
        instrument i = plainSine();
        i.wave = w;
        i.cutoff = 3000;
        s.setSound(part::lead, i);
        s.noteOn({0, 0, noteEvent::kind::on, part::lead, 1, 57.0f, 1.0f});
        auto held = renderSynth(s, rate / 2);
        s.noteOff({0, 0, noteEvent::kind::off, part::lead, 1, 0, 0});
        auto tail = renderSynth(s, rate / 2);
        const float steady = largestStep(held, 1, rate / 4, rate / 2);
        // the first and last samples move no more than the note itself does while it sounds
        EXPECT_LE(largestStep(held, 1, 0, 200), steady * 1.1f) << (int) w;
        EXPECT_LE(largestStep(tail, 1), steady * 1.1f) << (int) w;
        EXPECT_NEAR(tail.back(), 0.0f, 1e-6f);
        EXPECT_EQ(s.activeVoices(), 0);
    }
}

TEST(Synth, PolyphonyIsBoundedAndStealingIsDeterministicAndQuiet)
{
    synthesizer s((float) rate, 8);
    s.setSound(part::lead, plainSine());
    std::vector<float> all;
    for (std::uint32_t n = 1; n <= 64; n++) {
        s.noteOn({0, 0, noteEvent::kind::on, part::lead, n, 48.0f + (float) (n % 24), 0.8f});
        auto block = renderSynth(s, 512);
        all.insert(all.end(), block.begin(), block.end());
        ASSERT_LE(s.activeVoices(), 8);
    }
    EXPECT_EQ(s.report().stolen, 56);
    // a taken-over voice glides from where it was: no step bigger than eight full sines could make
    const float sineStep = 2.0f * 3.1416f * 440.0f / rate * tuning::headroom * 0.8f;
    EXPECT_LT(largestStep(all, 1), 8 * sineStep * 2.0f);
    // the oldest note went first: notes 57..64 are the ones left
    for (std::uint32_t n = 1; n <= 56; n++)
        s.noteOff({0, 0, noteEvent::kind::off, part::lead, n, 0, 0});
    EXPECT_EQ(s.activeVoices(), 8); // none of them was still sounding
}

TEST(Synth, PitchFollowsTheSampleRate)
{
    for (int r : {22050, 44100, 48000, 96000}) {
        synthesizer s((float) r, 2);
        s.setSound(part::lead, plainSine());
        s.noteOn({0, 0, noteEvent::kind::on, part::lead, 1, 69.0f, 1.0f});
        renderSynth(s, r / 10); // past the attack
        auto second = renderSynth(s, r);
        int crossings = 0;
        for (std::size_t i = 1; i < second.size(); i++)
            crossings += (second[i - 1] < 0) != (second[i] < 0);
        EXPECT_NEAR(crossings, 880, 4) << r;
    }
}

TEST(Synth, BadInputBecomesSilenceOrLimits)
{
    synthesizer s((float) rate, 4);
    s.setSound(part::lead, plainSine());
    s.noteOn({0, 0, noteEvent::kind::on, part::lead, 1, NAN, 1.0f});
    s.noteOn({0, 0, noteEvent::kind::on, part::lead, 2, 400.0f, 5.0f});
    s.noteOn({0, 0, noteEvent::kind::on, part::lead, 3, -400.0f, -1.0f});
    auto out = renderSynth(s, rate);
    for (float v : out) {
        ASSERT_TRUE(std::isfinite(v));
        ASSERT_LE(std::fabs(v), 1.0f);
    }
}

TEST(Synth, SoftClipNeverPassesOne)
{
    EXPECT_EQ(softClip(0.5f), 0.5f);
    EXPECT_EQ(softClip(-tuning::clipKnee), -tuning::clipKnee);
    for (float x = -50; x <= 50; x += 0.01f)
        ASSERT_LE(std::fabs(softClip(x)), 1.0f);
    EXPECT_GT(softClip(2.0f), softClip(1.0f));
}

// ---- the musician as a whole ----

TEST(Musician, PlaysContinuouslyAndDeterministically)
{
    adaptiveMusician a, b;
    ASSERT_TRUE(a.initialize({rate, 2}, 99));
    ASSERT_TRUE(b.initialize({rate, 2}, 99));
    a.setDifficulty(140);
    b.setDifficulty(140);
    const auto x = render(a, 30), y = render(b, 30);
    EXPECT_EQ(x, y);
    // something plays in every five seconds: the musician never stops by itself
    for (std::size_t s = 2; s < 30; s += 5) {
        float loudest = 0;
        for (std::size_t i = s * rate * 2; i < (s + 5) * rate * 2 && i < x.size(); i++)
            loudest = std::max(loudest, std::fabs(x[i]));
        EXPECT_GT(loudest, 0.01f) << "silent from " << s << " s";
    }
    EXPECT_GT(a.phrasesComposed(), 1);
}

TEST(Musician, EventsAreNotTiedToBufferBoundaries)
{
    adaptiveMusician a, b, c;
    for (auto *m : {&a, &b, &c}) {
        ASSERT_TRUE(m->initialize({rate, 2}, 1234));
        m->setDifficulty(90);
    }
    const auto small = render(a, 20, 64), odd = render(b, 20, 1000), big = render(c, 20, 4096);
    double signal = 0, diffOdd = 0, diffBig = 0;
    for (std::size_t i = 0; i < small.size(); i++) {
        signal += small[i] * small[i];
        diffOdd += (small[i] - odd[i]) * (small[i] - odd[i]);
        diffBig += (small[i] - big[i]) * (small[i] - big[i]);
    }
    // the same notes at the same samples; only slow timbre glides are updated at block edges
    EXPECT_LT(diffOdd / signal, 1e-3);
    EXPECT_LT(diffBig / signal, 1e-3);
}

TEST(Musician, NoClippingAndBoundedVoicesForManyPerformers)
{
    for (std::uint64_t seed = 1; seed <= 12; seed++) {
        adaptiveMusician m;
        ASSERT_TRUE(m.initialize({rate, 2}, seed));
        m.setDifficulty(256);
        m.setSituation(situation::danger);
        const auto out = render(m, 30);
        std::size_t bent = 0;
        for (float v : out) {
            ASSERT_TRUE(std::isfinite(v));
            ASSERT_LE(std::fabs(v), 1.0f);
            bent += std::fabs(v) > tuning::clipKnee;
        }
        EXPECT_LT((double) bent / out.size(), 1e-4) << "the headroom is enough, seed " << seed;
        EXPECT_LE(m.synth().report().activeVoices, m.synth().polyphony());
    }
}

TEST(Musician, RenderingNeverAllocates)
{
    adaptiveMusician m;
    ASSERT_TRUE(m.initialize({rate, 2}, 5));
    m.setDifficulty(200);
    std::vector<float> buf(4096 * 2);
    m.composeAhead();
    m.renderAudio(buf.data(), 4096);
    allocations = 0;
    countAllocations = true;
    for (int k = 0; k < 600; k++) { // about a minute, with composing and situation changes
        m.setSituation((situation) ((k / 100) % 3));
        m.setDifficulty(k % 257);
        m.composeAhead();
        m.renderAudio(buf.data(), 4096);
    }
    countAllocations = false;
    EXPECT_EQ(allocations.load(), 0);
}

TEST(Musician, PauseHoldsTheMusicAndResumeGoesOn)
{
    adaptiveMusician m;
    ASSERT_TRUE(m.initialize({rate, 2}, 8));
    render(m, 5);
    const auto at = m.position();
    m.pause();
    const auto paused = render(m, 2);
    // a short fade, then silence, and the music does not move on
    EXPECT_LE(m.position() - at, (std::int64_t) (tuning::fadeSeconds * rate) + 1024);
    for (std::size_t i = (std::size_t) rate * 2 / 2; i < paused.size(); i++)
        ASSERT_EQ(paused[i], 0.0f);
    std::size_t lastSound = 0;
    for (std::size_t i = 0; i < paused.size(); i++)
        if (paused[i] != 0.0f)
            lastSound = i;
    for (std::size_t i = lastSound > 400 ? lastSound - 400 : 0; i <= lastSound; i++)
        ASSERT_LT(std::fabs(paused[i]), 0.02f) << "it fades, it does not stop dead";
    m.resume();
    const auto back = render(m, 3);
    EXPECT_GT(m.position(), at + rate);
    EXPECT_GT(*std::max_element(back.begin(), back.end()), 0.01f);
}

TEST(Musician, DisableFadesOutAndEnableStartsAgain)
{
    adaptiveMusician m;
    ASSERT_TRUE(m.initialize({rate, 2}, 8));
    render(m, 4);
    m.setEnabled(false);
    const auto off = render(m, 4);
    EXPECT_EQ(m.queued(), 0);
    for (std::size_t i = (std::size_t) rate * 2; i < off.size(); i++)
        ASSERT_EQ(off[i], 0.0f);
    m.setEnabled(true);
    const auto on = render(m, 4);
    EXPECT_GT(*std::max_element(on.begin(), on.end()), 0.01f);
}

TEST(Musician, VolumeIsTheMusiciansOwnGain)
{
    adaptiveMusician loud, quiet;
    ASSERT_TRUE(loud.initialize({rate, 2}, 4));
    ASSERT_TRUE(quiet.initialize({rate, 2}, 4));
    quiet.setVolume(0.5f);
    const auto a = render(loud, 10), b = render(quiet, 10);
    double ea = 0, eb = 0;
    for (std::size_t i = 0; i < a.size(); i++) {
        ea += a[i] * a[i];
        eb += b[i] * b[i];
    }
    EXPECT_NEAR(std::sqrt(eb / ea), 0.5, 0.01);
    quiet.setVolume(NAN); // ignored
    quiet.setVolume(7.0f); // clamped to 1
    render(quiet, 1);
}

TEST(Musician, RisingDangerIsHeardFromTheNextBar)
{
    adaptiveMusician m;
    ASSERT_TRUE(m.initialize({rate, 2}, 13));
    m.setDifficulty(60);
    render(m, 3);
    m.setSituation(situation::danger);
    // the slowest bar is four seconds; the current phrase is not played to its end first
    double waited = 0;
    while (m.lastPhrase().theme != situation::danger && waited < 10)
        render(m, 0.1), waited += 0.1;
    EXPECT_EQ(m.lastPhrase().theme, situation::danger);
    EXPECT_LE(waited, 60.0 * 4 / tuning::minTempo);
    // and the difficulty did not move
    EXPECT_NEAR(m.tension().target(), tensionController::targetFor(60), 1e-6f);
}

TEST(Musician, InvalidFormatsFailToSilence)
{
    adaptiveMusician m;
    EXPECT_FALSE(m.initialize({rate, 6}, 1));
    EXPECT_FALSE(m.initialize({100, 2}, 1));
    std::vector<float> buf(512 * 2, 1.0f);
    m.composeAhead();
    m.renderAudio(buf.data(), 512);
    EXPECT_TRUE(std::all_of(buf.begin(), buf.end(), [](float v) { return v == 0.0f; }));
    m.renderAudio(nullptr, 512);
    ASSERT_TRUE(m.initialize({rate, 1}, 1)); // mono works
    const auto mono = render(m, 3, 1024, 1);
    EXPECT_GT(*std::max_element(mono.begin(), mono.end()), 0.01f);
    m.shutdown();
    m.renderAudio(buf.data(), 512);
    EXPECT_TRUE(std::all_of(buf.begin(), buf.begin() + 512, [](float v) { return v == 0.0f; }));
}

TEST(Musician, CheapEnoughForContinuousPlay)
{
    adaptiveMusician m;
    ASSERT_TRUE(m.initialize({48000, 2}, 17));
    m.setDifficulty(256);
    const auto t0 = std::chrono::steady_clock::now();
    std::vector<float> buf(2048 * 2);
    for (int k = 0; k < 48000 * 60 / 2048; k++) {
        m.composeAhead();
        m.renderAudio(buf.data(), 2048);
    }
    const double took = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    // a minute of music in well under a tenth of a minute, even in a slow debug build
    EXPECT_LT(took, 6.0);
}

// ---- the game's side ----

TEST(MusicCues, ReportsBecomeSituationsAndFade)
{
    goe::music::cues::reset();
    EXPECT_EQ(goe::music::cues::now(), situation::calm);
    goe::music::cues::sighted();
    EXPECT_EQ(goe::music::cues::now(), situation::alert);
    goe::music::cues::endangered();
    EXPECT_EQ(goe::music::cues::now(), situation::danger);
    for (int t = 0; t <= difficulty::musicDangerTicks; t++)
        gameClock::advance();
    EXPECT_EQ(goe::music::cues::now(), situation::alert); // the danger passed, the guardians still look
    for (int t = 0; t <= difficulty::musicAlertTicks; t++)
        gameClock::advance();
    EXPECT_EQ(goe::music::cues::now(), situation::calm);
    goe::music::cues::chased();
    EXPECT_EQ(goe::music::cues::now(), situation::alert);
    goe::music::cues::reset();
    EXPECT_EQ(goe::music::cues::now(), situation::calm);
}

TEST(MusicCues, DifficultyMapsIntoThePerformersRange)
{
    EXPECT_EQ(difficulty::musicianLevel(0), 0);
    EXPECT_EQ(difficulty::musicianLevel(1), 23);
    EXPECT_EQ(difficulty::musicianLevel(11), 253);
    EXPECT_EQ(difficulty::musicianLevel(40), 256);
    EXPECT_EQ(difficulty::musicianLevel(-3), 0);
}
