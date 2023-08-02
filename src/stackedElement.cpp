#include "stackedElement.h"

stackedElement::stackedElement():movableElements()
{
    //ctor
}
stackedElement::stackedElement(std::shared_ptr<chamber> board):stackedElement()
{
    this->setBoard(board);
}

stackedElement::~stackedElement()
{
    //dtor
}


bool stackedElement::additionalProvisioning(int st, std::shared_ptr<stackedElement>sbe)
{
    return this->additionalProvisioning(st,sbe->getType());
}

bool stackedElement::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool stackedElement::additionalProvisioning(int subtype,int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
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
            if(this->topDownConstruct[c-1]->status->getInstanceId()!=this->status->getInstanceId())
            {
                res=this->topDownConstruct[c-1]->stepOnElement(step->getBoard()->getElement(step->status->getMyPosition()));
            }
            if(!res) // this should actually happen only with the first element, the rest must be composed so the whole object can be created
                break;
        }
    }
    std::shared_ptr<bElem> be=step->getBoard()->getElement(step->status->getMyPosition());
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
        for(unsigned int c=0; c<this->topDownConstruct.size(); c++)
        {
            if(this->status->getInstanceId()==this->topDownConstruct[c]->status->getInstanceId())
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
    if(this->status->getInstanceId()!=newBottom->status->getInstanceId()) newBottom->setController(shared_from_this());
}
