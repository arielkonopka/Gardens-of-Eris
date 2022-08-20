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
bool usable::isCollectible()
{
    return true;
}
usable::usable(chamber* board):collectible(board)
{

}

usable::usable(chamber* board, int x, int y) : collectible(board)
{

}


usable::usable(chamber* board, int x, int y, bool registerEl):collectible(board,x,y,registerEl)
{

}

usable::usable(chamber* board, bool registerEl) :collectible(board,registerEl)
{

}


