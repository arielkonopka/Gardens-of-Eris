#include "plainMissile.h"


videoElement::videoElementDef* plainMissile::vd=NULL;
plainMissile::plainMissile(chamber *mychamber,gCollect *garbage) : movableElements(mychamber,garbageBin)
{
    this->power=_plainMissileEnergy;
    this->_me_moved=_plainMissileSpeed;
    this->setDirection(UP);
}
plainMissile::plainMissile(chamber* mychamber, gCollect* garbage, int energy):movableElements(mychamber,garbage)
{
    this->power=energy;
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
                if (this->moveInDirection(this->getDirection())==false)
                {
                    std::cout<<"This should not happen!\n";
                    this->kill();
                }
                return true;
            }
        if (myel->canBeKilled()==true)
        {
            std::cout<<"I'm the pain!\n";
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
    return true;
}
