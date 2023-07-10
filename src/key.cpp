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

}

key::key() : collectible(), nonSteppable()
{
}
bool key::additionalProvisioning(int subtype, std::shared_ptr<key>key)
{
    return this->additionalProvisioning(subtype,key->getType());
}
bool key::additionalProvisioning(int subtype, int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}

bool key::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}
