#include "nonSteppable.h"


videoElement::videoElementDef* nonSteppable::vd=NULL;


nonSteppable::nonSteppable(chamber *board,gCollect *garbage): bElem(board,garbage)
{

}

nonSteppable::nonSteppable(chamber* board, gCollect* garbage, int x, int y) : bElem(board,garbage,x,y)
{

}

nonSteppable::~nonSteppable()
{
    //dtor
}
bool nonSteppable::isSteppable()
{
    return false;
}
int nonSteppable::getType()
{
    return _nonSteppableType;
}


videoElement::videoElementDef* nonSteppable::getVideoElementDef()
{
    return nonSteppable::vd;
}
bool nonSteppable::mechanics(bool collected)
{
    return bElem::mechanics(collected);


}

