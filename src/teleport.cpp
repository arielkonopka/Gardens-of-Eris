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
#include "teleport.h"
std::once_flag teleport::_onceFlag;

std::vector<std::weak_ptr<teleport>> teleport::allTeleporters;



bool teleport::additionalProvisioning(int value,std::shared_ptr<teleport> t)
{
    if(!bElem::additionalProvisioning(value,t))
        return false;
    this->connectionsMade = false;
    if (teleport::allTeleporters.empty() && t->getAttrs()->getSubtype()==0)
    {
        t->getStats()->setFacing(dir::direction::LEFT);
        t->getStats()->setMyDirection(t->getStats()->getFacing());
   }

    teleport::allTeleporters.push_back(t);
    return true;
}



/* here we will try to teleport an object to the becon connected to this teleporter. if the becon is not yet established, randomly choose one */
bool teleport::interact(std::shared_ptr<bElem> who)
{
    bool r=true;
    if (!bElem::interact(who) || this->getStats()->getMyDirection()==dir::direction::LEFT )
        return false;
    if (this->theOtherEnd.expired())
        this->createConnectionsWithinSUbtype();

    this->playSound("Teleport", "Teleporting");
    if (!this->theOtherEnd.expired())
        r=this->theOtherEnd.lock()->teleportIt(who);
    else
        r=this->teleportIt(who);
    return r;
}

/***
 * @brief Find a partner for a teleport
 * this methid will select a random counterpart for our teleport, and set it up. the counterpart will become a receiver, in case it is a teleporter of subtype==0,
 * the music started by it will be stopped.
 * @return always true;
 */
bool teleport::createConnectionsWithinSUbtype()
{
    /// We do this only once, as soon as the first level is created. we can get away with this construct, because we know, that the first mirror is a receiver, and will be inactive.
    /// therefire we have to remove it from all teleporters vector.
    std::call_once(teleport::_onceFlag,[](){ if (!teleport::allTeleporters.empty()) teleport::allTeleporters.erase(teleport::allTeleporters.begin());});
    std::shared_ptr<teleport> tmpt,tmpt2;
    std::erase_if(teleport::allTeleporters, [&](const std::weak_ptr<teleport>& wp) {
        if (auto sp = wp.lock()) {
            return sp->getStats()->getInstanceId() == this->getStats()->getInstanceId();
        }
        return true;
    });
    candidates.clear();
    for (const auto& tel : teleport::allTeleporters) {
        if (tel.expired()) continue;
        if (auto t = tel.lock()) {
            if (t->getAttrs()->getSubtype() == this->getAttrs()->getSubtype() && t->getStats()->getInstanceId() != this->getStats()->getInstanceId()) {
                candidates.push_back(t);
            }
        }
    }
    if(candidates.size()>2 && this->getAttrs()->getSubtype()>0)
        for(int c=0; c<5555; c++)
        {
            int p=bElem::randomNumberGenerator()%(candidates.size()-2);
            std::shared_ptr<teleport> t=candidates[p];
            candidates[p]=candidates[p+1];
            candidates[p+1]=t;
        }
    if(!candidates.empty()) {
        tmpt= candidates[0];
        this->theOtherEnd=tmpt;
        tmpt2 = std::dynamic_pointer_cast<teleport>(shared_from_this()); 
        std::erase_if(teleport::allTeleporters, [&](const std::weak_ptr<teleport> &wp) {
            if (auto sp = wp.lock()) {
                return sp->getStats()->getInstanceId() == tmpt->getStats()->getInstanceId();
            }
            return true;
        });
        tmpt->getStats()->setFacing(dir::direction::LEFT);
        tmpt->getStats()->setMyDirection(tmpt->getStats()->getFacing());
        tmpt->theOtherEnd = tmpt2;
        soundManager::getInstance()->pauseSong(tmpt->getStats()->getInstanceId());
        this->candidates.clear();
        return true;
    }
    return false;
}

int teleport::getType() const
{
    return _teleporter;
}

// Teleport to this becon
bool teleport::teleportIt(std::shared_ptr<bElem> who)
{
    if (this->getStats()->isWaiting() || !this->getBoard())
        return false;
    this->getStats()->setWaiting(50);
    who->getStats()->setTelInProgress(_teleportationTime);
    if (who->getStats()->getSteppingOn() != nullptr)
        who->getStats()->getSteppingOn()->getStats()->setTelInProgress(_teleportationTime);
    std::vector<std::shared_ptr<bElem>> spots;
    for (auto d:dir::allDirections)
    {
        if (this->isSteppableDirection(d))
            spots.push_back(this->getElementInDirection(d));
    }
    if(spots.empty())
        this->interact(who);
    else
    {
        std::shared_ptr<bElem> spot = spots[this->randomNumberGenerator() % spots.size()];
        who->stepOnElement(spot);
        return true;
    }

        this->interact(who);
    return false;
}

bool teleport::stepOnAction(bool step, std::shared_ptr<bElem>who)
{
    bElem::stepOnAction(step,who);
    this->playSound("Teleport", "HummingSound");
    if(this->getStats()->getMyDirection()==dir::direction::LEFT)
        return false;
    if(step && !who->getStats()->isTeleporting() && !this->getStats()->isTeleporting())
    {
        this->getStats()->setWaiting(_teleportStandTime);
        this->registerLiveElement(shared_from_this());
    }
    else
    {
        if(this->getStats()->hasActivatedMechanics())
            this->deregisterLiveElement(this->getStats()->getInstanceId());
    }
    return true;
}




bool teleport::mechanics()
{
    if(!bElem::mechanics())
        return false;
    if (!this->getStats()->isWaiting() && this->getStats()->hasParent() && this->getStats()->getMyDirection()!=dir::direction::LEFT)
    {
            this->interact(this->getStats()->getStandingOn().lock());
            this->deregisterLiveElement(this->getStats()->getInstanceId());
    };
    return true;
}

bool teleport::removeFromAllTeleporters()
{
    for (unsigned int c = 0; c < teleport::allTeleporters.size(); c++)
    {
        std::shared_ptr<teleport> t=teleport::allTeleporters[c].lock();
        if (!teleport::allTeleporters[c].expired() && t->getAttrs()->getSubtype() == this->getAttrs()->getSubtype())
        {
            t->connectionsMade = false;
        }
    }
    for (unsigned int c = 0; c < teleport::allTeleporters.size();)
    {
        std::shared_ptr<teleport> t=teleport::allTeleporters[c].lock();
        if (teleport::allTeleporters[c].expired() || t->getStats()->getInstanceId() == this->getStats()->getInstanceId())
        {
            teleport::allTeleporters.erase(teleport::allTeleporters.begin() + c);
        }
        else
        {
            c++;
        }
    }
    return true;
}

oState teleport::disposeElement()
{
    this->removeFromAllTeleporters();
    return bElem::disposeElement();
}

oState teleport::disposeElementUnsafe()
{
    this->removeFromAllTeleporters();
    return bElem::disposeElementUnsafe();
}

bool teleport::stepOnElement(std::shared_ptr<bElem> step) {
    if(!bElem::stepOnElement(step))
        return false;
    if(this->getAttrs()->getSubtype()==0)
    {
        soundManager::getInstance()->setupSong(this->getStats()->getInstanceId(), 1,
                                           {(float) this->getStats()->getMyPosition().x,
                                            (float) this->getStats()->getMyPosition().y, 0.0f},
                                           this->getBoard()->getInstanceId(), true);

        if(this->getStats()->getMyDirection()==dir::direction::LEFT)
            soundManager::getInstance()->pauseSong(this->getStats()->getInstanceId());
    }
    return true;
}


