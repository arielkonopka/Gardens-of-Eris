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
#include "elementFactory.h"
#include "puppetMasterCollector.h"
#include "puppetMasterHunter.h"
#include "puppetMasterWallFollower.h"
#include "puppetMasterGuardian.h"
#include "puppetMasterHound.h"
#include "viewPoint.h"
#include "lineOfSight.h"
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

bool puppetMasterFR::collectOnAction(bool c, std::shared_ptr<bElem> who)
{
    bool r = bElem::collectOnAction(c, who);
    // a drone that walks into a loose controller does not get driven by it, it drops it again;
    // controllers are only handed over by the player (patrollingDrone::interact)
    if (c && r && who && who->getType() == bElemTypes::_patrollingDrone
        && who->getAttrs()->getInventory()->retrieveCollectibleFromInventory(
            this->getStats()->getInstanceId(), false))
        return who->dropItem(this->getStats()->getInstanceId());
    return true;
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
    return std::abs(dx) >= std::abs(dy) ? (dx > 0 ? dir::direction::RIGHT : dir::direction::LEFT)
                                        : (dy > 0 ? dir::direction::DOWN : dir::direction::UP);
}

bool puppetMasterFR::bite(std::shared_ptr<bElem> body, std::shared_ptr<bElem> prey, int damage)
{
    coords b = body->getStats()->getMyPosition(), p = prey->getStats()->getMyPosition();
    if (std::abs(p.x - b.x) + std::abs(p.y - b.y) != 1)
        return false;
    this->turn(body, towards(b, p));
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
    if (std::abs(this->lastSeen.x - me.x) + std::abs(this->lastSeen.y - me.y) <= 1) {
        if (preyInSight && !(this->lastSeen == me)) {
            this->turn(body, towards(me, this->lastSeen));
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
    if (!this->step(body, d))
        this->turn(body, d); // something walked into the way; try again next time
    return true;
}

bool puppetMasterFR::followWall(std::shared_ptr<bElem> body, coords centre, int radius)
{
    coords me = body->getStats()->getMyPosition();
    // a body already outside its circle (a loaded game, a push) patrols freely until it is back
    const bool bounded = radius > 0 && !(centre == NOCOORDS) && distance2(me, centre) <= radius * radius;
    auto inside = [&](coords offset) {
        return !bounded || distance2(coords(me.x + offset.x, me.y + offset.y), centre) <= radius * radius;
    };
    auto solid = [&](coords offset) {
        auto e = body->getElementInDirection(offset);
        return !inside(offset) || !e || !e->getAttrs()->isSteppable();
    };
    auto go = [&](dir::direction d) { return inside(dir::directionToCoordsMap[(int) d]) && this->step(body, d); };
    dir::direction cdir = body->getStats()->getMyDirection();
    dir::direction right = rightOf(cdir);
    // right-hand rule: when the wall on our right just ended, go around its corner;
    // otherwise go straight, then right, then left, and turn back only in a dead end.
    // In an open room this walks straight until it meets a wall, instead of circling. Now and
    // then it lets go of a corner, so it does not circle a pillar forever, but finds the maze's walls.
    coords toRight = dir::directionToCoordsMap[(int) right];
    coords toBack = dir::directionToCoordsMap[(int) behind(cdir)];
    coords backRight(toRight.x + toBack.x, toRight.y + toBack.y);
    const bool letGo = goe::rng::gameplay()() % 23 == 0;
    if (!letGo && !solid(toRight) && solid(backRight) && go(right))
        return true;
    for (auto d : {cdir, right, leftOf(cdir), behind(cdir)})
        if (go(d))
            return true;
    this->turn(body, right);
    return true;
}

bool puppetMasterFR::drive(std::shared_ptr<bElem> body)
{
    return this->wander(body);
}

void puppetMasterFR::turn(std::shared_ptr<bElem> body, dir::direction d)
{
    body->getStats()->setMyDirection(d);
    body->getStats()->setFacing(d);
    body->getStats()->setWaiting(GoEConstants::_mov_delay);
}

bool puppetMasterFR::step(std::shared_ptr<bElem> body, dir::direction d)
{
    if (!body->moveInDirection(d))
        return false;
    body->getStats()->setMyDirection(d);
    body->getStats()->setFacing(d);
    body->getStats()->setWaiting(GoEConstants::_mov_delay);
    return true;
}

bool puppetMasterFR::wander(std::shared_ptr<bElem> body)
{
    dir::direction cdir = body->getStats()->getMyDirection();
    dir::direction left = leftOf(cdir), right = rightOf(cdir);
    auto open = [&body](dir::direction d) {
        auto e = body->getElementInDirection(d);
        return e && e->getAttrs()->isSteppable();
    };
    int roulette = goe::rng::gameplay()() % 555;
    // now and then take a side passage, each side with the same probability
    if (roulette == 5 && open(left)) {
        this->turn(body, left);
        return true;
    }
    if (roulette == 25 && open(right)) {
        this->turn(body, right);
        return true;
    }
    if (this->step(body, cdir))
        return true;
    this->turn(body, (goe::rng::gameplay()() % 2 == 0) ? right : left);
    return true;
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
        for (int d = 0; d < 4; d++) {
            coords n(c.x + dir::directionToCoordsMap[d].x, c.y + dir::directionToCoordsMap[d].y);
            if (n.x < x0 || n.x > x1 || n.y < y0 || n.y > y1 || at(n) != -1
                || distance2(n, centre) > radius * radius)
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
