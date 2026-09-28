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

#include "puppetMasterHound.h"
#include "chamber.h"
#include "difficulty.h"
#include "elementFactory.h"
#include "gameClock.h"
#include "patrollingDrone.h"
#include "player.h"

bool puppetMasterHound::drive(std::shared_ptr<bElem> body)
{
    auto prey = player::getActivePlayer();
    if (!prey || prey->getBoard() != body->getBoard())
        return this->wander(body);
    coords p = prey->getStats()->getMyPosition();
    if (this->home == NOCOORDS)
        this->home = difficulty::areaOf(p);
    // the player got away: the hound gives up
    if (!(difficulty::areaOf(p) == this->home))
        return body->kill();
    if (this->bite(body, prey, biteDamage))
        return true;
    auto d = pathTowards(body, p, body->getStats()->getMyPosition(), searchRadius);
    if (d != dir::direction::NODIRECTION && this->step(body, d))
        return true;
    return this->wander(body);
}

void puppetMasterHound::watch()
{
    struct campState
    {
        std::weak_ptr<chamber> board;
        coords area = NOCOORDS;
        unsigned int since = 0;
        std::weak_ptr<bElem> hound;
    };
    static campState camp;
    auto prey = player::getActivePlayer();
    if (!prey || !prey->getBoard())
        return;
    coords area = difficulty::areaOf(prey->getStats()->getMyPosition());
    // a loaded game may have turned the clock back
    if (camp.board.lock() != prey->getBoard() || !(camp.area == area) || gameClock::now() < camp.since) {
        // a new place: the clock starts again; a hound already out gives up by itself
        camp.board = prey->getBoard();
        camp.area = area;
        camp.since = gameClock::now();
        return;
    }
    const int patience = difficulty::houndPatience(difficulty::of(prey));
    if (patience == 0 || gameClock::now() - camp.since < (unsigned int) patience)
        return;
    if (auto h = camp.hound.lock(); h && !h->getStats()->isDisposed())
        return;
    camp.hound = puppetMasterHound::release(prey);
    camp.since = gameClock::now();
}

std::shared_ptr<bElem> puppetMasterHound::release(const std::shared_ptr<bElem> &prey)
{
    auto board = prey->getBoard();
    coords p = prey->getStats()->getMyPosition();
    // a free floor cell on the square of cells spawnDistance away, tried at random places
    for (int attempt = 0; attempt < 64; attempt++) {
        int along = (int) (bElem::randomNumberGenerator() % (2 * spawnDistance + 1)) - spawnDistance;
        int side = (int) (bElem::randomNumberGenerator() % 4);
        coords c = side == 0   ? coords(p.x + along, p.y - spawnDistance)
                   : side == 1 ? coords(p.x + along, p.y + spawnDistance)
                   : side == 2 ? coords(p.x - spawnDistance, p.y + along)
                               : coords(p.x + spawnDistance, p.y + along);
        auto cell = board->getElement(c);
        if (!cell || !cell->getAttrs()->isSteppable() || cell->getType() != bElemTypes::_floorType)
            continue;
        // subtype 1 is the red hound look in the skin
        auto drone = elementFactory::generateAnElement<patrollingDrone>(board, 1);
        drone->stepOnElement(cell);
        auto brain = std::static_pointer_cast<puppetMasterHound>(puppetMasterFR::create(board, puppetMasterFR::hound));
        brain->home = difficulty::areaOf(p);
        drone->attachController(brain);
        return drone;
    }
    return nullptr;
}
