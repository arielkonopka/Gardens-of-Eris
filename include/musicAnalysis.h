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
#ifndef MUSICANALYSIS_H
#define MUSICANALYSIS_H

#include <optional>
#include <span>
#include <stop_token>
#include <string>

/**
 * @brief The DJ's ears: what a song sounds like, measured from its samples.
 *
 * The song is mixed down to mono at about 11 kHz. A short-time spectrum every 128 samples gives the
 * spectral flux (how much new sound starts), whose autocorrelation gives the tempo; a beat grid is
 * then fitted to the flux, and the strongest of every four beats is taken as the bar's downbeat.
 * Loudness, flux and brightness describe how intense the song is. No game header is included.
 */
namespace goe::dj {

/// changes whenever the analysis does, so remembered analyses are made again
inline constexpr int analysisVersion = 2;

struct trackInfo
{
    bool analysed = false;
    double seconds = 0.0;      ///< the song's length
    float bpm = 0.0f;          ///< beats per minute, 0 when no steady beat was found
    float beatStrength = 0.0f; ///< 0..1, how clearly the beat grid stands out of the flux
    double firstBeat = 0.0;    ///< seconds to the first beat of the grid
    double firstDownbeat = 0.0; ///< seconds to the first beat of a bar
    double entry = 0.0;        ///< where a mix brings the song in: the first downbeat once it is at full strength, past a quiet intro
    float loudnessDb = -90.0f; ///< RMS level in dBFS
    float flux = 0.0f;         ///< mean spectral flux of a frame: how busy the song is
    float brightness = 0.0f;   ///< mean spectral centroid in Hz

    /// seconds per beat at the song's own speed
    double beatSeconds() const { return this->bpm > 0.0f ? 60.0 / this->bpm : 0.0; }
    /// the beat grid is good enough to mix on
    bool hasBeat() const;
};

/// analyses mono samples at the given rate (any rate; about 11 kHz is plenty)
trackInfo analyse(std::span<const float> mono, int rate);

/// reads a sound file with libsndfile and analyses it; nothing when it cannot be read or a stop was asked
std::optional<trackInfo> analyseFile(const std::string &path, std::stop_token stop = {});

} // namespace goe::dj

#endif // MUSICANALYSIS_H
