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

/*
 * (y−yA)(xB−xA)−(yB−yA)(x−xA)=0
 * (y−yA)=((yB−yA)(x−xA))/(xB−xA)
 * y=yA+((yB−yA)(x−xA))/(xB−xA)
*/

/*





bool explosives::explode(float radius)
{
    this->radius = radius * 2.5;
    if (!this->getStats()->isDisposed() && !this->getStats()->isDestroying())
    {
        std::shared_ptr<chamber> brd = (this->getStats()->isCollected()) ? this->getStats()->getCollector().lock()->getBoard() : this->getBoard();
        coords mc = (this->getStats()->isCollected()) ? this->getStats()->getCollector().lock()->getStats()->getMyPosition() : this->getStats()->getMyPosition();
        this->playSound("Explosives", "Explode");

        std::shared_ptr<bElem> sowner = this->getStats()->getStatsOwner().lock();
        if (sowner)
        {
            for (int _direction = 0; _direction < 4; _direction++)
            {
                auto dr = (dir::direction)_direction;
                std::shared_ptr<bElem> inDir = this->getElementInDirection(dr);
                if (inDir && (inDir->getAttrs()->isDestroyable() || inDir->getAttrs()->isKillable()))
                {
                    sowner->getStats()->setPoints(SHOOT, sowner->getStats()->getPoints(SHOOT) + 1);
                    sowner->getStats()->setPoints(TOTAL, sowner->getStats()->getPoints(TOTAL) + inDir->getAttrs()->getEnergy());
                }
            }
        }

        int numRays = static_cast<int>(2.0f * M_PI * this->radius); // liczba promieni proporcjonalna do obwodu

        for (int i = 0; i < numRays; ++i)
        {
            float angle = (2.0f * M_PI * i) / numRays;
            float dx = cos(angle);
            float dy = sin(angle);

            for (float r = 0; r <= this->radius; r += 0.5f)
            {
                int x = static_cast<int>(mc.x + r * dx);
                int y = static_cast<int>(mc.y + r * dy);
                if (myUtility::Coords(x,y)==myUtility::Coords(this->getStats()->getMyPosition()))
                    continue;
                if (x < 0 || y < 0 || x >= brd->getSize().x || y >= brd->getSize().y)
                    break; // wyjdź, jeśli poza granicami

                coords currentPoint(x, y);
                std::shared_ptr<bElem> elem = brd->getElement(currentPoint);

                if (elem && elem->getAttrs()->isDestroyable())
                {
                    elem->destroy();
                }
                else
                {
                    break; // zatrzymaj się, jeśli napotkano niezniszczalny element
                }
            }
        }

        if (this->getStats()->isCollected())
        {
            this->getStats()->getCollector().lock()->destroy();
            viewPoint::get_instance()->addViewPoint(this->getStats()->getCollector().lock());
        }
        else
        {
            viewPoint::get_instance()->addViewPoint(shared_from_this());
        }

        bElem::destroy();
    }
    return true;
}
*/

/*bool explosives::explode(float radius)
{
    this->radius=radius*2.5;
    int per=radius*2*3.141592654;
    if(!this->getStats()->isDisposed() && !this->getStats()->isDestroying())
    {
        std::shared_ptr<chamber> brd = (this->getStats()->isCollected())?this->getStats()->getCollector().lock()->getBoard():this->getBoard();
        coords bsize=(brd)?brd->getSize():coords(0,0);
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
                coords ccrd=coords(x,y);
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
}*/

float explosives::getViewRadius() const
{
    return this->radius;
}


bool explosives::explode(float radius)
{
    this->radius=radius*3;
    this->brd=this->getBoard();
    if(!brd || this->getStats()->isDestroying() || this->getStats()->isDisposed())
        return false;
    viewPoint::get_instance()->addViewPoint(shared_from_this());
    this->bx=brd->getSize().x;
    this->by=brd->getSize().y;
    myUtility::Coords mpos=myUtility::Coords(this->getStats()->getMyPosition());
    bElem::destroy();
    viewPoint::get_instance()->addViewPoint(brd->getElement(mpos));
    traverser(mpos,mpos+myUtility::Coords::dir2coords(dir::direction::RIGHT),radius);
    traverser(mpos,mpos+myUtility::Coords::dir2coords(dir::direction::LEFT),radius);
    traverser(mpos,mpos+myUtility::Coords::dir2coords(dir::direction::UP),radius);
    traverser(mpos,mpos+myUtility::Coords::dir2coords(dir::direction::DOWN),radius);
    return true;

}
/***
 * @brief We will use cellular automata to resolve the shape of an explosion
 * @param  center - center of reference
 * @param point  - current point
 * @param radius - allowed radius
 * @return
 */
bool explosives::traverser(myUtility::Coords center, myUtility::Coords point, float radius)
{
    if (point.getX()>=bx || point.getX()<0 || point.getY()<0 ||point.getY()>=by ||(point.distance(center)>radius))
        return false;
    auto elem=brd->getElement(point);
    if(!elem || elem->getStats()->isDestroying())
        return false;
    if (!elem->getAttrs()->isDestroyable() && !elem->getAttrs()->isSteppable())
        return true;
    elem->destroy();
    traverser(center,point+myUtility::Coords::dir2coords(dir::direction::RIGHT),radius);
    traverser(center,point+myUtility::Coords::dir2coords(dir::direction::LEFT),radius);
    traverser(center,point+myUtility::Coords::dir2coords(dir::direction::UP),radius);
    traverser(center,point+myUtility::Coords::dir2coords(dir::direction::DOWN),radius);
    return true;
}

