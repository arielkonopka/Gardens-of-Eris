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





bool monster::additionalProvisioning(int subtype)
{
    if(!bElem::additionalProvisioning(subtype))
        return false;
    if (bElem::randomNumberGenerator() % 2 == 0)
    {
        this->rotA = 1;
        this->rotB = 3;
    }
    if (bElem::randomNumberGenerator() % 55 >5 )
    {
        if(bElem::randomNumberGenerator()%55>15)
            this->weapon = elementFactory::generateAnElement<plainGun>(this->getBoard(),1);
        else
            this->weapon=elementFactory::generateAnElement<bazooka>(this->getBoard(),1);
        this->weapon->getAttrs()->setEnergy(((bElem::randomNumberGenerator()*555)%55)*5);
        this->weapon->getAttrs()->setAmmo(5 * (5 + bElem::randomNumberGenerator() % 55));
        this->weapon->getAttrs()->setMaxEnergy(5*5*5);
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
    for (auto d:dir::allDirections)
    {
        auto e=this->getElementInDirection(d);
        if (!e)
            continue;
#ifdef _VerbousMode_
        std::cout<<"  ** CHK isCollectible\n";
#endif
        if (e->getAttrs()->isCollectible() && this->getAttrs()->canCollect())
        {
            this->collect(e);
            this->getStats()->setWaiting(GoEConstants::_mov_delay);
            r = true;
            /// we do that on purpose, so it would be continued in next cycle
            return true;
        }
#ifdef _VerbousMode_
        std::cout<<"  ** CHK isCollectible done\n";
        std::cout<<"  ** CHK getType\n";
#endif
        if (e->getType() == bElemTypes::_player)
        {
#ifdef _VerbousMode_
            std::cout<<"  *** Hurt \n";
#endif
            e->hurt(5);
#ifdef _VerbousMode_
            std::cout<<"  *** Done\n";
#endif
            r = true;
            continue;
        }
#ifdef _VerbousMode_
        std::cout<<"  ** CHK getType Done\n";
#endif
        if (this->getAttrs()->canCollect()) //
        {
            while (e != nullptr) // this is the "monstervision"
            {



                if (
                    ((e->getType() == bElemTypes::_player && e->getStats()->isActive()) || (e->getType() == bElemTypes::_patrollingDrone && e->getStats()->hasActivatedMechanics()))
                    &&
                    ((this->getAttrs()->canCollect() && this->getAttrs()->getInventory()->getActiveWeapon() != nullptr ) || this->weapon != nullptr)
                )
                {
                    this->getStats()->setFacing(d);
                    if (this->weapon != nullptr)
                    {
                        this->weapon->use(shared_from_this()); // shoot an object with native gun
                    }
                    else if (this->getAttrs()->getInventory()->getActiveWeapon())
                    {
                        this->getAttrs()->getInventory()->getActiveWeapon()->use(shared_from_this()); // shoot with the one from the inventory - surprise thing:)
                    }
                    this->getStats()->setWaiting(GoEConstants::_mov_delay); // will wait next couple times
                    break;
                }
                // if it is something interesting, go and fetch it
                if (e->getType() == bElemTypes::_stash || e->getType() == bElemTypes::_rubishType || (e->getType()==bElemTypes::_goldenAppleType && e->getAttrs()->getSubtype()!=0) || e->getAttrs()->isWeapon()) // take the dir::direction towards remainings from other objects, broken apples or guns
                {
                    this->getStats()->setMyDirection(d);
                    this->getStats()->setFacing(d);
                    this->inited = false;
                    this->getStats()->setWaiting(GoEConstants::_mov_delay); // will wait...
                    return false;
                }

                // closed door? and we got a key?
                if ((e->getType() == bElemTypes::_door && !e->getAttrs()->isSteppable()) && this->getAttrs()->canCollect() && (this->getAttrs()->getInventory()->countTokens(bElemTypes::_door, e->getAttrs()->getSubtype()) > 0))
                {

                    this->getStats()->setMyDirection(d);
                    this->inited = false;
                    this->getStats()->setWaiting(GoEConstants::_mov_delay);
                    return false;

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

    //dir::direction newDir = dir::direction::NODIRECTION;
    dir::direction newDir = (dir::direction)(((int)this->getStats()->getMyDirection()+1) % 4);
    if (!bElem::mechanics())
        return false;
    if(this->checkNeigh()) return true;
    if (this->getStats()->isWaiting())
        return true;
   if(this->moveInDirection(this->getStats()->getMyDirection()))
       return true;
   this->getStats()->setMyDirection(newDir);
   this->getStats()->setFacing(newDir);
   this->getStats()->setWaiting(5);
   return true;
}

bool monster::steppableNeigh()
{
    sNeighboorhood n = this->getSteppableNeighborhood();
    for (int c = 0; c < 8; c++)
    {
        if (n.steppable[c] == false)
            return false;
    }
    return true;
}
