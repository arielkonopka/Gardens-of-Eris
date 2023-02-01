#include "explosives.h"

explosives::explosives(std::shared_ptr<chamber> board) : audibleElement(board)
{
    // ctor
}

explosives::explosives() : audibleElement()
{
}

explosives::~explosives()
{
    // dtor
}

bool explosives::explode()
{
    std::shared_ptr<bElem> el;
    coords mc=(this->getCollector().get()!=nullptr)?this->getCollector()->getCoords():this->getCoords();
    std::shared_ptr<chamber> brd = (this->getCollector().get()!=nullptr)?this->getCollector()->getBoard():this->getBoard();
    std::shared_ptr<bElem> step;
    this->playSound("Explosives","Explode"); // This sound must not be looped, otherwise it will be swiped off
    this->removeElement();
    step = brd->getElement(mc);
    step->destroy();
    for (int cnt = 0; cnt < 4; cnt++)
    {
        el = step->getElementInDirection((direction)(cnt));
        if (el != nullptr)
        {
            el->destroy();
            el = el->getElementInDirection((direction)((cnt + 1) % 4));
            if (el != nullptr)
                el->destroy();
        }
    }

    this->disposeElement();
    return false;
}
