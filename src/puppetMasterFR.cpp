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
#include "viewPoint.h"
#include "chamber.h"
#include <algorithm>
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
    int roulette = this->randomNumberGenerator() % 555;
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
    this->turn(body, (this->randomNumberGenerator() % 2 == 0) ? right : left);
    return true;
}

dir::direction puppetMasterFR::pathTowards(std::shared_ptr<bElem> body, coords goal, coords centre, int radius)
{
    auto board = body->getBoard();
    if (!board)
        return dir::direction::NODIRECTION;
    coords start = body->getStats()->getMyPosition();
    // search a box around the circle, clipped to the board
    int x0 = std::max(0, centre.x - radius), y0 = std::max(0, centre.y - radius);
    int x1 = std::min(board->getSize().x - 1, centre.x + radius);
    int y1 = std::min(board->getSize().y - 1, centre.y + radius);
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
