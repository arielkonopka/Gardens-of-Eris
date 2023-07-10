#include "wall.h"

videoElement::videoElementDef *wall::vd = nullptr;

wall::wall(std::shared_ptr<chamber> board) : wall()
{
    this->setBoard(board);
}

wall::wall(std::shared_ptr<chamber> board, int subtype) : wall(board)
{

  //  this->attrs->setSubtype(subtype);
}

wall::wall() : nonSteppable()
{
}


bool wall::additionalProvisioning(int subtype, std::shared_ptr<wall>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool wall::additionalProvisioning(int subtype,int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}




bool wall::stepOnElement(std::shared_ptr<bElem> elem)
{
    bool res = nonSteppable::stepOnElement(elem);
    if (this->getBoard().get() != nullptr)
        this->getBoard()->setVisible(this->status->getMyPosition(), 254);
    return res;
}

std::shared_ptr<bElem> wall::removeElement()
{
    if (this->getBoard().get() != nullptr)
        this->getBoard()->setVisible(this->status->getMyPosition(), 255);
    std::shared_ptr<bElem> res = nonSteppable::removeElement();
    return res;
}

bool wall::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

int wall::getType()
{
    return _wallType;
}

// Return video element definition object, this will come in handy when drawing the object
videoElement::videoElementDef *wall::getVideoElementDef()
{
    return wall::vd;
}
/*
bool wall::isDying()
{
    return false;
}
bool wall::canBeKilled()
{
    return false;
}

bool wall::canBeDestroyed()
{
    return false;
}
*/
