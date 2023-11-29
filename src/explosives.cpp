#include "explosives.h"
#include <viewPoint.h>

explosives::explosives(std::shared_ptr<chamber> board) : explosives()
{
    this->setBoard(board);
}

explosives::explosives() : bElem()
{
}

explosives::~explosives()
{
    // dtor
}
bool explosives::additionalProvisioning(int subtype, std::shared_ptr<explosives>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool explosives::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());

}

bool explosives::additionalProvisioning(int subtype, int typeId)
{

    return bElem::additionalProvisioning(subtype,typeId);
}

bool explosives::explode(float radius)
{
    if(!this->getStats()->isDisposed() && !this->getStats()->isDestroying())
    {
        coords mc=(this->getStats()->isCollected())?this->getStats()->getCollector().lock()->getStats()->getMyPosition():this->getStats()->getMyPosition();
        std::shared_ptr<chamber> brd = (this->getStats()->isCollected())?this->getStats()->getCollector().lock()->getBoard():this->getBoard();
        int xs=std::min(0,(int)(mc.x-radius));
        int xe=std::min(brd->width-1,(int)(mc.x+radius));
        int ys=std::min(0,(int)(mc.y-radius));
        int ye=std::min(brd->height-1,(int)(mc.y+radius));
        this->playSound("Explosives","Explode");
        for (int x=xs; x<=xe; x++)
        {
            for(int y=ys; y<=ye; y++)
            {
                coords ccrd=(coords) { x,y };
                if(ccrd==mc)
                {
                    if(this->getStats()->isCollected())
                    {
                        this->getStats()->getCollector().lock()->destroy();
                        viewPoint::get_instance()->addViewPoint(this->getStats()->getCollector().lock());
                    }
                    else
                        viewPoint::get_instance()->addViewPoint(shared_from_this());
                    bElem::destroy();
                    continue;
                }
                if(mc.distance(ccrd)<=radius)
                    brd->getElement(ccrd)->destroy();
            }
        }
    }
    return true;
}


bool explosives::explode()
{
    std::shared_ptr<bElem> el;
    if(this->getStats()->isDestroying())
        return false;
    this->playSound("Explosives","Explode"); // This sound must not be looped, otherwise it will be swiped off
    for (int cnt = 0; cnt < 4; cnt++)
    {
        el = this->getElementInDirection((direction)(cnt));
        if (el != nullptr)
        {
            el->destroy();
            el = el->getElementInDirection((direction)((cnt + 1) % 4));
            if (el != nullptr)
                el->destroy();
        }
    }
    if(this->getStats()->isCollected())
        this->getStats()->getCollector().lock()->destroy();
    bElem::destroy();


    return true;
}
