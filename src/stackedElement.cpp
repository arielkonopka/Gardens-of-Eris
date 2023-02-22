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

void stackedElement::setController(std::shared_ptr<stackedElement> controller)
{
    this->controlUnit=controller;
}

bool stackedElement::stepOnElement(std::shared_ptr<bElem> step)
{
    bool res=true;
    if(getSubtype()==0)
    {
        for(unsigned int c=this->topDownConstruct.size(); c>0; c--)
        {
            if(this->topDownConstruct[c-1]->getInstanceid()==this->getInstanceid())
            {
                res=movableElements::stepOnElement(step->getBoard()->getElement(step->getCoords()));
                continue;
            }
            res=this->topDownConstruct[c-1]->stepOnElement(step->getBoard()->getElement(step->getCoords()));
            if(!res) // this should actually happen with the first element, the rest must be composed so the whole object can be created
                return res;
        }
    }
    else
        res=audibleElement::stepOnElement(step);
    return res;
}
std::shared_ptr<stackedElement> stackedElement::getController()
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
}
