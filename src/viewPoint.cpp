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
    float radius,hradius;
    int dh=divider/2;
    for (unsigned long int c=0; c<viewPoints.size();)
    {
        auto wp = viewPoints[c].lock();
        if (!wp || wp->getStats()->isDisposed() )
        {
            viewPoints.erase(viewPoints.begin()+c);
            continue;
        } else if(wp->getBoard() && wp->getBoard()->getInstanceId()==ownerId)
        {
            radius = wp->getViewRadius()*divider;
            coords viewPointPos = (wp->getStats()->getMyPosition()*divider) + (wp->getOffset()*divider)/64+dh;
            float dist=viewPointPos.distance(point);
            if(dist>radius && dist<radius+0.8 && obscured>1024)
            {
                obscured=1024;
            }
            else if(dist<=radius/3)
            {
                obscured=0;
            }
            else if (dist<=radius)
            {
           //     hradius=2*radius/3;
           //     float hdist=dist-hradius;
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



