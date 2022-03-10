#include "key.h"

videoElement::videoElementDef* key::vd=NULL;

videoElement::videoElementDef* key::getVideoElementDef()
{
    return key::vd;
}

int key::getType()
{
    return _key;
}

key::key(chamber* board, gCollect* garbage) : collectible::collectible(board,garbage)
{

}
key::key(chamber* board, gCollect* garbage, int subtype):collectible::collectible(board,garbage)
{
    this->setSubtype(subtype);
}

key::key(chamber* board, gCollect* garbage, int x, int y) : collectible::collectible(board,garbage,x,y)
{

}

key::~key()
{

}
