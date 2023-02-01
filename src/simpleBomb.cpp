#include "simpleBomb.h"

videoElement::videoElementDef *simpleBomb::vd = nullptr;

simpleBomb::simpleBomb(std::shared_ptr<chamber> board) : explosives(board), nonSteppable(board), killableElements(board), movableElements(board)
{
    this->setSubtype(0);
}

simpleBomb::simpleBomb() : explosives(), nonSteppable(), killableElements(), movableElements()
{
    this->setSubtype(0);
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
    bElem::destroy();
    if (this->triggered)
    {
        return false;
    }
    this->registerLiveElement(shared_from_this());
    this->triggered = true;
    this->setWait(5); /* magic number */
    return true;
}

bool simpleBomb::mechanics()
{
    bElem::mechanics();
    if (this->getWait() > 1)
        return false;
    this->explode();
    return true;
}

videoElement::videoElementDef *simpleBomb::getVideoElementDef()
{
    return simpleBomb::vd;
}

int simpleBomb::getType()
{
    return _simpleBombType;
}
