#include "nonSteppable.h"


videoElement::videoElementDef* nonSteppable::vd=NULL;


nonSteppable::nonSteppable(chamber *board):bElem(board)
{
    this->teleporting=0;

}
nonSteppable::nonSteppable(chamber* board,int x, int y) : bElem(board,x,y)
{
    this->teleporting=0;
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
    bool res= bElem::mechanics(collected);
    if(this->teleporting>0)
    {
        this->teleporting--;
        return false;
    }
    return res;

}
void nonSteppable::setTeleporting(int time)
{
    this->teleporting=time;
}
bool nonSteppable::isTeleporting()
{
    return (this->teleporting>0);
}

