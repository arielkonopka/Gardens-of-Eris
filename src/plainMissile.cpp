#include "plainMissile.h"


videoElement::videoElementDef* plainMissile::vd=NULL;
plainMissile::plainMissile(chamber *mychamber) : killableElements(mychamber,true)
{
    this->setEnergy(_plainMissileEnergy);
    this->setMoved(_plainMissileSpeed);
    this->setDirection(UP);
    this->setMoved(_plainMissileSpeed);
    this->statsOwner=NULL;
}
plainMissile::plainMissile(chamber* mychamber, int energy) : killableElements(mychamber,true)
{
    this->setEnergy(energy);
    this->setMoved(_plainMissileSpeed);
    this->setDirection(UP);
    this->myInventory=new inventory(this ); // This is for a mod, that could be installed on the ammo
    this->setMoved(_plainMissileSpeed);
    this->statsOwner=NULL;
}

plainMissile::~plainMissile()
{
    if(this->statsOwner!=NULL)
    {
        this->statsOwner->unlockThisObject(this);
        this->statsOwner=NULL;
    }
    //dtor
}
int plainMissile::getType()
{
    return _plainMissile;
}

int plainMissile::getSubtype()
{
    return 0;
}

videoElement::videoElementDef* plainMissile::getVideoElementDef()
{
    return plainMissile::vd;
}


bool plainMissile::mechanics(bool collected)
{
    bool res;
    int mvd=this->getMoved();
    res=killableElements::mechanics(collected);
    if(this->isDying())
        return true;
    if (this->getMoved()==0 && mvd==0)
    {
        bElem *myel=this->getElementInDirection(this->getDirection());
        if(myel==NULL)
        {
            //that is a dirty hack on situations that are bogus (should not happen, but happen sometimes)
            if (this->moveInDirectionSpeed(this->getDirection(),_plainMissileSpeed)==false) //if it is out of bounds, just vanish
                this->disposeElement();
            return true;
        }
        if (myel->isSteppable()==true)
        {
            this->moveInDirectionSpeed(this->getDirection(),_plainMissileSpeed);
            return true;
        }
        if (myel->canBeKilled()==true)
        {
            if (myel->getType()==this->getType() && myel->getDirection()==this->getDirection() && myel->getSubtype()==this->getSubtype())
            {
                return false;
            }
            int energy=this->getEnergy();
            if(this->statsOwner!=NULL)
            {
                if(this->statsOwner->getStats()->getDexterity()<_dexterityLevels)
                    energy=this->getEnergy()- (bElem::randomNumberGenerator()% (_dexterityLevels-this->statsOwner->getStats()->getDexterity()));
            }
            myel->hurt(energy);
            if(this->statsOwner!=NULL)
            {
                this->statsOwner->getStats()->countHit(myel);
            }
            if(!myel->isDying())
            {
                this->kill();
            }
            else
            {
                if(this->statsOwner!=NULL)
                {
                    this->statsOwner->getStats()->countKill(myel);
                }
                if (!this->isDying())
                    this->disposeElement();
            }
            return true;
        }
        if(myel->isDying()) // if next element in path is already dying, just disappear.
            this->disposeElement();
        this->kill();
        return true;
    }
    return res;
}
void plainMissile::setStatsOwner(bElem* owner)
{
    if(owner!=NULL)
    {
        this->statsOwner=owner;
        // this->owner->lockThisObject(this);
    }
}



