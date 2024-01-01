/*
 * Copyright (c) 2023, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "stackedElement.h"

stackedElement::stackedElement():bElem()
{
    //ctor
}
stackedElement::stackedElement(std::shared_ptr<chamber> board):stackedElement()
{
    this->setBoard(board);
}



bool stackedElement::additionalProvisioning(int st, std::shared_ptr<stackedElement>sbe)
{
    return bElem::additionalProvisioning(st,sbe);
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
            if(this->topDownConstruct[c-1]->getStats()->getInstanceId()!=this->getStats()->getInstanceId())
            {
                res=this->topDownConstruct[c-1]->stepOnElement(step->getBoard()->getElement(step->getStats()->getMyPosition()));
            }
            if(!res) // this should actually happen only with the first element, the rest must be composed so the whole object can be created
                break;
        }
    }
    std::shared_ptr<bElem> be=step->getBoard()->getElement(step->getStats()->getMyPosition());
    res=bElem::stepOnElement(be);
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
            if(this->getStats()->getInstanceId()==this->topDownConstruct[c]->getStats()->getInstanceId())
            {
                cnt=bElem::removeElement();
                continue;
            }
            this->topDownConstruct[c]->removeElement();
        }
        return cnt;

    }
    else
        return bElem::removeElement();
}

void stackedElement::linkAnElement(std::shared_ptr<stackedElement> newBottom)
{
    this->topDownConstruct.push_back(newBottom);
    if(this->getStats()->getInstanceId()!=newBottom->getStats()->getInstanceId()) newBottom->setController(shared_from_this());
}
