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

#ifndef MOTION_H
#define MOTION_H
#include "commons.h"
#include <memory>

class bElem;

/**
 * Moving elements around the board. An element that moves into a cell tries, in this order:
 *  1. collect what is there, if it is a collectible and the mover can collect; the mover then
 *     steps onto the cell the collectible uncovered (user rule of 2026-09-29: collect first,
 *     step after, and only when the collect worked);
 *  2. step onto the cell, if it can be stood on;
 *  3. push what is there one cell further, if the mover can push and it can be pushed;
 *  4. interact with what is there.
 */
namespace motion {
/// moves one cell in direction d; speed is how many ticks the move takes
bool step(const std::shared_ptr<bElem> &who, dir::direction d, int speed);
/// moves one cell in direction d, pulling the movable element behind it along
bool drag(const std::shared_ptr<bElem> &who, dir::direction d, int speed);
/// collects what is in the cell, then steps onto whatever that uncovered, if it can be stood on
bool collectAndStep(const std::shared_ptr<bElem> &who, const std::shared_ptr<bElem> &collectible,
                    int speed);
} // namespace motion

#endif // MOTION_H
