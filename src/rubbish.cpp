#include "rubbish.h"


rubbish::rubbish():bElem()
{

}
rubbish::rubbish(std::shared_ptr<chamber> board):bElem()
{
    this->setBoard(board);
}


int rubbish::getType() const
{
    return _rubishType;
}




bool rubbish::mechanics()
{
    std::shared_ptr<bElem> t=shared_from_this();
    this->deregisterLiveElement(this->getStats()->getInstanceId());
    if( this->getStats()->hasParent() && this->getStats()->getStandingOn().lock()->getAttrs()->canCollect())
        this->getStats()->getStandingOn().lock()->collect(t);
    return false;
}
bool rubbish::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool rubbish::additionalProvisioning(int subtype,int typeId)
{
    bool res=bElem::additionalProvisioning(subtype,typeId);
    this->getAttrs()->setEnergy(1);

    return res;
}

bool rubbish::additionalProvisioning(int subtype, std::shared_ptr<rubbish>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

