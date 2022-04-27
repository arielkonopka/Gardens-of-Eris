#include "teleport.h"

videoElement::videoElementDef* teleport::vd=NULL;
std::vector<teleport*> teleport::teleporters;

teleport::teleport(chamber* board, gCollect* garbage):nonSteppable(board,garbage)
{
    this->addToTeleports();
    this->theOtherEnd=NULL; // we do not have the other end configured yet. We will configure it on interact method;
}

teleport::~teleport()
{
    //dtor
}
bool teleport::interact(bElem* who)
{
    int cx,cy;
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

//The position must exist!
bool teleport::teleportIt(bElem* who)
{
    sNeighboorhood myNeigh;
    myNeigh=this->getSteppableNeighboorhood();
    for (int x=0; x<4; x++)
    {
        if(myNeigh.steppableClose[x]==true)
        {
            coords c=this->getAbsCoords((direction)x);
            if(c!=NOCOORDS)
            {
                who->removeElement(); //we found a space to be teleported
                who->setBoard(this->attachedBoard); // We can be teleported between the chambers
                who->stepOnElement(this->attachedBoard->getElement(c.x,c.y)); // We need to add the special effects and stuff, we will do it later
                // the best, would be remember last position of who and then create inactive player instance, and set it as teleport out, or dying, dying seems a little more attractive :)
                who->setMoved(_mov_delay*4);
                return true;
            }
        }
    }
    return false;
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
    this->removeFromTeleports();
    return nonSteppable::disposeElement();
}

oState teleport::disposeElementUnsafe()
{
    this->removeFromTeleports();
    return nonSteppable::disposeElementUnsafe();

}

