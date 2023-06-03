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
