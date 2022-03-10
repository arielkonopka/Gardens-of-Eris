#include "wall.h"


videoElement::videoElementDef* wall::vd=NULL;


wall::wall(chamber* board,gCollect* garbage) : nonSteppable(board,garbage)
{
        this->subtype=0;
}

wall::wall(chamber* board,gCollect* garbage,int subtype) : nonSteppable(board,garbage)
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

