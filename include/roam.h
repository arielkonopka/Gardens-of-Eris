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

#ifndef ROAM_H
#define ROAM_H
#include "commons.h"
#include <memory>

class bElem;

/**
 * How creatures move about the maze when nothing else calls them. Monsters and every drone
 * controller walk through these helpers, so nothing that moves stands still for no reason: an
 * idle creature roams the maze along its walls.
 */
namespace goe::roam {
/// which side a creature keeps the wall on while it roams
enum class hand { right, left };

constexpr dir::direction leftOf(dir::direction d) { return (dir::direction) (((int) d + 1) % 4); }
constexpr dir::direction rightOf(dir::direction d) { return (dir::direction) (((int) d + 3) % 4); }
constexpr dir::direction behind(dir::direction d) { return (dir::direction) (((int) d + 2) % 4); }

/// turns the body to face d and makes it wait, without moving
void turn(const std::shared_ptr<bElem> &body, dir::direction d);
/// moves the body in d, facing that way, and makes it wait; false when blocked
bool step(const std::shared_ptr<bElem> &body, dir::direction d);
/**
 * one move of a walk along the maze's walls (the wall follower rule, keeping the wall on the
 * given side). Now and then it lets go of a wall, so it does not circle a pillar forever.
 * When radius > 0, cells outside the circle of radius around centre count as walls, so the
 * walk keeps inside it. Always does something: a step, or a turn when it is walled in.
 */
bool followWall(const std::shared_ptr<bElem> &body, hand side = hand::right, coords centre = NOCOORDS,
                int radius = 0);
} // namespace goe::roam

#endif // ROAM_H
