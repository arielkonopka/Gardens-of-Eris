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
#include "musicAnalysis.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <memory>
#include <numbers>
#include <numeric>
#include <sndfile.h>
#include <vector>

namespace goe::dj {

namespace {
/// the rate the analysis works at: plenty for beats, and four times less work than CD rate
constexpr int analysisRate = 11025;
/// the spectrum window and the step between two windows, in samples
constexpr int windowSize = 1024;
constexpr int hop = 128;
/// only the flux below this frequency counts: drums and bass carry the beat
constexpr float fluxTopHz = 6000.0f;
/// the tempo range and the tempo a beat is most likely to have (a log-normal prior over it)
constexpr float minBpm = 70.0f;
constexpr float maxBpm = 180.0f;
constexpr float likelyBpm = 130.0f;
constexpr float likelyOctaves = 1.0f;
/// how far the fine tempo search looks around the autocorrelation's tempo, and its step
constexpr float refineSpan = 0.02f;
constexpr float refineStep = 0.001f;
/// a beat grid this much stronger than the average flux is a beat to mix on (noise reaches about
/// 0.12 over a three-minute song, the songs of the game 0.3 to 0.75)
constexpr float neededStrength = 0.2f;
/// the moving average taken off the flux, in seconds either side
constexpr float baselineSeconds = 0.25f;
/// a song is at full strength from the first second that is at most this far below its average level
constexpr float entryBelowDb = 6.0f;

void fft(std::vector<std::complex<float>> &a)
{
    const std::size_t n = a.size();
    for (std::size_t i = 1, j = 0; i < n; i++) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(a[i], a[j]);
    }
    for (std::size_t len = 2; len <= n; len <<= 1) {
        const float ang = -2.0f * std::numbers::pi_v<float> / (float) len;
        const std::complex<float> step(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (std::size_t k = 0; k < len / 2; k++) {
                const auto u = a[i + k];
                const auto v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= step;
            }
        }
    }
}

struct spectrumFeatures
{
    std::vector<float> flux; ///< one value per hop
    float brightness = 0.0f;
};

spectrumFeatures measureSpectrum(std::span<const float> mono, int rate)
{
    spectrumFeatures out;
    if ((int) mono.size() < windowSize)
        return out;
    std::vector<float> window(windowSize);
    float windowSum = 0.0f;
    for (int i = 0; i < windowSize; i++) {
        window[i] = 0.5f - 0.5f * std::cos(2.0f * std::numbers::pi_v<float> * (float) i / (float) windowSize);
        windowSum += window[i];
    }
    const int bins = windowSize / 2;
    const int fluxBins = std::min(bins, (int) (fluxTopHz * (float) windowSize / (float) rate));
    std::vector<std::complex<float>> buf(windowSize);
    std::vector<float> previous(bins, 0.0f);
    std::vector<float> now(bins);
    double centroidSum = 0.0;
    double centroidWeight = 0.0;
    for (std::size_t start = 0; start + windowSize <= mono.size(); start += hop) {
        for (int i = 0; i < windowSize; i++)
            buf[i] = {mono[start + i] * window[i], 0.0f};
        fft(buf);
        float flux = 0.0f;
        double magSum = 0.0;
        double weighted = 0.0;
        for (int b = 1; b < bins; b++) {
            const float mag = std::abs(buf[b]) * 2.0f / windowSum;
            now[b] = std::log1p(100.0f * mag);
            if (b < fluxBins)
                flux += std::max(0.0f, now[b] - previous[b]);
            magSum += mag;
            weighted += mag * (double) b * rate / windowSize;
        }
        // the first window has nothing before it
        out.flux.push_back(start == 0 ? 0.0f : flux);
        centroidSum += weighted;
        centroidWeight += magSum;
        std::swap(now, previous);
    }
    out.brightness = centroidWeight > 0.0 ? (float) (centroidSum / centroidWeight) : 0.0f;
    return out;
}

/// the flux with its moving average taken off and only the rises kept: the onsets
std::vector<float> onsets(const std::vector<float> &flux, int span)
{
    std::vector<float> out(flux.size());
    std::vector<double> sum(flux.size() + 1, 0.0);
    for (std::size_t i = 0; i < flux.size(); i++)
        sum[i + 1] = sum[i] + flux[i];
    for (std::size_t i = 0; i < flux.size(); i++) {
        const std::size_t a = i > (std::size_t) span ? i - span : 0;
        const std::size_t b = std::min(flux.size(), i + span + 1);
        const float mean = (float) ((sum[b] - sum[a]) / (double) (b - a));
        out[i] = std::max(0.0f, flux[i] - mean);
    }
    return out;
}

float at(const std::vector<float> &o, double x)
{
    const auto i = (std::size_t) x;
    if (i + 1 >= o.size())
        return i < o.size() ? o[i] : 0.0f;
    const float f = (float) (x - (double) i);
    return o[i] * (1.0f - f) + o[i + 1] * f;
}

/// the mean of the onsets on a beat grid, and how many beats it had
float gridMean(const std::vector<float> &o, double phase, double period)
{
    double s = 0.0;
    int n = 0;
    for (double x = phase; x < (double) o.size(); x += period, n++)
        s += at(o, x);
    return n > 0 ? (float) (s / n) : 0.0f;
}

float autocorrelation(const std::vector<float> &o, int lag)
{
    if (lag <= 0 || lag >= (int) o.size())
        return 0.0f;
    double s = 0.0;
    for (std::size_t i = 0; i + lag < o.size(); i++)
        s += (double) o[i] * o[i + lag];
    return (float) (s / (double) (o.size() - lag));
}

/// the tempo's beat period in frames from the autocorrelation, weighted towards likelyBpm; 0 when none
double roughPeriod(const std::vector<float> &o, double frameRate)
{
    const int shortest = std::max(1, (int) std::floor(frameRate * 60.0 / maxBpm));
    const int longest = (int) std::ceil(frameRate * 60.0 / minBpm);
    if (longest * 2 >= (int) o.size())
        return 0.0;
    std::vector<float> score(longest + 2, 0.0f);
    for (int lag = shortest - 1; lag <= longest + 1; lag++) {
        const double bpm = frameRate * 60.0 / lag;
        const double octaves = std::log2(bpm / likelyBpm) / likelyOctaves;
        const float prior = (float) std::exp(-0.5 * octaves * octaves);
        // a beat repeats at twice its period too; a mere offbeat does not
        score[lag] = (autocorrelation(o, lag) + 0.5f * autocorrelation(o, 2 * lag)) * prior;
    }
    int best = shortest;
    for (int lag = shortest; lag <= longest; lag++)
        if (score[lag] > score[best])
            best = lag;
    if (score[best] <= 0.0f)
        return 0.0;
    // a parabola through the peak and its neighbours
    const float l = score[best - 1], c = score[best], r = score[best + 1];
    const float d = l - 2.0f * c + r;
    const double shift = d < 0.0f ? std::clamp(0.5f * (l - r) / d, -0.5f, 0.5f) : 0.0f;
    return best + shift;
}
} // namespace

