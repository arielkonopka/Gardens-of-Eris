#include "teleport.h"

videoElement::videoElementDef *teleport::vd = nullptr;
std::vector<teleport *> teleport::allTeleporters;

teleport::teleport(std::shared_ptr<chamber> board) : teleport()
{
    this->setBoard(board);
}
teleport::teleport(std::shared_ptr<chamber> board, int newSubtype) : teleport(board)
{

    this->setSubtype(newSubtype);
}

teleport::teleport() : nonSteppable(), audibleElement()
{
    this->connectionsMade = false;
    teleport::allTeleporters.push_back(this);
    this->theOtherEnd = nullptr; // we do not
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
        teleport::allTeleporters[c]->checked = false;
    }
    return this->theOtherEnd->teleportIt(who);
}

bool teleport::createConnectionsWithinSUbtype()
{
    std::vector<teleport *> candidates;
    for (unsigned int c = 0; c < teleport::allTeleporters.size(); c++)
    {
        if (teleport::allTeleporters[c]->getSubtype() == this->getSubtype())
        {
            teleport::allTeleporters[c]->connectionsMade = true;
            candidates.push_back(teleport::allTeleporters[c]);
        }
    }
    for (unsigned int c = 0; c < candidates.size() - 1; c++)
    {
        candidates[c]->theOtherEnd = candidates[c + 1];
    }
    candidates[candidates.size() - 1]->theOtherEnd = candidates[0];
    return true;
}

videoElement::videoElementDef *teleport::getVideoElementDef()
{
    return teleport::vd;
}

int teleport::getType()
{
    return _teleporter;
}

// Teleport to this becon
bool teleport::teleportIt(std::shared_ptr<bElem> who)
{
    int dir = (int)who->getDirection();
    if (this->checked)
        return false;
    this->checked = true;
    who->setTeleporting(_teleportationTime);
    if (who->getSteppingOnElement() != nullptr)
        who->getSteppingOnElement()->setTeleporting(_teleportationTime);
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

bool teleport::isSteppable()
{
    if (this->getSubtype() > 0)
    {
        if (this->isTeleporting())
            return false;
        if (this->theOtherEnd != nullptr)
        {
            if (this->theOtherEnd->getStomper() == nullptr)
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

void teleport::stomp(std::shared_ptr<bElem> who)
{
    bElem::stomp(who);
    this->setWait(_teleportStandTime);
    this->registerLiveElement(shared_from_this());
}

void teleport::unstomp()
{
    bElem::unstomp();
    if (this->isLiveElement())
        this->deregisterLiveElement(this->getInstanceid());
}

bool teleport::mechanics()
{

    if (!this->isWaiting())
    {
        if (this->getStomper() != nullptr)
        {
            this->interact(this->getStomper());
            this->deregisterLiveElement(this->getInstanceid());
        }
    };
    this->playSound("Teleport", "HummingSound");
    return nonSteppable::mechanics();
}

bool teleport::removeFromAllTeleporters()
{
    for (unsigned int c = 0; c < teleport::allTeleporters.size(); c++)
    {
        if (teleport::allTeleporters[c]->getSubtype() == this->getSubtype())
        {
            teleport::allTeleporters[c]->connectionsMade = false;
        }
    }
    for (unsigned int c = 0; c < teleport::allTeleporters.size();)
    {
        if (teleport::allTeleporters[c]->getInstanceid() == this->getInstanceid())
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

bool teleport::canBeKilled()
{
    return false;
}

bool teleport::canBeDestroyed()
{
    return false;
}
