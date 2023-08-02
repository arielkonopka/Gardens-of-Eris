#include "musicElement.h"

musicElement::musicElement():mechanical()
{
    //ctor
}
musicElement::musicElement(std::shared_ptr<chamber>board):mechanical(board,true)
{

}

musicElement::~musicElement()
{
    //dtor
}
bool musicElement::mechanics()
{
    if(!bElem::mechanics()) return false;
    this->status->setWaiting(50);  /* We wait for a while to try to play it again, remember, it will get denied if wrong chamber */
    soundManager::getInstance()->registerMusic(this->attrs->getSubtype(),this->getBoard()->getInstanceId(), {this->status->getMyPosition().x,0,this->status->getMyPosition().y});
    return true;
}
bool musicElement::additionalProvisioning(int subtype, std::shared_ptr<musicElement>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool musicElement::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool musicElement::additionalProvisioning(int subtype,int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}
