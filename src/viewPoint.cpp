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
#include "viewPoint.h"


std::once_flag viewPoint::once;
viewPoint* viewPoint::instance=nullptr;

bool viewPoint::isElementInVector(const std::vector<std::weak_ptr<bElem>>& vec, const std::shared_ptr<bElem>& elem)
{
    return std::any_of(vec.begin(), vec.end(), [&elem](const std::weak_ptr<bElem>& wp)
    {
        return !wp.expired() && wp.lock()->getStats()->getInstanceId() == elem->getStats()->getInstanceId();
    });
}



void viewPoint::setOwner(std::shared_ptr<bElem> owner)
{
    this->addViewPoint(owner);
    std::mutex my_mutex;
    std::lock_guard<std::mutex> lock(my_mutex);
    this->_owner=owner;
}

void viewPoint::addViewPoint(std::shared_ptr<bElem> vp)
{
//   std::mutex my_mutex;
    //  std::lock_guard<std::mutex> lock(my_mutex);
    if(!this->isElementInVector(this->viewPoints,vp))
        this->viewPoints.push_back(vp);
}



std::shared_ptr<bElem> viewPoint::getOwner()
{
    return this->_owner.lock();
}

coords viewPoint::getViewPoint()
{
    std::shared_ptr<bElem> be=this->getOwner();
    if(be && be->getStats()->getMyPosition()!=NOCOORDS)
        return be->getStats()->getMyPosition();
    else
    {
        this->setOwner(player::getActivePlayer());
        be=this->getOwner();
        if(be)
            return be->getStats()->getMyPosition();

    }
    return NOCOORDS;
}
coords viewPoint::getViewPointOffset()
{
    std::shared_ptr<bElem> be=this->getOwner();
    if(be && be->getStats()->getMyPosition()!=NOCOORDS)
        return be->getOffset();
    else
    {
        this->setOwner(player::getActivePlayer());
        be=this->getOwner();
        if(be)
            return be->getOffset();
    }
    return NOCOORDS;
}

/**
 * Calculates the amount of obscuration at a given point with the divider equal 1
 *
 * @param point The point to calculate the obscuration for.
 * @return The amount of obscuration at the point, or 4096 if no owner is found, 1024 when the object is shown, but obscured fully.
 */
int viewPoint::calculateObscured(coords point)
{
    return this->calculateObscured(point,1);
}

/**
 * Calculates the amount of obscuration at a given point.
 *
 * @param point The point to calculate the obscuration for.
 * @param divider The divider to use for distance calculations.
 * @return The amount of obscuration at the point, or 4096 if no owner is found, 1024 when the object is shown, but obscured fully.
 */
int viewPoint::calculateObscured(const coords point,int divider)
{
    auto owner = getOwner();
    if (!owner)
    {
        return 4096;
    }
    int ownerId = owner->getBoard()->getInstanceId();
    int obscured = 4096;
    float radius;
    int dh=divider/2;
    for (unsigned long int c=0; c<viewPoints.size();)
    {
        auto wp = viewPoints[c].lock();
        if (!wp || wp->getStats()->isDisposed() )
        {
            viewPoints.erase(viewPoints.begin()+c);
            continue;
        }
        else if(wp->getBoard() && wp->getBoard()->getInstanceId()==ownerId)
        {
            radius = wp->getViewRadius()*divider;
            coords viewPointPos = (wp->getStats()->getMyPosition()*divider) + (wp->getOffset()*divider)/64+(dh*(8+4+2))/16;
            float dist=viewPointPos.distance(point);
            if(dist>radius && dist<radius+0.8 && obscured>1024)
            {
                obscured=1024;
            }
            else if(dist<=0.001)
            {
                obscured=0;
            }
            else if(dist<=radius/3)
            {
                obscured=1;
            }
            else if (dist<=radius)
            {
                int dst2 = (255*dist/radius);
                obscured=std::min(obscured,dst2);
            }
        }
        ++c;
    }
    return obscured;
}

bool viewPoint::isPointVisible(coords point)
{
    return this->calculateObscured(point)<1025;
}

viewPoint* viewPoint::get_instance()
{

    std::call_once(once, []()
    {
        viewPoint::instance = new viewPoint();
    });
    return viewPoint::instance;

}

std::vector<vpPoint> viewPoint::getViewPoints()
{

    auto owner = getOwner();
    int ownerId;
    if(!owner)
        ownerId=-1;
    else
        ownerId = owner->getBoard()->getInstanceId();
    auto resA=std::views::filter(this->viewPoints,[&](std::weak_ptr<bElem> be)
    {
       if(be.expired()) return false;
        return be.lock()->getBoard() && be.lock()->getBoard()->getInstanceId()==ownerId;
    }) | std::views::transform([](auto e){
        std::shared_ptr<bElem> be = e.lock();
        vpPoint vp;
        vp.x = be->getStats()->getMyPosition().x;
        vp.y = be->getStats()->getMyPosition().y;
        vp.radius = be->getViewRadius();
        return vp;
    }) |std::views::common;
    auto result=std::vector(resA.begin(), resA.end());
    return result;
}


