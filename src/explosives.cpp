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
    this->removeElement();
    step=brd->getElement(mc);
    step->destroy();
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
    this->disposeElement();
    return false;
}







