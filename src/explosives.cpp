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

bool explosives::explode(float radius)
{

    std::shared_ptr<bElem> step=(this->getCollector().get()!=nullptr)?this->getCollector()->getSteppingOnElement():this->getSteppingOnElement();
    coords mc=(this->getCollector().get()!=nullptr)?this->getCollector()->getCoords():this->getCoords();
    std::shared_ptr<chamber> brd = (this->getCollector().get()!=nullptr)?this->getCollector()->getBoard():this->getBoard();
    int xs=(mc.x-radius<0)?0:mc.x-radius;
    int xe=(mc.x+radius>=brd->width)?brd->width-1:mc.x+radius;
    int ys=(mc.y-radius<0)?0:mc.y-radius;
    int ye=(mc.y+radius>=brd->height)?brd->height-1:mc.y+radius;
    std::cout<<"SE: "<<xs<<" "<<xe<<" "<<ys<<" "<<ye<<"\n:";
    this->playSound("Explosives","Explode");
    this->removeElement();
    for (int x=xs;x<=xe;x++)
    {
        for(int y=ys;y<=ye;y++)
        {
            float distance;
            distance=sqrt((x-mc.x)*(x-mc.x)+(y-mc.y)*(y-mc.y));
            if(distance<=radius)
                brd->getElement((coords){x,y})->destroy();
        }
    }
    this->disposeElement();
    return true;

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
