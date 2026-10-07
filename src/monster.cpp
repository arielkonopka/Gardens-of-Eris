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
#include "monster.h"
#include "roam.h"

bool monster::additionalProvisioning(int subtype)
{
    if (!bElem::additionalProvisioning(subtype))
        return false;
    if (goe::rng::gameplay()() % 2 == 0) {
        this->rotA = 1;
        this->rotB = 3;
    }
    if (goe::rng::gameplay()() % 55 > 5) {
        if (goe::rng::gameplay()() % 55 > 15)
            this->weapon = elementFactory::generateAnElement<plainGun>(this->getBoard(), 1);
        else
            this->weapon = elementFactory::generateAnElement<bazooka>(this->getBoard(), 1);
        this->weapon->getAttrs()->setEnergy(((goe::rng::gameplay()() * 555) % 55) * 5);
        this->weapon->getAttrs()->setAmmo(5 * (5 + goe::rng::gameplay()() % 55));
        this->weapon->getAttrs()->setMaxEnergy(5 * 5 * 5);
        this->weapon->getStats()->setCollected(true);
        this->weapon->getStats()->setCollector(shared_from_this());
        this->weapon->getStats()->setStatsOwner(shared_from_this());
    }
    this->registerLiveElement(shared_from_this());

    return true;
}

int monster::getType() const
{
    return bElemTypes::_monster;
}

bool monster::checkNeigh()
{
    bool r = false;
    for (auto d : dir::allDirections) {
        auto e = this->getElementInDirection(d);
        if (!e)
            continue;
#ifdef _VerbousMode_
        std::cout << "  ** CHK isCollectible\n";
#endif
        // one item a turn; the next one is picked up on the next turn
        if (e->getAttrs()->isCollectible() && this->getAttrs()->canCollect() && this->collect(e)) {
            this->getStats()->setWaiting(GoEConstants::_mov_delay);
            return true;
        }
#ifdef _VerbousMode_
        std::cout << "  ** CHK isCollectible done\n";
        std::cout << "  ** CHK getType\n";
#endif
        if (e->getType() == bElemTypes::_player) {
#ifdef _VerbousMode_
            std::cout << "  *** Hurt \n";
#endif
            e->hurt(5);
#ifdef _VerbousMode_
            std::cout << "  *** Done\n";
#endif
            r = true;
            continue;
        }
#ifdef _VerbousMode_
        std::cout << "  ** CHK getType Done\n";
#endif
        if (this->getAttrs()->canCollect()) //
        {
            while (e != nullptr) // this is the "monstervision"
            {
                if (((e->getType() == bElemTypes::_player && e->getStats()->isActive())
                     || (e->getType() == bElemTypes::_patrollingDrone
                         && e->getStats()->hasActivatedMechanics()))
                    && ((this->getAttrs()->canCollect()
                         && this->getAttrs()->getInventory()->getActiveWeapon() != nullptr)
                        || this->weapon != nullptr)) {
                    // the native gun first, else the one from the inventory - surprise thing:)
                    std::shared_ptr<bElem> gun = this->weapon;
                    if (!gun)
                        gun = this->getAttrs()->getInventory()->getActiveWeapon();
                    this->getStats()->setFacing(d);
                    // a gun that cannot fire nor is charging is empty: the monster goes on its way
                    if (!gun->use(shared_from_this()) && !gun->getStats()->isWaiting())
                        break;
                    this->getStats()->setWaiting(
                        GoEConstants::_mov_delay); // will wait next couple times
                    return true;
                }
                // if it is something interesting, go and fetch it
                if (e->getType() == bElemTypes::_stash || e->getType() == bElemTypes::_rubishType
                    || (e->getType() == bElemTypes::_goldenAppleType
                        && e->getAttrs()->getSubtype() != 0)
                    || e->getAttrs()
                           ->isWeapon()) // take the dir::direction towards remainings from other objects, broken apples or guns
                {
                    // walk there now: waiting first would start the same wait again next turn,
                    // and the monster would never move. Blocked, it roams on instead.
                    return goe::roam::step(shared_from_this(), d);
                }

                // closed door? and we got a key?
                if ((e->getType() == bElemTypes::_door && !e->getAttrs()->isSteppable())
                    && this->getAttrs()->canCollect()
                    && (this->getAttrs()->getInventory()->countTokens(bElemTypes::_key,
                                                                      e->getAttrs()->getSubtype())
                        > 0)) {
                    // right in front of it, the monster unlocks the door with its key
                    if (e == this->getElementInDirection(d)) {
                        goe::roam::turn(shared_from_this(), d);
                        return e->interact(shared_from_this());
                    }
                    return goe::roam::step(shared_from_this(), d);
                }
                // we do not see behind non steppable objects
                if (!e->getAttrs()->isSteppable() || e->getElementInDirection(d) == nullptr)
                    break;
                e = e->getElementInDirection(d);
            }
        }
    }
    return r;
}
bool monster::mechanics()
{
    if (!bElem::mechanics())
        return false;
    // bites, shoots, picks up or walks to something it saw
    if (this->checkNeigh())
        return true;
    if (this->getStats()->isWaiting())
        return true;
    // nothing to do: roam the maze, each monster keeping the wall on its own side
    return goe::roam::followWall(shared_from_this(),
                                 this->rotA == 1 ? goe::roam::hand::left : goe::roam::hand::right);
}

bool monster::steppableNeigh()
{
    sNeighboorhood n = this->getSteppableNeighborhood();
    for (int c = 0; c < 8; c++) {
        if (n.steppable[c] == false)
            return false;
    }
    return true;
}
