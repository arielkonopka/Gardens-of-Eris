#include "collectible.h"
videoElement::videoElementDef* collectible::vd=NULL;

collectible::collectible(chamber *board):killableElements(board)
{
   // this->setSubtype(0);
}
collectible::collectible(chamber* board, int x, int y): killableElements(board,x,y)
{
   // this->setSubtype(0);
}

int collectible::getType()
{
    return _collectible;
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
bool collectible::isSteppable()
{
    return false;
}
