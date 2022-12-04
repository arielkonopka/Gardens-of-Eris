#include "usable.h"


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






