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
#include "goldenApple.h"


unsigned int goldenApple::appleNumber = 0;
std::vector<std::shared_ptr<bElem>> goldenApple::apples;


int goldenApple::getType() const
{
    return _goldenAppleType;
}

bool goldenApple::hurt(int points)
{
    if (this->getAttrs()->getSubtype() != 1)
    {
        this->getAttrs()->setSubtype(1);
        this->getAttrs()->setInteractive(true);
        for (unsigned int cnt = 0; cnt < goldenApple::apples.size();)
        {
            if (!goldenApple::apples[cnt] || goldenApple::apples[cnt]->getStats()->getInstanceId() == this->getStats()->getInstanceId())
            {
                goldenApple::apples.erase(goldenApple::apples.begin() + cnt);
            }
            else
                cnt++;
        }
    }
    goldenApple::appleNumber=goldenApple::apples.size();
    return bElem::hurt(points);
}

bool goldenApple::destroy()
{

    if(!this->getStats()->isDestroying())
        return this->explode(2.5);
    return false;
}




bool goldenApple::additionalProvisioning(int subtype, std::shared_ptr<goldenApple>sbe)
{
    if(!bElem::additionalProvisioning(subtype,sbe))
        return false;
    if(subtype==0)
    {
        goldenApple::apples.push_back(shared_from_this());
    }
    goldenApple::appleNumber=goldenApple::apples.size();
    return true;

}


oState goldenApple::disposeElement()
{
    for (unsigned int cnt = 0; cnt < goldenApple::apples.size();)
    {
        if (goldenApple::apples[cnt]->getStats()->getInstanceId() == this->getStats()->getInstanceId())
        {
            goldenApple::apples.erase(goldenApple::apples.begin() + cnt);
        }
        else
            cnt++;
    }
    goldenApple::appleNumber=goldenApple::apples.size();
    return bElem::disposeElement();
}

int goldenApple::getAppleNumber()
{
    return goldenApple::appleNumber;
}

std::shared_ptr<bElem> goldenApple::getApple(int num)
{
    return goldenApple::apples.at(num);
}

bool goldenApple::kill()
{
    return this->destroy();
}
bool goldenApple::mechanics()
{
    std::cout<<" * appleMEch\n";
    if (!this->getStats()->isActive() || !explosives::mechanics())
        return false;
    std::shared_ptr<bElem> _owner=this->getStats()->getCollector().lock();
    if ( this->getAttrs()->getSubtype() == 0 || !this->getStats()->isCollected() || !_owner || !_owner->getBoard() )
    {
        return false;
    }
    int e = _owner->getAttrs()->getEnergy();
    if (e < 100)
    {
        this->getStats()->setWaiting(55);
        _owner->getAttrs()->setEnergy(e + 5);
        this->hurt(5);
    }
    return true;
}

bool goldenApple::interact(std::shared_ptr<bElem>who)
{
    if(!this->getAttrs()->isInteractive() || !who || !bElem::interact(who))
        return false;
    this->getStats()->setWaiting(55);
    this->getStats()->setInteracted(55);
    who->getAttrs()->setEnergy(who->getAttrs()->getEnergy() + 5);
    this->hurt(5);
    return true;
}

/***
 * @brief collectOnAction - when collected, checks, if it is a first collect by the collector, if so, sets itself active.
 * enables its own mechanics when collected and is of subtype other than zero
 */
bool goldenApple::collectOnAction(bool collected, std::shared_ptr<bElem>who)
{
    if(collected && who && this->getAttrs()->getSubtype()!=0)
    {
        if(who->getAttrs()->getInventory()->countTokens(this->getType(),this->getAttrs()->getSubtype())<=1)
            this->getStats()->setActive(true);
        this->registerLiveElement(shared_from_this());
    }
    else
    {
        this->getStats()->setActive(false);
        this->deregisterLiveElement(this->getStats()->getInstanceId());
        this->getStats()->setActivatedMechanics(false);
    }
    bool r=bElem::collectOnAction(collected,who);
    return r;
}


