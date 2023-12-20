#include "plainMissile.h"


plainMissile::plainMissile(std::shared_ptr<chamber> mychamber) : plainMissile()
{
    this->setBoard(mychamber);
}

plainMissile::plainMissile(std::shared_ptr<chamber> mychamber, int energy) : plainMissile(mychamber)
{
}
plainMissile::plainMissile():killableElements(),movableElements()
{
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


int plainMissile::getType() const
{
    return _plainMissile;
}

bool plainMissile::stepOnAction(bool step, std::shared_ptr<bElem>who)
{
    std::shared_ptr<bElem> sowner=this->getStats()->getStatsOwner().lock();
    if(step && who->getType()!=this->getType())
    {
        int w=who->getAttrs()->getEnergy();
        int dw=0;
        who->hurt(this->getAttrs()->getEnergy());
        dw=w-who->getAttrs()->getEnergy();
        this->kill();
        if(sowner)
        {
            sowner->getStats()->setPoints(SHOOT,sowner->getStats()->getPoints(SHOOT)+1);
            if(dw!=0)
                sowner->getStats()->setPoints(TOTAL,sowner->getStats()->getPoints(TOTAL)+dw);
        }
    }
    return true;
}






bool plainMissile::mechanics()
{
    bool res;
    std::shared_ptr<bElem> sowner=this->getStats()->getStatsOwner().lock();
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
        int w=myel->getAttrs()->getEnergy();
        int dw=0;
        myel->hurt(this->getAttrs()->getEnergy());
        dw=w-myel->getAttrs()->getEnergy();
        if(!myel->getStats()->isDying() && !myel->getStats()->isDestroying())
        {
            this->kill();
            if(sowner)
            {
                sowner->getStats()->setPoints(SHOOT,sowner->getStats()->getPoints(SHOOT)+1);
                if (dw!=0)
                    sowner->getStats()->setPoints(TOTAL,sowner->getStats()->getPoints(TOTAL)+dw);

            }
        }
        else
        {
            if (!this->getStats()->isDying())
            {
                this->disposeElement();
                if(sowner)
                {
                    sowner->getStats()->setPoints(SHOOT,sowner->getStats()->getPoints(SHOOT)+1);
                    if (dw!=0)
                        sowner->getStats()->setPoints(TOTAL,sowner->getStats()->getPoints(TOTAL)+dw);
                }
            }
        }
        return true;
    }
    if(myel->getStats()->isDying()|| myel->getStats()->isDestroying()) // if next element in path is already dying, just disappear.
        this->disposeElement();
    this->kill();
    return true;

}




