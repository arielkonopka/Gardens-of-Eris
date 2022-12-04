#include "mechanical.h"

mechanical::mechanical(chamber* board):bElem(board)
{
    this->registerLiveElement(this);
}



mechanical::~mechanical()
{
    this->deregisterLiveElement(this);

}
mechanical::mechanical(chamber* board, bool registerEl): bElem(board)
{
    if(registerEl) this->registerLiveElement(this);
}


