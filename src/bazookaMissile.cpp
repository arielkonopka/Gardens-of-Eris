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

#include "bazookaMissile.h"

bazookaMissile::bazookaMissile():explosives()
{

}

bool bazookaMissile::mechanics()
{
    if(!explosives::mechanics())
        return false;
    if(++this->steps<_bazookaMaxSteps && this->moveInDirectionSpeed(this->getStats()->getMyDirection(),_bazookaMissileSpeed) )
        return true;
    std::shared_ptr<bElem> be=this->getElementInDirection(this->getStats()->getMyDirection());
    if(this->steps>1 || !be)
        return this->explode(1.5);

    int beEnergy=be->getAttrs()->getEnergy();

    if(be)
        be->hurt(this->getAttrs()->getEnergy());
    beEnergy=beEnergy-be->getAttrs()->getEnergy();
    if(beEnergy)
    {
        std::shared_ptr<bElem> sowner=this->getStats()->getStatsOwner().lock();
        if(sowner)
        {
            sowner->getStats()->setPoints(SHOOT,sowner->getStats()->getPoints(SHOOT)+1);
            if(beEnergy!=0)
                sowner->getStats()->setPoints(TOTAL,sowner->getStats()->getPoints(TOTAL)+beEnergy);
        }
    }

    return bElem::kill();
}

int bazookaMissile::getType() const
{
    return _bazookaMissileType;
}


bool bazookaMissile::additionalProvisioning(int subtype, std::shared_ptr<bazookaMissile>sbe)
{
    return explosives::additionalProvisioning(subtype,sbe);
}
