#include "key.h"

videoElement::videoElementDef *key::vd = nullptr;

videoElement::videoElementDef *key::getVideoElementDef()
{
    return key::vd;
}

int key::getType()
{
    return _key;
}

key::key(std::shared_ptr<chamber> board) : key()
{
    this->setBoard(board);
}
key::key(std::shared_ptr<chamber> board, int subtype) : key(board)
{

    this->setSubtype(subtype);
}

key::key() : collectible(), nonSteppable()
{
    this->setDirection(UP);
}
