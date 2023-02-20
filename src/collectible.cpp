#include "collectible.h"
videoElement::videoElementDef* collectible::vd=nullptr;

collectible::collectible(std::shared_ptr<chamber> board):audibleElement(board)
{
}
collectible::collectible():audibleElement()
{

}


bool collectible::isCollectible()
{
    return true;
}
videoElement::videoElementDef* collectible::getVideoElementDef()
{
    return collectible::vd;
}

void collectible::setCollected(std::shared_ptr<bElem> who)
{
    std::shared_ptr<bElem> wo=this->getCollector();
    bElem::setCollected(who);
    if(who.get()!=nullptr && who->getBoard().get()!=nullptr && (who->getType()==_player || this->getType()==_goldenAppleType) && wo.get()!=who.get())
    {
        this->playSound("Found","Collect");
    }

}
