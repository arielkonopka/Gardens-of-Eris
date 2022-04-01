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
    bElem *myel;
    if (this->shot>0)
        return false;
    this->shot=10;
    if (this->ammo<=0 && (this->getSubtype()%2)==1) //odd subtypes have infinite shots
        return false;
    if(who==NULL)
        return false;
 //   this->shot=_mov_delay;
    myel=who->getElementInDirection(who->getDirection());
    if(myel==NULL)
        {
            if(this->ammo>0)
                this->ammo--;
            return false;
        }
    if (myel->isSteppable()==true)
    {
        bElem* missile=new plainMissile(this->attachedBoard,this->garbageBin,this->energy);
        missile->stepOnElement(myel);
        missile->setDirection(who->getDirection());
    } else if (myel->canBeKilled())
    {
        myel->hurt(_plainMissileEnergy);
       // this->disposeElement();
    }
    if (this->ammo>0)
        this->ammo--;
    return true;
}


bool plainGun::readyToShoot()
{
    return this->shot<=0;
}


bool plainGun::mechanics(bool collected)
{
    bool res=usable::mechanics(collected);
    if(this->shot>0)
    {
        this->shot--;
    }
    if(this->energy<this->maxEnergy)
    {
        this->energy++;
    }
    return res;
}
