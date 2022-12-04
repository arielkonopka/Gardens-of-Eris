#include "killableElements.h"

killableElements::killableElements(chamber* board) :bElem(board)
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
