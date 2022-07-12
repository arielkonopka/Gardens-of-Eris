#include "teleport.h"

videoElement::videoElementDef* teleport::vd=NULL;
std::vector<teleport*> teleport::teleporters;

teleport::teleport(chamber* board):nonSteppable(board)
{
    this->addToTeleports();
    this->theOtherEnd=NULL; // we do not have the other end configured yet. We will configure it on interact method;
}

teleport::~teleport()
{
    this->purgeFromTeleporters();
}
/* here we will try to teleport an object to the becon connected to this teleporter. if the becon is not yet established, randomly choose one */
bool teleport::interact(bElem* who)
{

    if(who==NULL)
        return false;
    if(who->canInteract()==false)
        return false;
    if(this->theOtherEnd==NULL)
    {
        int b=(this->randomNumberGenerator()%teleport::teleporters.size());
        while(this==teleport::teleporters[b])
        {
            ///////////////////////// New Chamber ///////////////////////////// -> everytime we hit the same object, we create a new chamber, that should have few teleporters there.
            //Here we should create a new chamber
            b=(this->randomNumberGenerator()%teleport::teleporters.size());
        }
        this->theOtherEnd=teleport::teleporters[b];
        this->theOtherEnd->removeFromTeleports(); //We remove the other end from teleporters available to choose, as it was already chosen.
        //this way, we can always have an allocated teleporter

    }
    return this->theOtherEnd->teleportIt(who);

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
    for(int c=0; c<4; c++)
    {
        direction d=(direction)((dir+c)%4);
        if (this->isSteppableDirection(d))
        {
            who->removeElement();
            who->setBoard(this->getBoard());
            who->stepOnElement(this->getElementInDirection(d));
            who->setTeleporting(_mov_delay+1*40);
            return true;
        }
    }
    return false;
}

void teleport::purgeFromTeleporters()
{
    std::vector<teleport*>::iterator ptr;
    for(ptr=teleport::teleporters.begin(); ptr!=teleport::teleporters.end(); )
    {
        if((*ptr)->getInstanceid()==this->getInstanceid())
        {
            teleport::teleporters.erase(ptr); //
        }
        else
        {
            if ((*ptr)->theOtherEnd!=NULL)
            {
                if((*ptr)->theOtherEnd->getInstanceid()==this->getInstanceid())
                {
                    (*ptr)->theOtherEnd->addToTeleports(); // not sure if we should do the teleport recycling
                    (*ptr)->theOtherEnd=NULL; //We remove the reference to the disposed teleport
                }
            }
            ptr++;
        }
    }

}


bool teleport::removeFromTeleports()
{
    std::vector<teleport*>::iterator ptr;
    for(ptr=teleport::teleporters.begin(); ptr!=teleport::teleporters.end(); )
    {
        if((*ptr)->getInstanceid()==this->getInstanceid())
        {
            teleport::teleporters.erase(ptr); //
        }
        else
        {
            ptr++;
        }
    }
    return true;
}

bool teleport::addToTeleports()
{
    teleport::teleporters.push_back(this);  //we add this object to the teleport list
    return true;
}



oState teleport::disposeElement()
{
    this->purgeFromTeleporters();
    return nonSteppable::disposeElement();
}

oState teleport::disposeElementUnsafe()
{
    this->purgeFromTeleporters();
    return nonSteppable::disposeElementUnsafe();

}

bool teleport::canBeKilled()
{
    return false;
}

bool teleport::canBeDestroyed()
{
    return true;
}
