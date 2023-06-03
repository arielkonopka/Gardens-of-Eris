#include "killableElements.h"

killableElements::killableElements(std::shared_ptr<chamber> board) :bElem(board)
{
    this->setStats(std::make_shared<elemStats>(555));
}



killableElements::killableElements():bElem()
{
    this->setStats(std::make_shared<elemStats>(555));
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
