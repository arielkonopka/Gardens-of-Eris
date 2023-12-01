#include "bazooka.h"

bazooka::bazooka(std::shared_ptr<chamber> board):plainGun(board)
{
    //ctor
}

bazooka::~bazooka()
{
    //dtor
}

int bazooka::getType() const
{
    return _bazookaType;
}
std::shared_ptr<bElem> bazooka::createProjectible(std::shared_ptr<bElem>who)
{
    std::shared_ptr<bElem> pm=elementFactory::generateAnElement<bazookaMissile>(who->getBoard(),0);
    pm->getStats()->setStatsOwner(who);
    who->lockThisObject(pm);
    pm->getStats()->setMyDirection(who->getStats()->getFacing());
    pm->getStats()->setFacing(who->getStats()->getFacing());
    pm->stepOnElement(who->getElementInDirection(who->getStats()->getFacing()));
    pm->getAttrs()->setEnergy(this->getAttrs()->getEnergy());
    if(who->getType()==_player)
        viewPoint::get_instance()->setOwner(pm);
    pm->registerLiveElement(pm);
    return pm;
}
bool bazooka::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool bazooka::additionalProvisioning(int subtype, int typeId)
{
    return plainGun::additionalProvisioning(subtype,typeId);;
}

bool bazooka::additionalProvisioning(int subtype, std::shared_ptr<bazooka>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}