bool trackInfo::hasBeat() const
{
    return this->analysed && this->bpm > 0.0f && this->beatStrength >= neededStrength;
}

trackInfo analyse(std::span<const float> mono, int rate)
{
    trackInfo info;
    if (rate <= 0 || mono.empty())
        return info;
    info.analysed = true;
    info.seconds = (double) mono.size() / rate;
    double square = 0.0;
    for (float s : mono)
        square += (double) s * s;
    const double rms = std::sqrt(square / (double) mono.size());
    info.loudnessDb = rms > 1e-9 ? (float) (20.0 * std::log10(rms)) : -90.0f;
    // the end of a quiet intro: the first second near the song's average level
    for (std::size_t start = 0; start < mono.size(); start += (std::size_t) rate) {
        const std::size_t end = std::min(mono.size(), start + (std::size_t) rate);
        double s = 0.0;
        for (std::size_t i = start; i < end; i++)
            s += (double) mono[i] * mono[i];
        const double blockRms = std::sqrt(s / (double) (end - start));
        if (blockRms > 1e-9 && 20.0 * std::log10(blockRms) >= info.loudnessDb - entryBelowDb) {
            info.entry = (double) start / rate;
            break;
        }
    }

    const auto spectrum = measureSpectrum(mono, rate);
    info.brightness = spectrum.brightness;
    if (spectrum.flux.empty())
        return info;
    info.flux = std::accumulate(spectrum.flux.begin(), spectrum.flux.end(), 0.0f) / (float) spectrum.flux.size();
    const double frameRate = (double) rate / hop;
    const auto o = onsets(spectrum.flux, (int) (baselineSeconds * frameRate));
    const float mean = std::accumulate(o.begin(), o.end(), 0.0f) / (float) o.size();
    const double rough = roughPeriod(o, frameRate);
    if (rough <= 0.0 || mean <= 0.0f)
        return info;

    // the exact tempo and phase: the grid that collects the most onsets over the whole song
    double bestPeriod = rough;
    double bestPhase = 0.0;
    float bestScore = -1.0f;
    for (float k = -refineSpan; k <= refineSpan + 1e-6f; k += refineStep) {
        const double period = rough * (1.0 + k);
        for (double phase = 0.0; phase < period; phase += 0.5) {
            const float s = gridMean(o, phase, period);
            if (s > bestScore) {
                bestScore = s;
                bestPeriod = period;
                bestPhase = phase;
            }
        }
    }
    info.bpm = (float) (frameRate * 60.0 / bestPeriod);
    info.beatStrength = std::clamp((bestScore - mean) / (bestScore + mean), 0.0f, 1.0f);
    // the strongest of every four beats starts the bar
    int downbeat = 0;
    float strongest = -1.0f;
    for (int j = 0; j < 4; j++) {
        const float s = gridMean(o, bestPhase + j * bestPeriod, 4.0 * bestPeriod);
        if (s > strongest) {
            strongest = s;
            downbeat = j;
        }
    }
    // a frame's flux peaks when the onset is three quarters into its window
    const double beat = info.beatSeconds();
    const double first = (bestPhase * hop + windowSize * 0.75) / rate;
    info.firstBeat = std::fmod(first, beat);
    info.firstDownbeat = std::fmod(first + downbeat * beat, 4.0 * beat);
    const double bar = 4.0 * beat;
    info.entry = info.firstDownbeat + std::ceil(std::max(0.0, info.entry - info.firstDownbeat) / bar) * bar;
    return info;
}

