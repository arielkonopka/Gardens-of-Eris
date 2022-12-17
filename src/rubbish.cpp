#include "rubbish.h"


videoElement::videoElementDef* rubbish::vd=nullptr;

rubbish::rubbish():bElem(),nonSteppable(),collectible()
{

}

rubbish::rubbish(std::shared_ptr<chamber> board) : bElem(board),nonSteppable(board),collectible(board)
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

