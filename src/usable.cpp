#include "usable.h"


usable::~usable()
{
    //dtor
}
bool usable::use(bElem* who)
{
    return true;
}

bool usable::isUsable()
{
    return true;
}

bool usable::use(bElem* who, direction dir)
{
    return true;
}

usable::usable(chamber* board):bElem(board)
{

}

usable::usable(chamber* board, int x, int y) : bElem(board,x,y)
{

}




