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

std::vector<std::weak_ptr<teleport>> teleport::allTeleporters;



bool teleport::additionalProvisioning(int value,std::shared_ptr<teleport> t)
{
    if(!bElem::additionalProvisioning(value,t))
        return false;
    this->connectionsMade = false;
    if (teleport::allTeleporters.empty())
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
    if (!bElem::interact(who) )
        return false;
    if (!this->theOtherEnd)
        this->createConnectionsWithinSUbtype();
    if (this->getStats()->getMyDirection()==dir::direction::LEFT)
        return r;
    this->playSound("Teleport", "Teleporting");
    if (this->theOtherEnd)
        r=this->theOtherEnd->teleportIt(who);
    else
        if(!this->candidates.empty())
        {
            r=this->candidates[0]->teleportIt(who);
        }
        else
            r=this->teleportIt(who);
    return r;
}

//// ChangeME to something cool!
bool teleport::createConnectionsWithinSUbtype()
{
    for (unsigned int c = 0; c < teleport::allTeleporters.size(); c++)
    {
        std::shared_ptr<teleport> t=teleport::allTeleporters[c].lock();
        if (!teleport::allTeleporters[c].expired() && t->getAttrs()->getSubtype() == this->getAttrs()->getSubtype())
        {

            candidates.push_back(t);

        }
    }

    if(candidates.size()>2 && this->getAttrs()->getSubtype()>0)
        for(int c=0; c<5555; c++)
        {
            int p=bElem::randomNumberGenerator()%(candidates.size()-1);
            std::shared_ptr<teleport> t=candidates[p];
            candidates[p]=candidates[p+1];
            candidates[p+1]=t;

        }

    for (unsigned int c = 0; c < candidates.size() - 1; c++)
    {
        if(c%2==0) {
            candidates[c]->getStats()->setFacing(dir::direction::LEFT );
        } else
        {
            candidates[c]->getStats()->setFacing(dir::direction::UP);
        }
        candidates[c]->getStats()->setMyDirection(candidates[c]->getStats()->getFacing());
        candidates[c]->theOtherEnd = candidates[c + 1];
    }
    // candidates[candidates.size() - 1]->theOtherEnd = candidates[0];
    return true;
}
////////////////

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
        soundManager::getInstance()->setupSong(this->getStats()->getInstanceId(),1, {(float)this->getStats()->getMyPosition().x,(float)this->getStats()->getMyPosition().y, 0.0f},this->getBoard()->getInstanceId(),true);

    return bElem::stepOnElement(step);
}


