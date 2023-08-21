#include "plainMissile.h"


videoElement::videoElementDef* plainMissile::vd=nullptr;
plainMissile::plainMissile(std::shared_ptr<chamber> mychamber) : plainMissile()
{
    this->setBoard(mychamber);
}

plainMissile::plainMissile(std::shared_ptr<chamber> mychamber, int energy) : plainMissile(mychamber)
{
}
plainMissile::plainMissile():killableElements(),movableElements(),mechanical()
{
    this->statsOwner=nullptr;
    this->getStats()->setWaiting(_plainMissileSpeed);
    this->getStats()->setMyDirection(UP);
    this->getStats()->setMoved(0);


}

bool plainMissile::additionalProvisioning(int subtype, std::shared_ptr<plainMissile>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool plainMissile::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool plainMissile::additionalProvisioning(int subtype,int typeId)
{
    this->registerLiveElement(shared_from_this());
    return bElem::additionalProvisioning(subtype,typeId);
}

plainMissile::~plainMissile()
{
    if(this->statsOwner!=nullptr)
    {
        this->statsOwner->unlockThisObject(shared_from_this());
        this->statsOwner=nullptr;
    }
    //dtor
}
int plainMissile::getType() const
{
    return _plainMissile;
}

bool plainMissile::stepOnAction(bool step, std::shared_ptr<bElem>who)
{
    if(step && who->getType()!=this->getType())
    {
        who->hurt(this->getAttrs()->getEnergy());
        this->kill();
    }
    return true;
}


videoElement::videoElementDef* plainMissile::getVideoElementDef()
{
    return plainMissile::vd;
}



bool plainMissile::mechanics()
{
    bool res;

    res=killableElements::mechanics();
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
        myel->hurt(this->getAttrs()->getEnergy());
        if(!myel->getStats()->isDying() && !myel->getStats()->isDestroying())
        {
            this->kill();
        }
        else
        {
            if (!this->getStats()->isDying())
                this->disposeElement();
        }
        return true;
    }
    if(myel->getStats()->isDying()|| myel->getStats()->isDestroying()) // if next element in path is already dying, just disappear.
        this->disposeElement();
    this->kill();
    return true;

}




