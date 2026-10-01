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

#ifndef LINEOFSIGHT_H
#define LINEOFSIGHT_H
#include "commons.h"
#include <memory>

class bElem;
class chamber;

/**
 * What elements can see. Every element that watches for the player (cameras, guardians, hunters,
 * hounds) looks through these helpers, so none of them sees through walls.
 */
namespace goe::sight {
/// true when e blocks the view: walls, brick clusters, bunkers, teleporters, closed doors, and
/// cells the board does not have. Floors, items and creatures do not block it.
bool opaque(const std::shared_ptr<bElem> &e);
/**
 * true when nothing opaque lies on the straight line between the two cells (Bresenham's line).
 * The two end cells themselves never block. A diagonal step does not squeeze between two opaque
 * cells that touch at their corners.
 */
bool clear(const std::shared_ptr<chamber> &board, coords from, coords to);
} // namespace goe::sight

#endif // LINEOFSIGHT_H
