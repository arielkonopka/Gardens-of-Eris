#include "mechanical.h"

mechanical::mechanical(std::shared_ptr<chamber> board) : mechanical()
{
    this->setBoard(board);
}

mechanical::~mechanical()
{

    this->deregisterLiveElement(this->getInstanceid());
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
    if (bElem::additionalProvisioning() == true)
        return true;
    if (this->registerEl)
        this->registerLiveElement(shared_from_this());
    return false;
}
