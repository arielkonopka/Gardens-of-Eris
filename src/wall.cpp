#include "wall.h"


videoElement::videoElementDef* wall::vd=NULL;


wall::wall(chamber* board) : nonSteppable(board)
{
        this->subtype=0;
}

wall::wall(chamber* board,int subtype) : nonSteppable(board)
{
        this->subtype=subtype;
}


int wall::getSubtype()
{
    return this->subtype;
}

int wall::getType()
{
    return _wallType;
}


wall::~wall()
{
    //dtor
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

