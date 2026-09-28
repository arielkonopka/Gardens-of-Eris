/*
 * Copyright (c) 2023, Ariel Konopka
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

#include "puppetMasterWallFollower.h"

bool puppetMasterWallFollower::drive(std::shared_ptr<bElem> body)
{
    dir::direction cdir = body->getStats()->getMyDirection();
    dir::direction right = rightOf(cdir);
    auto solid = [&body](coords offset) {
        auto e = body->getElementInDirection(offset);
        return !e || !e->getAttrs()->isSteppable();
    };
    // right-hand rule: when the wall on our right just ended, go around its corner;
    // otherwise go straight, then right, then left, and turn back only in a dead end.
    // In an open room this walks straight until it meets a wall, instead of circling.
    coords toRight = dir::directionToCoordsMap[(int) right];
    coords toBack = dir::directionToCoordsMap[(int) behind(cdir)];
    coords backRight(toRight.x + toBack.x, toRight.y + toBack.y);
    if (!solid(toRight) && solid(backRight) && this->step(body, right))
        return true;
    for (auto d : {cdir, right, leftOf(cdir), behind(cdir)})
        if (this->step(body, d))
            return true;
    this->turn(body, right);
    return true;
}
