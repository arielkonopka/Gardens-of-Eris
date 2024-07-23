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
#include "explosives.h"
#include <viewPoint.h>



bool explosives::additionalProvisioning(int subtype)
{
    return bElem::additionalProvisioning(subtype);
}


bool explosives::explode(float radius)
{
    this->radius=radius*2.5;
    if(!this->getStats()->isDisposed() && !this->getStats()->isDestroying())
    {
        std::shared_ptr<chamber> brd = (this->getStats()->isCollected())?this->getStats()->getCollector().lock()->getBoard():this->getBoard();
        coords bsize=(brd)?brd->getSize():(coords){0,0};
        coords mc=(this->getStats()->isCollected())?this->getStats()->getCollector().lock()->getStats()->getMyPosition():this->getStats()->getMyPosition();
        int xs=std::max(0,(int)(mc.x-(int)radius));
        int xe=std::min(bsize.x-1,(int)(mc.x+(int)radius));
        int ys=std::max(0,(int)(mc.y-(int)radius));
        int ye=std::min(bsize.y-1,(int)(mc.y+(int)radius));
        this->playSound("Explosives","Explode");
        std::shared_ptr<bElem> sowner=this->getStats()->getStatsOwner().lock();
        if(sowner)
        {
            for(int _direction=0; _direction<4; _direction++)
            {
                auto dr=(dir::direction)_direction;
                std::shared_ptr<bElem> inDir=this->getElementInDirection(dr);
                if(inDir && (inDir->getAttrs()->isDestroyable() || inDir->getAttrs()->isKillable()))
                {
                    sowner->getStats()->setPoints(SHOOT,sowner->getStats()->getPoints(SHOOT)+1);
                    sowner->getStats()->setPoints(TOTAL,sowner->getStats()->getPoints(TOTAL)+inDir->getAttrs()->getEnergy());
                }
            }

        }

        for (int x=xs; x<=xe; x++)
        {
            for(int y=ys; y<=ye; y++)
            {
                coords ccrd=(coords) { x,y };
                if(ccrd==mc)
                {
                    if(this->getStats()->isCollected())
                    {
                        this->getStats()->getCollector().lock()->destroy();
                        viewPoint::get_instance()->addViewPoint(this->getStats()->getCollector().lock());
                    }
                    else
                        viewPoint::get_instance()->addViewPoint(shared_from_this());
                    bElem::destroy();
                    continue;
                }
               if(std::round(mc.distance(ccrd))<=radius)
                    brd->getElement(ccrd)->destroy();
            }
        }
    }
    return true;
}

float explosives::getViewRadius() const
{
    return this->radius;
}

