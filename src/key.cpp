#include "key.h"

videoElement::videoElementDef* key::vd=nullptr;

videoElement::videoElementDef* key::getVideoElementDef()
{
    return key::vd;
}

int key::getType()
{
    return _key;
}

key::key(std::shared_ptr<chamber> board) : collectible(board), nonSteppable(board)
{
    this->setDirection(UP);
}
key::key(std::shared_ptr<chamber> board, int subtype): collectible(board), nonSteppable(board)
{
    this->setSubtype(subtype);
    this->setDirection(UP);
}

key::key():collectible(),nonSteppable()
{

}
