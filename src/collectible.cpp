#include "collectible.h"
videoElement::videoElementDef* collectible::vd=nullptr;

collectible::collectible(chamber *board):bElem(board)
{
   // this->setSubtype(0);
}


collectible::~collectible()
{
    //dtor
}
bool collectible::isCollectible()
{
    return true;
}
videoElement::videoElementDef* collectible::getVideoElementDef()
{
    return collectible::vd;
}

