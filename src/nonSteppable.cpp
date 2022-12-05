#include "nonSteppable.h"


videoElement::videoElementDef* nonSteppable::vd=nullptr;


nonSteppable::nonSteppable(chamber *board):bElem(board)
{


}




bool nonSteppable::isSteppable()
{
    return false;
}



videoElement::videoElementDef* nonSteppable::getVideoElementDef()
{
    return nonSteppable::vd;
}


