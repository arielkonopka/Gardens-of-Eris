#include "rubbish.h"


videoElement::videoElementDef* rubbish::vd=nullptr;

rubbish::rubbish():collectible()
{

}

rubbish::rubbish(std::shared_ptr<chamber> board):collectible(board)
{

}



videoElement::videoElementDef* rubbish::getVideoElementDef()
{
    return rubbish::vd;
}
void rubbish::stomp(std::shared_ptr<bElem> who)
{
    bElem::stomp(who);
    if(who->canCollect())
        this->registerLiveElement(shared_from_this());

}

bool rubbish::mechanics()
{
    std::shared_ptr<bElem> t=shared_from_this();
    this->deregisterLiveElement(this->getInstanceid());
    if( this->getStomper().get()!=nullptr && this->getStomper()->canCollect())
        this->getStomper()->collect(t);
    return false;
}
