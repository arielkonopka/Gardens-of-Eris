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
    this->status->setWaiting(_plainMissileSpeed);
    this->status->setMyDirection(UP);
    this->status->setMoved(0);


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
int plainMissile::getType()
{
    return _plainMissile;
}

bool plainMissile::stepOnAction(bool step, std::shared_ptr<bElem>who)
{
    if(step && who->getType()!=this->getType())
    {
        who->hurt(this->attrs->getEnergy());
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
    if(this->status->isDying() || this->status->isMoving() || this->status->isWaiting())
        return true;
    std::shared_ptr<bElem> myel=this->getElementInDirection(this->status->getMyDirection());
    if(myel==nullptr || myel->status->isDying() || myel->status->isTeleporting() || myel->status->isDestroying())
    {
        this->disposeElement();
        return true;
    }
    if (myel->attrs->isSteppable()==true)
    {
        this->moveInDirectionSpeed(this->status->getMyDirection(),_plainMissileSpeed);
        return true;
    }
    if (myel->attrs->isKillable()==true)
    {
        myel->hurt(this->attrs->getEnergy());
        if(!myel->status->isDying() && !myel->status->isDestroying())
        {
            this->kill();
        }
        else
        {
            if (!this->status->isDying())
                this->disposeElement();
        }
        return true;
    }
    if(myel->status->isDying()|| myel->status->isDestroying()) // if next element in path is already dying, just disappear.
        this->disposeElement();
    this->kill();
    return true;

}
void plainMissile::setStatsOwner(std::shared_ptr<bElem> owner)
{
    if(false && owner!=nullptr)
    {
        this->statsOwner=owner;
        this->attrs->setEnergy(this->attrs->getEnergy());

    }
}



