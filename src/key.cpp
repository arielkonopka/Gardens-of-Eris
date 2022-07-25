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

key::key(chamber* board) : collectible::collectible(board)
{
this->setDirection(UP);
}
key::key(chamber* board, int subtype):collectible::collectible(board)
{
    this->setSubtype(subtype);
    this->setDirection(UP);
   std::cout<<"key subtype: "<<this->getSubtype()<<"\n";
}

key::key(chamber* board, int x, int y) : collectible::collectible(board,x,y)
{

}

key::~key()
{

}
