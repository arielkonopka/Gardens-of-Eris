#include "floorElement.h"

floorElement::floorElement()
{
    //ctor
}

floorElement::floorElement(chamber* board) : bElem(board)
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
