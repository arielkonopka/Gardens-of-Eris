#include "killableElements.h"

killableElements::killableElements(chamber* board) :bElem(board)
{

}

killableElements::killableElements(chamber* board, int x, int y):bElem(board,x,y)
{
}

killableElements::killableElements(chamber* board, bool registerEl):bElem(board)
{
}

killableElements::killableElements(chamber* board, int x, int y, bool registerEl): bElem(board,x,y)
{
}






killableElements::~killableElements()
{
    //dtor
}




bool killableElements::hurt(int points)
{
    if (this->canBeKilled()==false || this->isTeleporting() || this->isDying() || this->isDestroyed())
    {
        return false;
    }

    this->setEnergy(this->getEnergy()-points);
    if (this->getEnergy()<=0)
        this->kill();
    return true;
}
