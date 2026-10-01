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
#include "musicCues.h"
#include "difficulty.h"
#include "gameClock.h"
#include <atomic>

namespace goe::music::cues {

namespace {
// the tick of the last report plus one; 0 means never
std::atomic<unsigned int> lastAlert{0};
std::atomic<unsigned int> lastDanger{0};

void note(std::atomic<unsigned int> &when)
{
    when.store(gameClock::now() + 1, std::memory_order_relaxed);
}

bool fresh(const std::atomic<unsigned int> &when, unsigned int hold)
{
    const unsigned int at = when.load(std::memory_order_relaxed);
    // a report from the future (an older save loaded) is as stale as an old one
    return at != 0 && gameClock::now() + 1 - at <= hold;
}
} // namespace

void sighted()
{
    note(lastAlert);
}

void chased()
{
    note(lastAlert);
}

void endangered()
{
    note(lastDanger);
}

goe::musician::situation now()
{
    if (fresh(lastDanger, difficulty::musicDangerTicks))
        return goe::musician::situation::danger;
    if (fresh(lastAlert, difficulty::musicAlertTicks))
        return goe::musician::situation::alert;
    return goe::musician::situation::calm;
}

void reset()
{
    lastAlert = 0;
    lastDanger = 0;
}

} // namespace goe::music::cues
