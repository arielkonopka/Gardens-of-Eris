#include "collectible.h"
videoElement::videoElementDef* collectible::vd=nullptr;

collectible::collectible(std::shared_ptr<chamber> board):collectible()
{
    this->setBoard(board);
}
collectible::collectible():audibleElement()
{

}

videoElement::videoElementDef* collectible::getVideoElementDef()
{
    return collectible::vd;
}
bool collectible::additionalProvisioning(int subtype, std::shared_ptr<collectible> sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}


bool collectible::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool collectible::additionalProvisioning(int subtype, int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}
