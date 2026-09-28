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
#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include "commons.h"
#include <algorithm>
#include <memory>

class bElem;
class chamber;

/**
 * @brief How hard the game is right now, and what that means for every element that scales with it.
 *
 * The difficulty D is the sum of three parts:
 * - the player level: floor(log5(shots + 1)), the "Dex" in the HUD. Every hit the player lands counts.
 * - the depth of the chamber: set by the level generator (0 for the easiest levels, 4 for the hardest).
 * - the distance: floor(log2(1 + d / distanceUnit)), where d is how far the player is from where the
 *   chamber was entered. The unit is the size of a chunk in the planned endless world, so the same
 *   rule keeps working once the world is made of chunks.
 *
 * Every tuning rule lives here, so the whole difficulty curve can be read and changed in one place.
 * D is never capped; each rule caps its own value where it stops making sense.
 */
namespace difficulty {
    /// the distance that adds one difficulty step the first time (doubling after that)
    constexpr int distanceUnit = 64;
    /// game ticks per second (the presenter's timer)
    constexpr int ticksPerSecond = 50;

    /// floor(log5(shots + 1)), computed on integers so exact powers of five are never off by one
    int playerLevel(int shots);
    /// the player level of an element, from its shooting points
    int playerLevel(const std::shared_ptr<bElem> &who);
    /// floor(log2(1 + d / distanceUnit)) for the longer axis distance d between the two cells
    int distanceLevel(coords from, coords to);
    /// the area a cell belongs to: distanceUnit x distanceUnit squares, like the planned chunks
    coords areaOf(coords cell);
    /// the difficulty for a player at their current place
    int of(const std::shared_ptr<bElem> &who);
    /// the difficulty for the active player; 0 when there is none
    int current();

    // What the difficulty changes. Each rule starts at today's value for D = 0.

    /// how far a bunker looks along each line for the player
    constexpr int bunkerRange(int d) { return std::min(10 + d, 20); }
    /// a bunker's rest between shots, shortened as D grows but never under 10 ticks
    constexpr int bunkerRest(int baseTicks, int d) { return std::max(10, baseTicks * 3 / (3 + d)); }
    /// how far a security camera sees
    constexpr int cameraSight(int d) { return std::min(8 + d / 2, 14); }
    /// how many guardian drones a camera calls up
    constexpr int guardianCount(int d) { return std::min(2 + d / 3, 5); }
    /// how much a kiki beam (a bouba) hurts at once
    constexpr int beamDamage(int d) { return std::min(GoEConstants::_radioActivityPower + d, 3 * GoEConstants::_radioActivityPower); }
    /// landmines in the level generator's pick table, for a chamber of the given depth
    constexpr int landmineCopies(int depth) { return depth * 10; }
    /**
     * how long, in ticks, the player may stay in one area before the Hound is sent;
     * 0 means never. 3 minutes at D 1, 24 s less per step, never under a minute.
     */
    constexpr int houndPatience(int d) { return d < 1 ? 0 : std::max(60, 180 - 24 * (d - 1)) * ticksPerSecond; }
}

#endif // DIFFICULTY_H
