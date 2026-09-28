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

#include "securityCamera.h"
#include "difficulty.h"
#include "chamber.h"
#include "elementFactory.h"
#include "patrollingDrone.h"
#include "player.h"
#include "puppetMasterGuardian.h"
#include <cstdlib>

bool securityCamera::additionalProvisioning(int subtype)
{
    if (!bElem::additionalProvisioning(subtype))
        return false;
    this->registerLiveElement(shared_from_this());
    return true;
}

int securityCamera::getType() const
{
    return bElemTypes::_securityCamera;
}

bool securityCamera::lineOfSight(std::shared_ptr<chamber> board, coords from, coords to)
{
    // Bresenham's line; only the cells strictly between the two ends must be see-through
    int dx = std::abs(to.x - from.x), dy = -std::abs(to.y - from.y);
    int sx = from.x < to.x ? 1 : -1, sy = from.y < to.y ? 1 : -1;
    int err = dx + dy;
    coords c = from;
    while (true) {
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            c.x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            c.y += sy;
        }
        if (c == to)
            return true;
        auto e = board->getElement(c);
        // drones do not block the view, so guardians never hide the player from their camera
        if (!e || !(e->getAttrs()->isSteppable() || e->getType() == bElemTypes::_patrollingDrone))
            return false;
    }
}

void securityCamera::spawnGuardians()
{
    this->guardiansSpawned = true;
    auto board = this->getBoard();
    coords me = this->getStats()->getMyPosition();
    const int wanted = difficulty::guardianCount(difficulty::current());
    int placed = 0;
    // fill free cells in growing squares around the camera
    for (int r = 1; r <= 4 && placed < wanted; r++)
        for (int x = me.x - r; x <= me.x + r && placed < wanted; x++)
            for (int y = me.y - r; y <= me.y + r && placed < wanted; y++) {
                if (std::max(std::abs(x - me.x), std::abs(y - me.y)) != r)
                    continue;
                auto cell = board->getElement(coords(x, y));
                if (!cell || !cell->getAttrs()->isSteppable() || cell->getType() != bElemTypes::_floorType)
                    continue;
                auto drone = elementFactory::generateAnElement<patrollingDrone>(board, 0);
                drone->stepOnElement(cell);
                auto brain = std::static_pointer_cast<puppetMasterGuardian>(
                    puppetMasterFR::create(board, puppetMasterFR::guardian));
                brain->guard(std::static_pointer_cast<securityCamera>(shared_from_this()));
                drone->attachController(brain);
                placed++;
            }
}

bool securityCamera::mechanics()
{
    if (!bElem::mechanics())
        return false;
    if (!this->guardiansSpawned)
        this->spawnGuardians();
    auto prey = player::getActivePlayer();
    if (prey && prey->getBoard() == this->getBoard()) {
        coords me = this->getStats()->getMyPosition(), p = prey->getStats()->getMyPosition();
        int dx = p.x - me.x, dy = p.y - me.y;
        const int sight = difficulty::cameraSight(difficulty::current());
        if (dx * dx + dy * dy <= sight * sight
            && securityCamera::lineOfSight(this->getBoard(), me, p)) {
            this->alertAt = p;
            this->alertNumber++;
        }
    }
    this->getStats()->setWaiting(GoEConstants::_mov_delay);
    return true;
}
