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
#ifndef MUSICCUES_H
#define MUSICCUES_H

#include "musicEvents.h"

/**
 * @brief The game's side of the music-control interface: what the music should know about
 * the danger the player is in.
 *
 * Cameras and guardians report what they do; the presenter asks now() once a tick and hands
 * the answer to the sound system. The musician never looks at game objects itself.
 *
 * - a camera that sees the player, or a guardian chasing them: alert, for difficulty::musicAlertTicks
 *   after the last report;
 * - a guardian within difficulty::musicDangerDistance that sees the player, or one fighting them:
 *   danger, for difficulty::musicDangerTicks after the last report.
 *
 * Reports are counted in game ticks (gameClock), so a paused or saved game does not age them.
 * Safe from any thread.
 */
namespace goe::music::cues {
/// a camera saw the player
void sighted();
/// a guardian is chasing the player, or going to where they were seen
void chased();
/// a guardian is about to hurt the player
void endangered();
/// the strongest situation still fresh at this tick
goe::musician::situation now();
/// forgets every report (a new world)
void reset();
} // namespace goe::music::cues

#endif // MUSICCUES_H
