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

teleport::teleport() : nonSteppable(), audibleElement()
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
        if (!teleport::allTeleporters[c].expired() && t->attrs->getSubtype() == this->attrs->getSubtype())
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
    int dir = (int)who->status->getMyDirection();
    if (this->checked)
        return false;
    this->checked = true;
    who->status->setTelInProgress(_teleportationTime);
    if (who->status->getSteppingOn() != nullptr)
        who->status->getSteppingOn()->status->setTelInProgress(_teleportationTime);
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
    if (this->attrs->getSubtype() > 0)
    {
        if (this->status->isTeleporting())
            return false;
        if (this->theOtherEnd != nullptr)
        {
            if (this->theOtherEnd->status->getStandingOn() == nullptr)
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
    this->status->setWaiting(_teleportStandTime);
    this->registerLiveElement(shared_from_this());
}

void teleport::unstomp()
{
    bElem::unstomp();
    if (this->isLiveElement())
        this->deregisterLiveElement(this->status->getInstanceId());
}
*/
bool teleport::mechanics()
{

    if (!this->status->isWaiting())
    {
        if (this->status->hasParent())
        {
            this->interact(this->status->getStandingOn().lock());
            this->deregisterLiveElement(this->status->getInstanceId());
        }
    };
    this->playSound("Teleport", "HummingSound");
    return nonSteppable::mechanics();
}

bool teleport::removeFromAllTeleporters()
{
    for (unsigned int c = 0; c < teleport::allTeleporters.size(); c++)
    {
        std::shared_ptr<teleport> t=teleport::allTeleporters[c].lock();
        if (!teleport::allTeleporters[c].expired() && t->attrs->getSubtype() == this->attrs->getSubtype())
        {
            t->connectionsMade = false;
        }
    }
    for (unsigned int c = 0; c < teleport::allTeleporters.size();)
    {
    std::shared_ptr<teleport> t=teleport::allTeleporters[c].lock();
        if (teleport::allTeleporters[c].expired() || t->status->getInstanceId() == this->status->getInstanceId())
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
    return nonSteppable::disposeElement();
}

oState teleport::disposeElementUnsafe()
{
    this->removeFromAllTeleporters();
    return nonSteppable::disposeElementUnsafe();
}


