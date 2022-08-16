#include "plainMissile.h"


videoElement::videoElementDef* plainMissile::vd=NULL;
plainMissile::plainMissile(chamber *mychamber) : killableElements(mychamber,true)
{
    this->setEnergy(_plainMissileEnergy);
    this->_me_moved=_plainMissileSpeed;
    this->setDirection(UP);
    this->setMoved(_plainMissileSpeed);

}
plainMissile::plainMissile(chamber* mychamber, int energy) : killableElements(mychamber,true)
{
    this->setEnergy(energy);
    this->_me_moved=_plainMissileSpeed;
    this->setDirection(UP);
    this->myInventory=new inventory(); // This is for a mod, that could be installed on the ammo
    this->setMoved(_plainMissileSpeed);
}

plainMissile::~plainMissile()
{
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
    int mvd=this->_me_moved;
    res=killableElements::mechanics(collected);
    if(this->isDying())
        return true;
    if (this->_me_moved==0 && mvd==0)
    {
        bElem *myel=this->getElementInDirection(this->getDirection());
        if(myel==NULL)
        { //that is a dirty hack on situations that are bogus (should not happen, but happen sometimes)
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
            myel->hurt(this->getEnergy());
            if(!myel->isDying())
                this->kill();
            else
                this->disposeElement();
            return true;
        }
        if(myel->isDying()) // if next element in path is already dying, just disappear.
            this->disposeElement();
        this->kill();
        return true;
    }
    return res;
}



