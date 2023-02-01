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
    if (this->getCollector().get()!=nullptr)
    {
        this->ps(this->getCollector(),eventType,event);
    }
    else if(this->getBoard().get()!=nullptr)
    {
        this->ps(shared_from_this(),eventType,event);
    }
    else if (this->getStomper().get()!=nullptr)
    {
        this->ps(this->getStomper(),eventType,event);
    }
    else if (this->getSteppingOnElement().get()!=nullptr)
    {
        this->ps(this->getSteppingOnElement(),eventType,event);
    }
}

void audibleElement::ps(std::shared_ptr<bElem>who, std::string eventType, std::string event)
{
    coords3d c3d;
    c3d.x=who->getCoords().x*32+who->getOffset().x;
    c3d.z=who->getCoords().y*32+who->getOffset().y;
    c3d.y=0;
    coords3d vel= {who->getOffset().x,0,who->getOffset().y};
    soundManager::getInstance()->registerSound(who->getBoard()->getInstanceId(),c3d,vel,this->getInstanceid(),this->getType(),this->getSubtype(),eventType,event);


}



