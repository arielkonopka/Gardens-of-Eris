#include "plainMissile.h"


videoElement::videoElementDef* plainMissile::vd=NULL;
plainMissile::plainMissile(chamber *mychamber,gCollect *garbage) : killableElements(mychamber,garbageBin)
{
    this->setEnergy(_plainMissileEnergy);
    this->_me_moved=_plainMissileSpeed;
    this->setDirection(UP);

}
plainMissile::plainMissile(chamber* mychamber, gCollect* garbage, int energy):killableElements(mychamber,garbage)
{
    this->setEnergy(energy);
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
    res=killableElements::mechanics(collected);
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
            myel->hurt(this->getEnergy());
            this->disposeElement();
            return true;
        }
        this->kill();
        return true;
    }
    return res;
}



