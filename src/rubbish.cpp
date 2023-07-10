#include "rubbish.h"


videoElement::videoElementDef* rubbish::vd=nullptr;

rubbish::rubbish():collectible()
{

}
rubbish::rubbish(std::shared_ptr<chamber> board):collectible()
{
    this->setBoard(board);
}


int rubbish::getType() const
{
    return _rubishType;
}

videoElement::videoElementDef* rubbish::getVideoElementDef()
{
    return rubbish::vd;
}


bool rubbish::mechanics()
{
    std::shared_ptr<bElem> t=shared_from_this();
    this->deregisterLiveElement(this->status->getInstanceId());
    if( this->status->hasParent() && this->status->getStandingOn().lock()->attrs->canCollect())
        this->status->getStandingOn().lock()->collect(t);
    return false;
}
bool rubbish::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool rubbish::additionalProvisioning(int subtype,int typeId)
{
    bool res=bElem::additionalProvisioning(subtype,typeId);
    this->attrs->setEnergy(1);

    return res;
}

bool rubbish::additionalProvisioning(int subtype, std::shared_ptr<rubbish>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

