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

#include "roam.h"
#include "bElem.h"
#include "randomStreams.h"

namespace goe::roam {
namespace {
int distance2(coords a, coords b)
{
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}
} // namespace

void turn(const std::shared_ptr<bElem> &body, dir::direction d)
{
    body->getStats()->setMyDirection(d);
    body->getStats()->setFacing(d);
    body->getStats()->setWaiting(GoEConstants::_mov_delay);
}

bool step(const std::shared_ptr<bElem> &body, dir::direction d)
{
    if (!body->moveInDirection(d))
        return false;
    body->getStats()->setMyDirection(d);
    body->getStats()->setFacing(d);
    body->getStats()->setWaiting(GoEConstants::_mov_delay);
    return true;
}

bool followWall(const std::shared_ptr<bElem> &body, hand side, coords centre, int radius)
{
    coords me = body->getStats()->getMyPosition();
    // a body already outside its circle (a loaded game, a push) roams freely until it is back
    const bool bounded = radius > 0 && !(centre == NOCOORDS) && distance2(me, centre) <= radius * radius;
    auto inside = [&](coords offset) {
        return !bounded || distance2(coords(me.x + offset.x, me.y + offset.y), centre) <= radius * radius;
    };
    auto solid = [&](coords offset) {
        auto e = body->getElementInDirection(offset);
        return !inside(offset) || !e || !e->getAttrs()->isSteppable();
    };
    auto go = [&](dir::direction d) { return inside(dir::directionToCoordsMap[(int) d]) && step(body, d); };
    dir::direction cdir = body->getStats()->getMyDirection();
    if (cdir == dir::direction::NODIRECTION)
        cdir = dir::direction::UP;
    // the wall side, and the other one
    const dir::direction wallSide = side == hand::right ? rightOf(cdir) : leftOf(cdir);
    const dir::direction openSide = side == hand::right ? leftOf(cdir) : rightOf(cdir);
    // when the wall at our side just ended, go around its corner; otherwise go straight, then
    // towards the wall, then away from it, and turn back only in a dead end. In an open room this
    // walks straight until it meets a wall, instead of circling. Now and then it lets go of a
    // corner, so it does not circle a pillar forever, but finds the maze's walls.
    coords toWall = dir::directionToCoordsMap[(int) wallSide];
    coords toBack = dir::directionToCoordsMap[(int) behind(cdir)];
    coords backWall(toWall.x + toBack.x, toWall.y + toBack.y);
    const bool letGo = goe::rng::gameplay()() % 23 == 0;
    if (!letGo && !solid(toWall) && solid(backWall) && go(wallSide))
        return true;
    for (auto d : {cdir, wallSide, openSide, behind(cdir)})
        if (go(d))
            return true;
    turn(body, wallSide);
    return true;
}
} // namespace goe::roam
