#include "stackedElement.h"

stackedElement::stackedElement():movableElements()
{
    //ctor
}
stackedElement::stackedElement(std::shared_ptr<chamber> board):movableElements(board)
{

}

stackedElement::~stackedElement()
{
    //dtor
}

void stackedElement::setController(std::shared_ptr<bElem> controller)
{
    this->controlUnit=controller;
}

bool stackedElement::stepOnElement(std::shared_ptr<bElem> step)
{
    bool res=true;
    if(this->getController().get()==nullptr)
    {
        this->removeElement();
        for(unsigned int c=this->topDownConstruct.size(); c>0; c--)
        {
            if(this->topDownConstruct[c-1]->getInstanceid()!=this->getInstanceid())
            {
                res=this->topDownConstruct[c-1]->stepOnElement(step->getBoard()->getElement(step->getCoords()));
            }
            if(!res) // this should actually happen only with the first element, the rest must be composed so the whole object can be created
                break;
        }
    }
    std::shared_ptr<bElem> be=step->getBoard()->getElement(step->getCoords());
    res=movableElements::stepOnElement(be);
    return res;
}
std::shared_ptr<bElem> stackedElement::getController()
{
    return this->controlUnit;
}

std::shared_ptr<bElem> stackedElement::removeElement()
{
    std::shared_ptr<bElem> cnt=shared_from_this();
    if(this->getController().get()==nullptr)
    {
        for(unsigned int c=0;c<this->topDownConstruct.size();c++)
        {
            if(this->getInstanceid()==this->topDownConstruct[c]->getInstanceid())
                {
                    cnt=movableElements::removeElement();
                    continue;
                }
            this->topDownConstruct[c]->removeElement();
        }
        return cnt;

    }
    else
        return movableElements::removeElement();
}

void stackedElement::linkAnElement(std::shared_ptr<stackedElement> newBottom)
{
    this->topDownConstruct.push_back(newBottom);
    if(this->getInstanceid()!=newBottom->getInstanceid()) newBottom->setController(shared_from_this());
}
