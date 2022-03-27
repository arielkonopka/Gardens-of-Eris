#include "plainGun.h"


videoElement::videoElementDef* plainGun::vd=NULL;

videoElement::videoElementDef* plainGun::getVideoElementDef()
{
    return plainGun::vd;
}

plainGun::plainGun(chamber *board,gCollect *garbage): usable(board,garbage)
{
    this->maxEnergy=_plainMissileEnergy;
    this->ammo=_plainGunAmmo;
}

int plainGun::getType()
{
    return _plainGun;
}

int plainGun::getSubtype()
{
    return 0;
}

plainGun::~plainGun()
{
    //dtor
}
bool plainGun::isWeapon()
{
    return true;
}
bool plainGun::use(bElem* who)
{
    coords newcoords=NOCOORDS;
    if (this->shot>0)
        return false;
    if (this->ammo<=0)
        return false;
    if(who==NULL)
        return false;
    newcoords=who->getAbsCoords(who->getDirection());
    if(newcoords==NOCOORDS)
        return false;
    if (who->isSteppableDirection(who->getDirection())==true)
    {
        bElem* missile=new plainMissile(this->attachedBoard,this->garbageBin,this->energy);
        missile->setDirection(who->getDirection());
        missile->stepOnElement(this->attachedBoard->getElement(newcoords.x,newcoords.y));
    }
    if (this->attachedBoard->getElement(newcoords.x,newcoords.y)->canBeKilled())
    {
        this->attachedBoard->getElement(newcoords.x,newcoords.y)->hurt(_plainMissileEnergy);
    }



}




bool plainGun::mechanics(bool collected)
{
    usable::mechanics(collected);
    if(this->shot>0)
    {

     this->shot--;
    }
    if(this->energy<this->maxEnergy)
    {
        this->energy++;
    }

}
