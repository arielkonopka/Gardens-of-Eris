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
/*
void plainMissile::stomp(std::shared_ptr<bElem> who)
{
    bElem::stomp(who);
    if(who->getType()!=this->getType())
    {
        who->hurt(this->attrs->getEnergy());
        this->disposeElement();
    }
    return;
}

*/
videoElement::videoElementDef* plainMissile::getVideoElementDef()
{
    return plainMissile::vd;
}



bool plainMissile::mechanics()
{
    bool res;
    int mvd=this->status->getMoved();
    res=killableElements::mechanics();
    if(!res) return false;
    if(this->status->isDying() || this->status->isMoving() || this->status->isWaiting())
        return true;
    if (this->status->getMoved()==0 && mvd==0)
    {
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
            /*  if(this->statsOwner!=nullptr && this->statsOwner->getStats()!=nullptr)
              {
                  int ep=(100*4)/(_dexterityLevels+1-this->statsOwner->getStats()->getDexterity());
                  int dp=100-ep;
                  int r=this->randomNumberGenerator()%(100+1);
                  this->attrs->setEnergy(((r<=dp)?ep/2:ep)+1);
              }
              */
            myel->hurt(this->attrs->getEnergy());
            /* if(this->statsOwner!=nullptr && this->statsOwner->getStats()!=nullptr)
             {
                 this->statsOwner->getStats()->countHit(myel);
             }*/
            if(!myel->status->isDying() && !myel->status->isDestroying())
            {
                this->kill();
            }
            else
            {
                /*  if(this->statsOwner!=nullptr && this->statsOwner->getStats()!=nullptr)
                  {
                      this->statsOwner->getStats()->countKill(myel);
                  }*/
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
    return res;
}
void plainMissile::setStatsOwner(std::shared_ptr<bElem> owner)
{
    if(owner!=nullptr)
    {
        this->statsOwner=owner;
        this->attrs->setEnergy(this->attrs->getEnergy());

    }
}



