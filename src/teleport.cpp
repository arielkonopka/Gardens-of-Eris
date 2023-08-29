#include "teleport.h"

videoElement::videoElementDef *teleport::vd = nullptr;
std::vector<std::weak_ptr<teleport>> teleport::allTeleporters;

teleport::teleport(std::shared_ptr<chamber> board) : teleport()
{
    this->setBoard(board);
}
teleport::teleport(std::shared_ptr<chamber> board, int newSubtype) : teleport(board)
{

}

teleport::teleport() : bElem()
{

}

bool teleport::additionalProvisioning(int value,std::shared_ptr<teleport> t)
{
    this->additionalProvisioning(value,t->getType());
    this->connectionsMade = false;
    teleport::allTeleporters.push_back(t);
    return true;

}

bool teleport::additionalProvisioning()
{
   return  this->additionalProvisioning(0,this->getType());
}

bool teleport::additionalProvisioning(int subtype,int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}


teleport::~teleport()
{

    this->removeFromAllTeleporters();
}
/* here we will try to teleport an object to the becon connected to this teleporter. if the becon is not yet established, randomly choose one */
bool teleport::interact(std::shared_ptr<bElem> who)
{
    if (bElem::interact(who) == false)
        return false;
    this->playSound("Teleport", "Teleporting");
    if (this->connectionsMade == false)
        this->createConnectionsWithinSUbtype();
    for (unsigned int c = 0; c < teleport::allTeleporters.size(); c++)
    {
        std::shared_ptr<teleport> t=teleport::allTeleporters[c].lock();
        if(!teleport::allTeleporters[c].expired())
            t->checked = false;
    }
    return this->theOtherEnd->teleportIt(who);
}

//// ChangeME to something cool!
bool teleport::createConnectionsWithinSUbtype()
{
    std::vector<std::shared_ptr<teleport>> candidates;
    for (unsigned int c = 0; c < teleport::allTeleporters.size(); c++)
    {
        std::shared_ptr<teleport> t=teleport::allTeleporters[c].lock();
        if (!teleport::allTeleporters[c].expired() && t->getAttrs()->getSubtype() == this->getAttrs()->getSubtype())
        {
            t->connectionsMade = true;
            candidates.push_back(t);
        }
    }
    for (unsigned int c = 0; c < candidates.size() - 1; c++)
    {
        candidates[c]->theOtherEnd = candidates[c + 1];
    }
    candidates[candidates.size() - 1]->theOtherEnd = candidates[0];
    return true;
}
////////////////
videoElement::videoElementDef *teleport::getVideoElementDef()
{
    return teleport::vd;
}

int teleport::getType() const
{
    return _teleporter;
}

// Teleport to this becon
bool teleport::teleportIt(std::shared_ptr<bElem> who)
{
    int dir = (int)who->getStats()->getMyDirection();
    if (this->getStats()->isWaiting())
        return false;
    this->getStats()->setWaiting(50);
    who->getStats()->setTelInProgress(_teleportationTime);
    if (who->getStats()->getSteppingOn() != nullptr)
        who->getStats()->getSteppingOn()->getStats()->setTelInProgress(_teleportationTime);
    for (int c = 0; c < 4; c++)
    {
        direction d = (direction)((dir + c) % 4);
        if (this->isSteppableDirection(d))
        {
            who->stepOnElement(this->getElementInDirection(d));
            return true;
        }
    }
    this->theOtherEnd->teleportIt(who);
    return false;
}
/*
bool teleport::isSteppable()
{
    if (this->getAttrs()->getSubtype() > 0)
    {
        if (this->getStats()->isTeleporting())
            return false;
        if (this->theOtherEnd != nullptr)
        {
            if (this->theOtherEnd->getStats()->getStandingOn() == nullptr)
            {
                return true;
            }
            else
            {
                return false; // somebody is standing on the other side
            }
        }
        else
        {
            return true;
        }
    }
    return false;
}
*/
/*
void teleport::stomp(std::shared_ptr<bElem> who)
{
    bElem::stomp(who);
    this->getStats()->setWaiting(_teleportStandTime);
    this->registerLiveElement(shared_from_this());
}

void teleport::unstomp()
{
    bElem::unstomp();
    if (this->isLiveElement())
        this->deregisterLiveElement(this->getStats()->getInstanceId());
}
*/

bool teleport::stepOnAction(bool step, std::shared_ptr<bElem>who)
{
    bElem::stepOnAction(step,who);
    if(step)
    {
        this->getStats()->setWaiting(_teleportStandTime);
        this->registerLiveElement(shared_from_this());


    } else
    {
        if(this->getStats()->hasActivatedMechanics())
           this->deregisterLiveElement(this->getStats()->getInstanceId());

    }
    return true;
}




bool teleport::mechanics()
{

    if (!this->getStats()->isWaiting())
    {
        if (this->getStats()->hasParent())
        {
            this->interact(this->getStats()->getStandingOn().lock());
          //  this->deregisterLiveElement(this->getStats()->getInstanceId());
        }
    };
    this->playSound("Teleport", "HummingSound");
    return bElem::mechanics();
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


