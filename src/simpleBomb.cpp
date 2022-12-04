#include "simpleBomb.h"

videoElement::videoElementDef* simpleBomb::vd=nullptr;


simpleBomb::simpleBomb(chamber* board):explosives(board),nonSteppable(board),killableElements(board), movableElements(board)
{

}

bool simpleBomb::hurt(int points)
{
    return this->destroy();
}

bool simpleBomb::kill()
{
    return this->destroy();
}

bool simpleBomb::destroy()
{
    bool res=bElem::destroy();
    if(!res || this->triggered)
    {
        return false;
    }
    this->triggered=true;
    this->setWait(20);
    return res;
}

bool simpleBomb::mechanics()
{
    bool r=bElem::mechanics();
    if(this->getWait()==2)
    {
        this->explode();

    }
    return r;
}


videoElement::videoElementDef* simpleBomb::getVideoElementDef()
{
    return simpleBomb::vd;
}

int simpleBomb::getType()
{
    return _explosivesType;
}


