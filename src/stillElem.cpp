#include "stillElem.h"

videoElement::videoElementDef* stillElem::vd=NULL;

stillElem::~stillElem()
{

}

int stillElem::getType()
{
    return _stillElemType;
}

int stillElem::getSubtype()
{
    return 0;
}

stillElem::stillElem(chamber* board) :nonSteppable(board)
{
    return;
}

videoElement::videoElementDef* stillElem::getVideoElementDef()
{
    return stillElem::vd;
}
