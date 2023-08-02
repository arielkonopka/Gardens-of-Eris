#include "mechanical.h"

mechanical::mechanical(std::shared_ptr<chamber> board) : mechanical()
{
    this->setBoard(board);
}

mechanical::~mechanical()
{

    this->deregisterLiveElement(this->status->getInstanceId());
}
mechanical::mechanical(std::shared_ptr<chamber> board, bool rEl) : mechanical(board)
{

    this->registerEl = rEl;
}

mechanical::mechanical() : bElem()
{
}
bool mechanical::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}
bool mechanical::additionalProvisioning(int subtype, int typeId)
{
    bElem::additionalProvisioning(subtype,typeId);
    if (this->registerEl)
        this->registerLiveElement(shared_from_this());
    return true;
}
bool mechanical::additionalProvisioning(int subtype, std::shared_ptr<mechanical>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}
