#include "wall.h"


videoElement::videoElementDef* wall::vd=nullptr;


wall::wall(std::shared_ptr<chamber> board) : nonSteppable(board)
{

    this->setSubtype(0);
}

wall::wall(std::shared_ptr<chamber> board,int subtype) : nonSteppable(board)
{

    this->setSubtype(subtype);

}

wall::wall():nonSteppable()
{

}

bool wall::stepOnElement(std::shared_ptr<bElem>elem)
{
    bool res=nonSteppable::stepOnElement(elem);
    if(this->getBoard().get()!=nullptr)
        this->getBoard()->setVisible(this->getCoords(),254);
    return res;
}

std::shared_ptr<bElem> wall::removeElement()
{
    if(this->getBoard().get()!=nullptr)
        this->getBoard()->setVisible(this->getCoords(),255);
    std::shared_ptr<bElem> res=nonSteppable::removeElement();
    return res;

}


bool wall::additionalProvisioning()
{
    return true;
}

int wall::getType()
{
    return _wallType;
}


//Return video element definition object, this will come in handy when drawing the object
videoElement::videoElementDef* wall::getVideoElementDef()
{
    return wall::vd;
}


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

