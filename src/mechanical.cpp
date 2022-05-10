#include "mechanical.h"

mechanical::mechanical(chamber* board):nonSteppable(board)
{
    this->registerLiveElement(this);
}

mechanical::mechanical(chamber* board, int x, int y):nonSteppable(board,x,y)
{
     this->registerLiveElement(this);
}

mechanical::~mechanical()
{
    this->deregisterLiveElement(this);

}
