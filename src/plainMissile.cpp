#include "plainMissile.h"


videoElement::videoElementDef* plainMissile::vd=NULL;
plainMissile::plainMissile(chamber *mychamber,gCollect *garbage) : killableElements(mychamber,garbageBin)
{
    this->setEnergy(_plainMissileEnergy);
    this->_me_moved=_plainMissileSpeed;
    this->setDirection(UP);

}
plainMissile::plainMissile(chamber* mychamber, gCollect* garbage, int energy) : killableElements(mychamber,garbage)
{
    this->setEnergy(energy);
    this->_me_moved=_plainMissileSpeed;
    this->setDirection(UP);
    this->myInventory=new inventory(); // This is for a mod, that could be installed on the ammo
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
            this->disposeElement();
            return true;
        }
        if (myel->isSteppable()==true)
        {
            this->moveInDirectionSpeed(this->getDirection(),_plainMissileSpeed);
            return true;
        }


        if(myel->isDying())
            this->disposeElement();
        this->kill();
        return true;
    }
    return res;
}



