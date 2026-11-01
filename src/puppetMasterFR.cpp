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

#include "puppetMasterFR.h"
#include "gameEvents.h"
#include "elementFactory.h"
#include "puppetMasterCollector.h"
#include "puppetMasterHunter.h"
#include "puppetMasterWallFollower.h"
#include "puppetMasterGuardian.h"
#include "puppetMasterHound.h"
#include "viewPoint.h"
#include "lineOfSight.h"
#include "motion.h"
#include "player.h"
#include "chamber.h"
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <vector>

std::shared_ptr<puppetMasterFR> puppetMasterFR::create(std::shared_ptr<chamber> board, int subtype)
{
    switch (subtype) {
    case collector:
        return elementFactory::generateAnElement<puppetMasterCollector>(board, subtype);
    case hunter:
        return elementFactory::generateAnElement<puppetMasterHunter>(board, subtype);
    case wallFollower:
        return elementFactory::generateAnElement<puppetMasterWallFollower>(board, subtype);
    case guardian:
        return elementFactory::generateAnElement<puppetMasterGuardian>(board, subtype);
    case hound:
        return elementFactory::generateAnElement<puppetMasterHound>(board, subtype);
    default:
        return elementFactory::generateAnElement<puppetMasterFR>(board, subtype);
    }
}

int puppetMasterFR::getType() const
{
    return bElemTypes::_puppetMasterType;
}

bool puppetMasterFR::collectibleBy(const bElem &who) const
{
    // a drone that walked into a loose controller used to pick it up and drop it straight back
    // into the same cell, again every turn, so it stood there for good
    return who.getType() != bElemTypes::_patrollingDrone;
}

void puppetMasterFR::onAttach(std::shared_ptr<bElem> body)
{
    // the plain patrol controller turns its body into a roaming camera
    if (this->getAttrs()->getSubtype() == patrol)
        viewPoint::get_instance().addViewPoint(body);
}

dir::direction puppetMasterFR::towards(coords from, coords to)
{
    int dx = to.x - from.x, dy = to.y - from.y;
    // a diagonal only when the way is closer to it than to a side (within half a cell per cell)
    if (std::abs(dx) >= 2 * std::abs(dy))
        dy = 0;
    else if (std::abs(dy) >= 2 * std::abs(dx))
        dx = 0;
    return dir::towards(coords(dx, dy));
}

bool puppetMasterFR::touching(const chamber &board, coords a, coords b)
{
    const coords d(b.x - a.x, b.y - a.y);
    return std::max(std::abs(d.x), std::abs(d.y)) == 1 && motion::cornerOpen(board, a, dir::towards(d));
}

bool puppetMasterFR::bite(std::shared_ptr<bElem> body, std::shared_ptr<bElem> prey, int damage)
{
    coords b = body->getStats()->getMyPosition(), p = prey->getStats()->getMyPosition();
    auto board = body->getBoard();
    if (!board || !touching(*board, b, p))
        return false;
    goe::roam::turn(body, towards(b, p));
    goe::events::harming by(body.get());
    prey->hurt(damage);
    body->getStats()->setWaiting(GoEConstants::_mov_delay * 2);
    return true;
}

std::shared_ptr<bElem> puppetMasterFR::lookout(std::shared_ptr<bElem> body, int range)
{
    auto prey = player::getActivePlayer();
    if (!prey || prey->getBoard() != body->getBoard())
        return nullptr;
    coords me = body->getStats()->getMyPosition(), p = prey->getStats()->getMyPosition();
    if (distance2(me, p) > range * range || !goe::sight::clear(body->getBoard(), me, p))
        return nullptr;
    this->lastSeen = p;
    return prey;
}

bool puppetMasterFR::followTrail(std::shared_ptr<bElem> body, bool preyInSight, coords centre, int radius)
{
    if (this->lastSeen == NOCOORDS)
        return false;
    coords me = body->getStats()->getMyPosition();
    if (distance2(this->lastSeen, centre) > radius * radius) {
        this->lastSeen = NOCOORDS; // out of our reach
        return false;
    }
    auto board = body->getBoard();
    if (this->lastSeen == me || (board && touching(*board, me, this->lastSeen))) {
        if (preyInSight && !(this->lastSeen == me)) {
            goe::roam::turn(body, towards(me, this->lastSeen));
            return true;
        }
        this->lastSeen = NOCOORDS; // checked, nobody here
        return false;
    }
    auto d = pathTowards(body, this->lastSeen, centre, radius);
    if (d == dir::direction::NODIRECTION) {
        this->lastSeen = NOCOORDS; // cannot get there
        return false;
    }
    if (!goe::roam::step(body, d))
        goe::roam::turn(body, d); // something walked into the way; try again next time
    return true;
}

bool puppetMasterFR::drive(std::shared_ptr<bElem> body)
{
    // the plain patrol roams the maze keeping the wall on its left, so it does not walk the same
    // way as the wall follower
    return goe::roam::followWall(body, goe::roam::hand::left);
}

dir::direction puppetMasterFR::pathTowards(std::shared_ptr<bElem> body, coords goal, coords centre, int radius)
{
    auto board = body->getBoard();
    if (!board)
        return dir::direction::NODIRECTION;
    coords start = body->getStats()->getMyPosition();
    // search a box around the circle; cells the board does not have are never steppable
    int x0 = centre.x - radius, y0 = centre.y - radius;
    int x1 = centre.x + radius, y1 = centre.y + radius;
    if (start.x < x0 || start.x > x1 || start.y < y0 || start.y > y1)
        return dir::direction::NODIRECTION;
    int w = x1 - x0 + 1, h = y1 - y0 + 1;
    // for each visited cell, the direction of the first step that led there
    std::vector<int8_t> firstStep(w * h, -1);
    auto at = [&](coords c) -> int8_t & { return firstStep[(c.y - y0) * w + (c.x - x0)]; };
    std::deque<coords> queue{start};
    at(start) = (int8_t) dir::direction::NODIRECTION;
    while (!queue.empty()) {
        coords c = queue.front();
        queue.pop_front();
        for (const auto d : dir::ways) {
            coords n = c + dir::dirToCoords(d);
            if (n.x < x0 || n.x > x1 || n.y < y0 || n.y > y1 || at(n) != -1
                || distance2(n, centre) > radius * radius || !motion::cornerOpen(*board, c, d))
                continue;
            int8_t step = (c == start) ? (int8_t) d : at(c);
            if (n == goal)
                return (c == start) ? dir::direction::NODIRECTION : (dir::direction) step;
            auto e = board->getElement(n);
            if (!e || !e->getAttrs()->isSteppable())
                continue;
            at(n) = step;
            queue.push_back(n);
        }
    }
    return dir::direction::NODIRECTION;
}
