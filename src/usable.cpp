#include "usable.h"


bool usable::use(std::shared_ptr<bElem> who)
{
    return true;
}


bool usable::use(std::shared_ptr<bElem> who, direction dir)
{
    this->playSound("Usable","Use");
    return true;
}

usable::usable(std::shared_ptr<chamber> board):usable()
{
    this->setBoard(board);
}
usable::usable():audibleElement()
{

}

bool usable::additionalProvisioning(int subtype, std::shared_ptr<usable>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool usable::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool usable::additionalProvisioning(int subtype,int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}





