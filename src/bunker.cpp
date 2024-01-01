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

#include "bunker.h"
#include <elementFactory.h>

bunker::bunker(std::shared_ptr<chamber> board):bunker()
{
    this->setBoard(board);
    this->myGun->setBoard(board);



}

bunker::bunker(): bElem(), myGun(elementFactory::generateAnElement<plainGun>(nullptr,1))
{

}


bool bunker::additionalProvisioning(int subtype, std::shared_ptr<bunker>sbe)
{
    if(!bElem::additionalProvisioning(subtype,sbe))
        return false;
    this->registerLiveElement(shared_from_this());
    this->myGun->getStats()->setCollector(shared_from_this());
     this->myGun->getStats()->setCollected(true);
    return true;
}






bool bunker::mechanics()
{
    bool res=bElem::mechanics();
    if(!res || this->getStats()->isMoving() || this->getStats()->isWaiting() || this->myGun->getStats()->isWaiting())
        return false;
    int randomTest=bElem::randomNumberGenerator()%55;
    if(randomTest>25)
    {
        this->help=0;
        this->myGun->use(shared_from_this());
    }
    for(int c=0; c<4; c++)
    {
        direction d=(direction)c;
        std::shared_ptr<bElem> e=this->getElementInDirection(d);
        int dd=0;
        while(e && ++dd<this->brange)
        {
            if(e->getType()==_player)
            {
                this->getStats()->setFacing(d);
                this->getStats()->setMyDirection(d);
                this->myGun->use(shared_from_this());
            }
            if (!e->getAttrs()->isSteppable())
            {
                break;
            }
            e=e->getElementInDirection(d);
        }
    }
    this->getStats()->setWaiting((1+bElem::randomNumberGenerator()%55)*5);
    return res;
}

bool bunker::interact(std::shared_ptr<bElem> Who)
{
    if(!bElem::interact(Who))
        return false;
    this->help=5555;
    return true;
}

direction bunker::findLongestShot()
{
    int dir=0;
    int longest=0;
    direction longestDir=UP;
    std::shared_ptr<bElem> element;
    int routes[]= {0,0,0,0};
    for(dir=0; dir<4; dir++)
    {
        element=this->getElementInDirection((direction)(dir));
        if(element.get()==nullptr) continue;
        while(element->getAttrs()->isSteppable()==true)
        {
            routes[dir]++;
            element=element->getElementInDirection((direction)(dir));
            if (element.get()==nullptr)
                break;
        }
        if (element)
        {
            if (element->getAttrs()->isKillable()==true)
            {
                routes[dir]=655350; // We shoot here, the place, where something to be killed stands at
            }
        }
        if(routes[dir]>longest)
        {
            longest=routes[dir];
            longestDir=(direction)((dir));
        }
    }
    return longestDir;
}


bool bunker::selfAlign()
{
    if(this->getBoard())
        this->getStats()->setFacing(this->findLongestShot());
    return true;
}


int bunker::getType() const
{
    return _bunker;
}

