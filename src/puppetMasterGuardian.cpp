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

#include "puppetMasterGuardian.h"
#include "difficulty.h"
#include "chamber.h"
#include "elementFactory.h"
#include "plainGun.h"
#include "player.h"
#include "securityCamera.h"
#include <cstdlib>

void puppetMasterGuardian::guard(std::shared_ptr<securityCamera> cam)
{
    this->camera = cam;
    this->handledAlert = cam->getAlertNumber();
}

void puppetMasterGuardian::onAttach(std::shared_ptr<bElem> body)
{
    this->home = body->getStats()->getMyPosition();
    // a built-in gun, like the monsters have; subtype 1 never runs out of ammo
    this->gun = elementFactory::generateAnElement<plainGun>(body->getBoard(), 1);
    this->gun->getAttrs()->setEnergy(25);
    this->gun->getStats()->setCollected(true);
    this->gun->getStats()->setCollector(body);
    this->gun->getStats()->setStatsOwner(body);
}

bool puppetMasterGuardian::fight(std::shared_ptr<bElem> body, std::shared_ptr<bElem> prey)
{
    if (this->bite(body, prey, meleeDamage))
        return true;
    coords b = body->getStats()->getMyPosition(), p = prey->getStats()->getMyPosition();
    if ((p.x == b.x || p.y == b.y) && this->gun && !this->gun->getStats()->isWaiting()) {
        this->turn(body, towards(b, p));
        this->gun->use(body);
        return true;
    }
    return false;
}

bool puppetMasterGuardian::drive(std::shared_ptr<bElem> body)
{
    auto cam = this->camera.lock();
    coords centre = cam ? cam->getStats()->getMyPosition() : this->home;
    if (centre == NOCOORDS)
        centre = this->home = body->getStats()->getMyPosition();
    const int leash = securityCamera::leash;
    coords me = body->getStats()->getMyPosition();

    // 1. the player is in sight and inside the leash: fight, or chase
    auto prey = player::getActivePlayer();
    if (prey && prey->getBoard() == body->getBoard()) {
        coords p = prey->getStats()->getMyPosition();
        const int sight = difficulty::cameraSight(difficulty::current());
        if (distance2(me, p) <= sight * sight && distance2(p, centre) <= leash * leash
            && securityCamera::lineOfSight(body->getBoard(), me, p)) {
            this->target = p;
            if (this->fight(body, prey))
                return true;
            auto d = pathTowards(body, p, centre, leash);
            if (d != dir::direction::NODIRECTION && this->step(body, d))
                return true;
        }
    }
    // 2. the camera saw the player somewhere: go and check
    if (cam && cam->getAlertNumber() != this->handledAlert) {
        this->handledAlert = cam->getAlertNumber();
        if (distance2(cam->getAlertPosition(), centre) <= leash * leash)
            this->target = cam->getAlertPosition();
    }
    if (!(this->target == NOCOORDS)) {
        if (std::abs(this->target.x - me.x) + std::abs(this->target.y - me.y) <= 1) {
            this->target = NOCOORDS; // checked, nobody here
        } else {
            auto d = pathTowards(body, this->target, centre, leash);
            if (d != dir::direction::NODIRECTION && this->step(body, d))
                return true;
            if (d == dir::direction::NODIRECTION)
                this->target = NOCOORDS; // cannot get there
            else
                return true; // blocked for now, try again next time
        }
    }
    // 3. strayed to the edge of the leash: head back towards the camera
    if (distance2(me, centre) > (leash - 2) * (leash - 2)) {
        auto d = pathTowards(body, centre, centre, leash);
        if (d != dir::direction::NODIRECTION && this->step(body, d))
            return true;
    }
    // 4. nothing going on: patrol
    return this->wander(body);
}
