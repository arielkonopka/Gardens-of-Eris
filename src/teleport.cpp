#include "teleport.h"

videoElement::videoElementDef* teleport::vd=NULL;

teleport::teleport(chamber* board, gCollect* garbage):nonSteppable(board,garbage)
{
    //ctor
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

    for(cx=this->getCoords().x+1; cx<this->attachedBoard->width-1; cx++)
    {
        if(this->attachedBoard->getElement(cx,this->getCoords().y)->getType()==this->getType() && this->attachedBoard->getElement(cx,this->getCoords().y)->getSubtype()==this->getSubtype())
        {
            //We had found the teleporter
            if (this->teleportIt(who,(coords){cx,this->getCoords().y})==true)
            return true;

        }
    }
    for (cy=this->getCoords().y; cy<this->attachedBoard->height-1; cy++)
    {

        for(cx=0; cx<this->attachedBoard->width-1; cx++)
        {
            if(this->attachedBoard->getElement(cx,cy)->getType()==this->getType() && this->attachedBoard->getElement(cx,cy)->getSubtype()==this->getSubtype())
            {
                //We had found the teleporter
                if (this->teleportIt(who,(coords){cx,cy})==true)
                    return true;

            }
        }
    }
    for (cy=0; cy<this->getCoords().y-1; cy++)
    {

        for(cx=0; cx<this->attachedBoard->width-1; cx++)
        {
            if(this->attachedBoard->getElement(cx,cy)->getType()==this->getType() && this->attachedBoard->getElement(cx,cy)->getSubtype()==this->getSubtype())
            {
                //We had found the teleporter
                if (this->teleportIt(who,(coords){cx,cy})==true)
                    return true;

            }
        }
    }
    cy=this->getCoords().y;
    for(cx=0; cx<this->getCoords().x; cx++)
    {
        if(this->attachedBoard->getElement(cx,cy)->getType()==this->getType() && this->attachedBoard->getElement(cx,cy)->getSubtype()==this->getSubtype())
        {
                //We had found the teleporter
            if (this->teleportIt(who,(coords){cx,cy})==true)
               return true;

        }
    }
    return false;

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
bool teleport::teleportIt(bElem* who, coords teleportPosition)
{

    sNeighboorhood myNeigh;
    myNeigh=this->attachedBoard->getElement(teleportPosition.x,teleportPosition.y)->getSteppableNeighboorhood();
    for (int x=0; x<4; x++)
    {
        if(myNeigh.steppableClose[x]==true)
        {
            coords c=this->attachedBoard->getElement(teleportPosition.x,teleportPosition.y)->getAbsCoords((direction)x);
            if(c!=NOCOORDS)
            {
                who->stepOnElement(this->attachedBoard->getElement(c.x,c.y)); // We need to add the special effects and stuff, we will do it later
                // the best, would be remember last position of who and then create inactive player instance, and set it as teleport out, or dying, dying seems a little more attractive :)
                return true;
            }
            else
                continue; //this should not happen, because steppableClose is true, so the coordinates must be valid
        }
    }
    return false;
}