std::optional<trackInfo> analyseFile(const std::string &path, std::stop_token stop)
{
    SF_INFO sfInfo{};
    std::unique_ptr<SNDFILE, int (*)(SNDFILE *)> file(sf_open(path.c_str(), SFM_READ, &sfInfo), sf_close);
    if (!file || sfInfo.channels < 1 || sfInfo.samplerate < 1)
        return std::nullopt;
    const int factor = std::max(1, (int) std::lround((double) sfInfo.samplerate / analysisRate));
    std::vector<float> mono;
    if (sfInfo.frames > 0)
        mono.reserve((std::size_t) (sfInfo.frames / factor + 1));
    std::vector<float> chunk(4096 * (std::size_t) sfInfo.channels);
    double acc = 0.0;
    int inAcc = 0;
    sf_count_t frames = 0;
    for (;;) {
        if (stop.stop_requested())
            return std::nullopt;
        const sf_count_t got = sf_readf_float(file.get(), chunk.data(), 4096);
        if (got <= 0)
            break;
        frames += got;
        for (sf_count_t f = 0; f < got; f++) {
            float s = 0.0f;
            for (int ch = 0; ch < sfInfo.channels; ch++)
                s += chunk[(std::size_t) (f * sfInfo.channels + ch)];
            acc += s / (float) sfInfo.channels;
            if (++inAcc == factor) {
                mono.push_back((float) (acc / factor));
                acc = 0.0;
                inAcc = 0;
            }
        }
    }
    if (mono.empty())
        return std::nullopt;
    auto info = analyse(mono, (int) std::lround((double) sfInfo.samplerate / factor));
    info.seconds = (double) frames / sfInfo.samplerate; // the playing length, not the decimated one
    return info;
}

} // namespace goe::dj
