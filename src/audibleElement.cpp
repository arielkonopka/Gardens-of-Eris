#include "audibleElement.h"

audibleElement::audibleElement():bElem()
{
    //ctor
}
audibleElement::audibleElement(std::shared_ptr<chamber> board):bElem(board)
{

}

void audibleElement::playSound(std::string eventType, std::string event)
{
    if (!this->status->getCollector().expired())
    {
        this->ps(this->status->getCollector().lock(),eventType,event);
    }
    else if(this->getBoard())
    {
        this->ps(shared_from_this(),eventType,event);
    }
    else if (this->status->hasParent())
    {
        this->ps(this->status->getStandingOn().lock(),eventType,event);
    }
    else if (this->status->getSteppingOn())
    {
        this->ps(this->status->getSteppingOn(),eventType,event);
    }
}

void audibleElement::ps(std::shared_ptr<bElem>who, std::string eventType, std::string event)
{
    coords3d c3d;
    c3d.x=who->status->getMyPosition().x*32+who->getOffset().x;
    c3d.z=who->status->getMyPosition().y*32+who->getOffset().y;
    c3d.y=0;
    coords3d vel= {(who->getOffset().x)?32:0, 0,(who->getOffset().y>0)?32:0};

    soundManager::getInstance()->registerSound(who->getBoard()->getInstanceId(),c3d,vel,who->status->getInstanceId(),this->getType(),this->attrs->getSubtype(),eventType,event);
}
audibleElement::~audibleElement()
{
    // We stop all the sounds by that element
    soundManager::getInstance()->stopSoundsByElementId(this->status->getInstanceId());
}

bool audibleElement::additionalProvisioning(int subtype, std::shared_ptr<audibleElement>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());

}

bool audibleElement::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());

}

bool audibleElement::additionalProvisioning(int subtype, int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);

}


