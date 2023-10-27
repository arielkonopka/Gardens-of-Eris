#include "viewPoint.h"


std::once_flag viewPoint::once;
viewPoint* viewPoint::instance=nullptr;
void viewPoint::setOwner(std::shared_ptr<bElem> owner)
{

    std::mutex my_mutex;
    std::lock_guard<std::mutex> lock(my_mutex);
    this->_owner=owner;
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



viewPoint* viewPoint::get_instance()
{

    std::call_once(once, []()
    {
        viewPoint::instance = new viewPoint();
    });
    return viewPoint::instance;

}



