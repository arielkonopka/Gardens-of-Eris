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
#include "musicCues.h"
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

    // 1. the guardian sees the player itself, inside the leash: fight
    auto seen = this->lookout(body, difficulty::cameraSight(difficulty::current()));
    const bool inReach = seen && distance2(seen->getStats()->getMyPosition(), centre) <= leash * leash;
    if (inReach) {
        // the music hears about it: a guardian on the player, and closer still, a direct danger
        goe::music::cues::chased();
        const int close = difficulty::musicDangerDistance;
        if (distance2(seen->getStats()->getMyPosition(), me) <= close * close)
            goe::music::cues::endangered();
    }
    if (inReach && this->fight(body, seen)) {
        goe::music::cues::endangered();
        return true;
    }
    // 2. the camera saw the player somewhere: that is where they were last seen
    if (cam && cam->getAlertNumber() != this->handledAlert) {
        this->handledAlert = cam->getAlertNumber();
        if (!seen)
            this->lastSeen = cam->getAlertPosition();
    }
    // 3. chase the player, or go and check where they were seen
    if (this->followTrail(body, seen != nullptr, centre, leash)) {
        goe::music::cues::chased();
        return true;
    }
    // 4. strayed to the edge of the leash: head back towards the camera
    if (distance2(me, centre) > (leash - 2) * (leash - 2)) {
        auto d = pathTowards(body, centre, centre, leash);
        if (d != dir::direction::NODIRECTION && this->step(body, d))
            return true;
    }
    // 5. nothing going on: patrol the walls around the camera
    return this->followWall(body, centre, leash - 2);
}
