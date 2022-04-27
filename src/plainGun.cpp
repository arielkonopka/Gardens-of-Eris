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
    this->myInventory=new inventory();
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
    this->shot=_plainMissileSpeed+_mov_delay;
    if (this->ammo<=0 || who==NULL) //odd subtypes have infinite shots
        if (this->getSubtype()%2)
            return false;
    if(who->getType()==_player)
        std::cout<<"energy: "<<this->getEnergy()<<"\n";
    myel=who->getElementInDirection(who->getDirection());
    if(myel!=NULL)
    {

        int ener=0;
        if (who->getStats().dexterity>0)
        {
            ener= this->randomNumberGenerator()%(_dexterityLevels-who->getStats().dexterity);
        }

        if (myel->isSteppable()==true)
        {
            bElem* missile=new plainMissile(this->attachedBoard,this->garbageBin,this->getEnergy()-ener);
            missile->stepOnElement(myel);
            missile->setDirection(who->getDirection());

        }
        else if ( myel->getType()==_plainMissile && myel->getDirection()==this->getDirection())
        {
            myel->setEnergy(myel->getEnergy()+ener);
        }
        else if (myel->canBeKilled() )
        {
            myel->hurt(this->getEnergy());

            // this->disposeElement();
        }
    }
    if (this->ammo>0)
        if (this->getSubtype()%2==0)
            this->ammo--;
    this->setEnergy(this->getEnergy()/2);
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
    if(this->getEnergy()<this->maxEnergy)
    {
        if (this->taterCounter%3==0)
            this->setEnergy(this->getEnergy()+1);
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



