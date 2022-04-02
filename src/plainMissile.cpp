#include "plainMissile.h"


videoElement::videoElementDef* plainMissile::vd=NULL;
plainMissile::plainMissile(chamber *mychamber,gCollect *garbage) : movableElements(mychamber,garbageBin)
{
    this->energy=_plainMissileEnergy;
    this->_me_moved=_plainMissileSpeed;
    this->setDirection(UP);

}
plainMissile::plainMissile(chamber* mychamber, gCollect* garbage, int energy):movableElements(mychamber,garbage)
{
    this->energy=energy;
    this->_me_moved=_plainMissileSpeed;
    this->setDirection(UP);
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
    res=movableElements::mechanics(collected);
    if(this->isDying())
        return true;
    if (this->_me_moved==0)
    {
        bElem *myel=this->getElementInDirection(this->getDirection());
        if(myel==NULL)
        {
            this->kill();
            return false;
        }
        if (myel->isSteppable()==true)
        {
            this->moveInDirection(this->getDirection());
            return true;
        }
        if (myel->canBeKilled()==true)
        {
            myel->hurt(this->energy);
            this->disposeElement();
            return true;
        }
        this->kill();
        return true;
    }
    return res;
}


bool plainMissile::canBeKilled()
{
    return (!this->isDying());
}
