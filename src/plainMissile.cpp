#include "plainMissile.h"


videoElement::videoElementDef* plainMissile::vd=NULL;
plainMissile::plainMissile(chamber *mychamber,gCollect *garbage) : movableElements(mychamber,garbageBin)
{
    this->power=_plainMissileEnergy;
    this->_me_moved=_plainMissileSpeed;
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
    bool kill;

    if (this->_me_moved==0)
    {
        coords newcoords=this->getAbsCoords(this->getDirection());
        if(newcoords==NOCOORDS)
        {
            this->kill();
            return true;
        }
        if (this->isSteppableDirection(this->getDirection()))
        {
            return this->moveInDirection(this->getDirection());
        }
        if (this->attachedBoard->getElement(newcoords.x,newcoords.y)->canBeKilled()==true)
        { //we hit something, now visual effect, the missile will vanish here (we could add something, here, like another kill state, but no)
            this->attachedBoard->getElement(newcoords.x,newcoords.y)->hurt(this->energy);
            this->disposeElement();
            return true;
        }
        this->kill();
    }

    return     movableElements::mechanics(collected);
}
