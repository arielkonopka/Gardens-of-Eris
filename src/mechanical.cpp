#include "mechanical.h"

mechanical::mechanical(chamber* board):movableElements(board)
{
    this->registerLiveElement(this);
}

mechanical::mechanical(chamber* board, int x, int y):movableElements(board,x,y)
{
     this->registerLiveElement(this);
}

mechanical::~mechanical()
{
    this->deregisterLiveElement(this);

}
mechanical::mechanical(chamber* board, bool registerEl):movableElements(board)
{
    if(registerEl) this->registerLiveElement(this);
}

mechanical::mechanical(chamber* board, int x, int y, bool registerEl):movableElements(board,x,y)
{
    if(registerEl) this->registerLiveElement(this);
}

bool mechanical::kill()
{
    if(isDying())
        return true;
    //return true;
    if(!isLiveElement())
        this->registerLiveElement(this);
    //movableElements::kill();
    return true;
}
