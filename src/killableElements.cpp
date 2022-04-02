#include "killableElements.h"

killableElements::killableElements(chamber* board,gCollect* garbageBin) :movableElements(board,garbageBin)
{
    //ctor
}

killableElements::~killableElements()
{
    //dtor
}
bool killableElements::canBeKilled()
{
    return (!this->isDying());
}
