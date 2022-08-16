#include "collectible.h"
videoElement::videoElementDef* collectible::vd=NULL;

collectible::collectible(chamber *board):killableElements(board,false)
{
   // this->setSubtype(0);
}
collectible::collectible(chamber* board, int x, int y): killableElements(board,x,y,false)
{
   // this->setSubtype(0);
}

collectible::collectible(chamber* board, bool registerEl):killableElements(board,registerEl)
{

}

collectible::collectible(chamber* board, int x, int y, bool registerEl): killableElements(board,x,y,registerEl)
{

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
