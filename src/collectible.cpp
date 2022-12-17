#include "collectible.h"
videoElement::videoElementDef* collectible::vd=nullptr;

collectible::collectible(std::shared_ptr<chamber> board):bElem(board)
{
}
collectible::collectible():bElem()
{

}


bool collectible::isCollectible()
{
    return true;
}
videoElement::videoElementDef* collectible::getVideoElementDef()
{
    return collectible::vd;
}

