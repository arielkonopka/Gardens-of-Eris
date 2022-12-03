#include "explosives.h"


explosives::explosives(chamber *board):bElem(board)
{
    //ctor
}

explosives::explosives(chamber* board, int x, int y):bElem(board,x,y)
{

}

explosives::~explosives()
{
    //dtor
}

bool explosives::explode()
{
    bElem *el;
    //this->destroy();
    for (int cnt=0 ; cnt<4; cnt++)
    {
        el=this->getElementInDirection((direction)(cnt));
        if (el!=nullptr)
        {
            el->destroy();
            el=el->getElementInDirection((direction)((cnt+1)%4));
            if(el!=nullptr)
                el->destroy();
        }
    }
    return false;
}







