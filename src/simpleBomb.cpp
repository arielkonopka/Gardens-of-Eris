#include "simpleBomb.h"

videoElement::videoElementDef *simpleBomb::vd = nullptr;

simpleBomb::simpleBomb(std::shared_ptr<chamber> board) : explosives(board), nonSteppable(board), killableElements(board), movableElements(board)
{
}

simpleBomb::simpleBomb() : explosives(), nonSteppable(), killableElements(), movableElements()
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
    if (this->triggered)
    {
        bElem::destroy();
        return false;
    }

    this->registerLiveElement(shared_from_this());
    this->triggered = true;
    this->setWait(20);
    return true;
}

bool simpleBomb::mechanics()
{
    bElem::mechanics();
    if (this->getWait() != 1)
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
    return _explosivesType;
}
