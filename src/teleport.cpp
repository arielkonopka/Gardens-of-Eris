#include "teleport.h"

videoElement::videoElementDef* teleport::vd=NULL;
std::vector<teleport*> teleport::allTeleporters;

teleport::teleport(chamber* board):nonSteppable(board)
{
    this->connectionsMade=false;
    teleport::allTeleporters.push_back(this);
    this->theOtherEnd=NULL; // we do not have the other end configured yet. We will configure it on interact method;
}
teleport::teleport(chamber* board,int newSubtype):nonSteppable(board)
{
    this->connectionsMade=false;
    teleport::allTeleporters.push_back(this);
    this->theOtherEnd=NULL; // we do not have the other end configured yet. We will configure it on interact method;
    this->setSubtype(newSubtype);
}

teleport::~teleport()
{

    this->removeFromAllTeleporters();


}
/* here we will try to teleport an object to the becon connected to this teleporter. if the becon is not yet established, randomly choose one */
bool teleport::interact(bElem* who)
{
    if (bElem::interact(who)==false)
        return false;
    if(this->connectionsMade==false)
        this->createConnectionsWithinSUbtype();
    return this->theOtherEnd->teleportIt(who);

}


bool teleport::createConnectionsWithinSUbtype()
{
    std::vector<teleport*> candidates;
    for(unsigned int c=0; c<teleport::allTeleporters.size(); c++)
    {
        if(teleport::allTeleporters[c]->getSubtype()==this->getSubtype())
        {
            teleport::allTeleporters[c]->connectionsMade=true;
            candidates.push_back(teleport::allTeleporters[c]);
        }
    }
    for(unsigned int c=0; c<candidates.size()-1; c++)
    {
        candidates[c]->theOtherEnd=candidates[c+1];
    }
    candidates[candidates.size()-1]->theOtherEnd=candidates[0];
    return true;
}




bool teleport::isInteractive()
{
    return true;
}
videoElement::videoElementDef* teleport::getVideoElementDef()
{
    return teleport::vd;
}

int teleport::getType()
{
    return _teleporter;
}

//Teleport to this becon
bool teleport::teleportIt(bElem* who)
{
    int dir=(int)who->getDirection();
    who->setTeleporting(_teleportationTime);
    if(who->getSteppingOnElement()!=NULL)
        who->getSteppingOnElement()->setTeleporting(_teleportationTime);
    for(int c=0; c<4; c++)
    {
        direction d=(direction)((dir+c)%4);
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
    if(this->getSubtype()>0)
    {
        if(this->isTeleporting())
            return false;
        if(this->theOtherEnd!=NULL)
        {
            if(this->theOtherEnd->getStomper()==NULL)
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

void teleport::stomp(bElem* who)
{
    nonSteppable::stomp(who);
    this->setWait(_teleportStandTime);
    this->registerLiveElement(this);


}

void teleport::unstomp()
{
    nonSteppable::unstomp();
    if(this->isLiveElement())
        this->deregisterLiveElement(this);
}

bool teleport::mechanics()
{
    if(!this->isWaiting())
    {
        if(this->getStomper()!=NULL)
        {
            this->interact(this->getStomper());
            this->deregisterLiveElement(this);
        }
    };
    return nonSteppable::mechanics();
}




bool teleport::removeFromAllTeleporters()
{
    for(unsigned int c=0; c<teleport::allTeleporters.size(); c++)
    {
        if(teleport::allTeleporters[c]->getSubtype()==this->getSubtype())
        {
            teleport::allTeleporters[c]->connectionsMade=false;
        }
    }
    for(unsigned int c=0; c<teleport::allTeleporters.size();)
    {
        if(teleport::allTeleporters[c]->getInstanceid()==this->getInstanceid())
        {
            teleport::allTeleporters.erase(teleport::allTeleporters.begin()+c);
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




