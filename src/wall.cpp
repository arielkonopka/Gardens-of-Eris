#include "wall.h"


videoElement::videoElementDef* wall::vd=nullptr;


wall::wall(chamber* board) : nonSteppable(board)
{

            this->setSubtype(0);
}

wall::wall(chamber* board,int subtype) : nonSteppable(board)
{

            this->setSubtype(subtype);

}



int wall::getType()
{
    return _wallType;
}


//Return video element definition object, this will come in handy when drawing the object
videoElement::videoElementDef* wall::getVideoElementDef()
{
    return wall::vd;
}


bool wall::isDying()
{
    return false;
}
bool wall::canBeKilled()
{
   return false;
}

bool wall::canBeDestroyed()
{
    return false;
}

