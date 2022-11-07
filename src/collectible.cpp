#include "collectible.h"
videoElement::videoElementDef* collectible::vd=NULL;

collectible::collectible(chamber *board):bElem(board)
{
   // this->setSubtype(0);
}
collectible::collectible(chamber* board, int x, int y): bElem(board,x,y)
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

