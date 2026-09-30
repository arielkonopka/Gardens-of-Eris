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
#include "randomStreams.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

namespace goe::rng {
namespace {
engine seeded()
{
    std::random_device rd;
    std::array<seed, 4> seedData{};
    std::generate_n(seedData.data(), seedData.size(), std::ref(rd));
    std::seed_seq seq(seedData.begin(), seedData.end());
    return engine(seq);
}

std::mutex audioMutex;
std::mutex worldSeedMutex;
bool worldSeedSet = false;
seed theWorldSeed = 0;
std::atomic<seed> levelNumber{0};
/// the active generation scopes on this thread, innermost last
thread_local std::vector<std::reference_wrapper<engine>> scopes;
} // namespace

seed freshSeed()
{
    return std::random_device{}();
}

engine &saved() noexcept
{
    static engine eng = seeded();
    return eng;
}

engine &gameplay() noexcept
{
    return scopes.empty() ? saved() : scopes.back().get();
}

engine &cosmetic() noexcept
{
    static engine eng = seeded();
    return eng;
}

seed audio()
{
    static engine eng = seeded();
    std::lock_guard<std::mutex> lock(audioMutex);
    return eng();
}

seed worldSeed()
{
    std::lock_guard<std::mutex> lock(worldSeedMutex);
    if (!worldSeedSet) {
        theWorldSeed = freshSeed();
        worldSeedSet = true;
    }
    return theWorldSeed;
}

void setWorldSeed(seed s)
{
    std::lock_guard<std::mutex> lock(worldSeedMutex);
    theWorldSeed = s;
    worldSeedSet = true;
    levelNumber = 0;
}

seed nextLevelSeed()
{
    std::seed_seq seq{worldSeed(), levelNumber++};
    std::array<seed, 1> out{};
    seq.generate(out.begin(), out.end());
    return out[0];
}

seed placeSeed(int x, int y, seed salt)
{
    // two's complement bits, so negative chunk numbers give their own seeds too
    std::seed_seq seq{worldSeed(), (seed) x, (seed) y, salt};
    std::array<seed, 1> out{};
    seq.generate(out.begin(), out.end());
    return out[0];
}

generationScope::generationScope(engine &levelEngine)
{
    scopes.emplace_back(levelEngine);
}

generationScope::~generationScope()
{
    scopes.pop_back();
}
} // namespace goe::rng
