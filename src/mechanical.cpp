#include "mechanical.h"

mechanical::mechanical(chamber* board, gCollect* garbage):nonSteppable(board,garbage)
{
    this->registerLiveElement(this);
}

mechanical::mechanical(chamber* board, gCollect* garbage, int x, int y):nonSteppable(board,garbage,x,y)
{
     this->registerLiveElement(this);
}

mechanical::~mechanical()
{
    this->deregisterLiveElement(this);

}
