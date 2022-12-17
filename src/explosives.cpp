#include "explosives.h"


explosives::explosives(std::shared_ptr<chamber> board):bElem(board)
{
    //ctor
}

explosives::explosives():bElem()
{

}

explosives::~explosives()
{
    //dtor
}

bool explosives::explode()
{
    std::shared_ptr<bElem> el;
    coords mc=this->getCoords();
    std::shared_ptr<chamber>  brd=this->getBoard();
    std::shared_ptr<bElem> step;
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







