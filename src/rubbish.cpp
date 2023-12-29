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

