#include "explosives.h"


explosives::explosives(chamber *board):bElem(board)
{
    //ctor
}


explosives::~explosives()
{
    //dtor
}

bool explosives::explode()
{
    bElem *el;
    coords mc=this->getCoords();
    chamber* brd=this->getBoard();
    bElem* step;
    this->disposeElement();
    step=brd->getElement(mc);
    for (int cnt=0 ; cnt<4; cnt++)
    {
        el=step->getElementInDirection((direction)(cnt));
        if (el!=nullptr)
        {
            el->destroy();
            el=el->getElementInDirection((direction)((cnt+1)%4));
            if(el!=nullptr)
                el->destroy();
        }
    }
    step->destroy();

    return false;
}







