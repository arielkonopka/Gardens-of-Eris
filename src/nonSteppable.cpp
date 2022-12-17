#include "nonSteppable.h"


videoElement::videoElementDef* nonSteppable::vd=nullptr;


nonSteppable::nonSteppable(std::shared_ptr<chamber> board):bElem(board)
{


}
nonSteppable::nonSteppable():bElem()
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


