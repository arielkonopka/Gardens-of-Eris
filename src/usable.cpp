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
usable::usable(chamber* board, gCollect* garbage):collectible(board,garbage)
{

}

usable::usable(chamber* board, gCollect* garbage, int x, int y) : collectible(board,garbage)
{

}
