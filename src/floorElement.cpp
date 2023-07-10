#include "floorElement.h"

floorElement::floorElement(): bElem()
{
    //ctor
}

floorElement::floorElement(std::shared_ptr<chamber> board) : floorElement()
{
    this->setBoard(board);
}



floorElement::~floorElement()
{
    //dtor
}
videoElement::videoElementDef* floorElement::vd=nullptr;

videoElement::videoElementDef* floorElement::getVideoElementDef()
{
    return floorElement::vd;
}
int floorElement::getType() const
{
    return _floorType;
}
bool floorElement::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool floorElement::additionalProvisioning(int subtype, int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}
bool floorElement::additionalProvisioning(int subtype, std::shared_ptr<floorElement>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}


