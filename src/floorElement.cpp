#include "floorElement.h"

floorElement::floorElement(): bElem()
{
    //ctor
}

floorElement::floorElement(std::shared_ptr<chamber> board) : bElem(board)
{

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
