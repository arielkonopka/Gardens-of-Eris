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
    if (this->readyToShoot()==false)
        return true; //The gun is fine, not ready to shoot though
    this->shot=_plainMissileSpeed*4;
    if (this->ammo<=0 || who==NULL) //odd subtypes have infinite shots
        if (this->getSubtype()%2)
            return false;
    if(who->getType()==_player)
        std::cout<<"energy: "<<this->energy<<"\n";
    myel=who->getElementInDirection(who->getDirection());
    if(myel!=NULL)
    {

        if (myel->isSteppable()==true)
        {
            bElem* missile=new plainMissile(this->attachedBoard,this->garbageBin,this->energy);
            missile->stepOnElement(myel);

            missile->setDirection(who->getDirection());
        }
        else if (myel->canBeKilled())
        {
            myel->hurt(_plainMissileEnergy);

            // this->disposeElement();
        }
    }
    if (this->ammo>0)
        if (this->getSubtype()%2==0)
            this->ammo--;
    this->energy=this->energy/2;
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
        if (this->taterCounter%3==0)
            this->energy++;
    }
    return res;
}

int plainGun::getAmmo()
{
    return this->ammo;
}

void plainGun::setAmmo(int ammo)
{
    this->ammo=ammo;
}



