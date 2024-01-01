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
#include "plainMissile.h"


plainMissile::plainMissile(std::shared_ptr<chamber> mychamber) : plainMissile()
{
    this->setBoard(mychamber);
}

plainMissile::plainMissile(std::shared_ptr<chamber> mychamber, int energy) : plainMissile(mychamber)
{
}
plainMissile::plainMissile():bElem()
{
    this->getStats()->setWaiting(_plainMissileSpeed);
    this->getStats()->setMyDirection(UP);
    this->getStats()->setMoved(0);


}

bool plainMissile::additionalProvisioning(int subtype, std::shared_ptr<plainMissile>sbe)
{
    if(!bElem::additionalProvisioning(subtype,sbe))
        return false;
    this->registerLiveElement(shared_from_this());
    return true;
}




int plainMissile::getType() const
{
    return _plainMissile;
}

bool plainMissile::stepOnAction(bool step, std::shared_ptr<bElem>who)
{
    std::shared_ptr<bElem> sowner=this->getStats()->getStatsOwner().lock();
    if(step && who->getType()!=this->getType())
    {
        int w=who->getAttrs()->getEnergy();
        int dw=0;
        who->hurt(this->getAttrs()->getEnergy());
        dw=w-who->getAttrs()->getEnergy();
        this->kill();
        if(sowner)
        {
            sowner->getStats()->setPoints(SHOOT,sowner->getStats()->getPoints(SHOOT)+1);
            if(dw!=0)
                sowner->getStats()->setPoints(TOTAL,sowner->getStats()->getPoints(TOTAL)+dw);
        }
    }
    return true;
}






bool plainMissile::mechanics()
{
    bool res;
    std::shared_ptr<bElem> sowner=this->getStats()->getStatsOwner().lock();
    res=bElem::mechanics();
    if(!res) return false;
    if(this->getStats()->isDying() || this->getStats()->isMoving() || this->getStats()->isWaiting())
        return true;
    std::shared_ptr<bElem> myel=this->getElementInDirection(this->getStats()->getMyDirection());
    if(myel==nullptr || myel->getStats()->isDying() || myel->getStats()->isTeleporting() || myel->getStats()->isDestroying())
    {
        this->disposeElement();
        return true;
    }
    if (myel->getAttrs()->isSteppable()==true)
    {
        this->moveInDirectionSpeed(this->getStats()->getMyDirection(),_plainMissileSpeed);
        return true;
    }
    if (myel->getAttrs()->isKillable()==true)
    {
        int w=myel->getAttrs()->getEnergy();
        int dw=0;
        myel->hurt(this->getAttrs()->getEnergy());
        dw=w-myel->getAttrs()->getEnergy();
        if(!myel->getStats()->isDying() && !myel->getStats()->isDestroying())
        {
            this->kill();
            if(sowner)
            {
                sowner->getStats()->setPoints(SHOOT,sowner->getStats()->getPoints(SHOOT)+1);
                if (dw!=0)
                    sowner->getStats()->setPoints(TOTAL,sowner->getStats()->getPoints(TOTAL)+dw);

            }
        }
        else
        {
            if (!this->getStats()->isDying())
            {
                this->disposeElement();
                if(sowner)
                {
                    sowner->getStats()->setPoints(SHOOT,sowner->getStats()->getPoints(SHOOT)+1);
                    if (dw!=0)
                        sowner->getStats()->setPoints(TOTAL,sowner->getStats()->getPoints(TOTAL)+dw);
                }
            }
        }
        return true;
    }
    if(myel->getStats()->isDying()|| myel->getStats()->isDestroying()) // if next element in path is already dying, just disappear.
        this->disposeElement();
    this->kill();
    return true;

}




