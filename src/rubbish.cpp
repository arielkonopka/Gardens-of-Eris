#include "rubbish.h"


videoElement::videoElementDef* rubbish::vd=nullptr;


rubbish::rubbish(chamber *board) : bElem(board),nonSteppable(board),collectible(board)
{
}

int rubbish::getType()
{
    return _rubishType;
}




videoElement::videoElementDef* rubbish::getVideoElementDef()
{
    return rubbish::vd;
}

