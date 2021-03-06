#include "killableElements.h"

killableElements::killableElements(chamber* board) :movableElements(board)
{
    this->movable=false;
}

killableElements::killableElements(chamber* board, int x, int y):movableElements(board,x,y)
{
    this->movable=false;
}

bool killableElements::mechanics(bool collected)
{
    bool res=movableElements::mechanics(collected);
    if(this->killed>0)
    {
        this->killed--;
        if(this->killed==0)
        {
            this->disposeElement(); //it seems we really died. what a waste
            return true;
        }
    }
    return res;
}



killableElements::~killableElements()
{
    //dtor
}
bool killableElements::canBeKilled()
{
    return (!this->isDying());
}

bool killableElements::canBeDestroyed()
{
    return (!this->isDying());
}




bool killableElements::hurt(int points)
{
   if (this->canBeKilled()==false || this->isTeleporting() || this->isDying() || this->isDestroyed())
    {
        return false;
    }

    this->setMoved(_mov_delay);
    this->setEnergy(this->getEnergy()-points);
    if (this->getEnergy()<=0)
        this->kill();
    return true;
}

bool killableElements::kill()
{
    if (this->canBeKilled()==false)
    {
        return false;
    }
    this->killed=_defaultKillTime;
    return true;
}

