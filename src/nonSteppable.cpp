#include "nonSteppable.h"


videoElement::videoElementDef* nonSteppable::vd=NULL;


nonSteppable::nonSteppable(chamber *board):bElem(board)
{


}
nonSteppable::nonSteppable(chamber* board,int x, int y) : bElem(board,x,y)
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
bool nonSteppable::mechanics()
{
    bool res= bElem::mechanics();

    return res;

}


