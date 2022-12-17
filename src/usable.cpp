#include "usable.h"


bool usable::use(std::shared_ptr<bElem> who)
{
    return true;
}

bool usable::isUsable()
{
    return true;
}

bool usable::use(std::shared_ptr<bElem> who, direction dir)
{
    return true;
}

usable::usable(std::shared_ptr<chamber> board):bElem(board)
{

}
usable::usable():bElem()
{

}






