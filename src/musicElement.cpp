#include "musicElement.h"

musicElement::musicElement():audibleElement(),mechanical()
{
    //ctor
}
musicElement::musicElement(std::shared_ptr<chamber>board):audibleElement(board),mechanical(board,true)
{

}

musicElement::~musicElement()
{
    //dtor
}
bool musicElement::mechanics()
{
    if(!bElem::mechanics()) return false;
    this->setWait(50);  /* We wait for a while to try to play it again, remember, it will get denied if wrong chamber */
    soundManager::getInstance()->registerMusic(this->getSubtype(),this->getBoard()->getInstanceId(),{this->getCoords().x,0,this->getCoords().y});
    return true;
}
