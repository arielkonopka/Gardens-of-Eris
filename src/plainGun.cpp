#include "plainGun.h"


videoElement::videoElementDef* plainGun::vd=NULL;

videoElement::videoElementDef* plainGun::getVideoElementDef()
{
    return plainGun::vd;
}

plainGun::plainGun(chamber *board): usable(board,true)
{
    this->maxEnergy=_plainMissileEnergy;
    this->ammo=_plainGunAmmo;
    this->myInventory=new inventory(this);
}

plainGun::plainGun(chamber* board, int newSubtype): usable(board,true)
{
    this->maxEnergy=_plainMissileEnergy;
    this->ammo=_plainGunAmmo;
    this->myInventory=new inventory(this);
    this->setSubtype(newSubtype);
}




int plainGun::getType()
{
    return _plainGun;
}

plainGun::~plainGun()
{

}
bool plainGun::isWeapon()
{
    return true;
}
bool plainGun::use(bElem* who)
{
    bElem *myel;
    if (who==NULL)
    {
#ifdef _VerbousMode_
        std::cout<<"Who is null for plain gun!";
#endif
        return false;

    }
    if(who->getType()==_player)
    {
#ifdef _VerbousMode_
        std::cout<<"Shoot: "<<this->readyToShoot()<<" s "<<this->shot<<" Energy "<<this->getEnergy()<<"\n";
#endif
    }

    if (this->readyToShoot()==false)
        return true; //The gun is fine, not ready to shoot though
    this->shot=this->getCntr()+_plainGunCharge;
    if (this->ammo<=0) //odd subtypes have infinite shots
        if (this->getSubtype()%2)
            return false;
    myel=who->getElementInDirection(who->getDirection());
    if(myel!=NULL)
    {
        if (this->ammo>0 || this->getSubtype()%2==1)
        {
            if (myel->isSteppable()==true)
            {
                plainMissile* missile=new plainMissile(who->getBoard(),this->getEnergy());
               // if(who->getType()==_player)
               // {
                missile->setStatsOwner(who);
                who->lockThisObject(missile);
                //}
                missile->setDirection(who->getDirection());
                missile->stepOnElement(myel);
            }
            else if ( myel->getType()==_plainMissile && myel->getDirection()==who->getDirection())
            {
                myel->setEnergy(myel->getEnergy()+this->getEnergy()); // if somehow you hit a missile going in the same direction, the show would be boosted;
            }
            else if (myel->canBeKilled() )
            {
                who->getStats()->countKill(myel);
                myel->hurt(this->getEnergy());
                // this->disposeElement();
            }
            if (this->getSubtype()%2==0)
            {
                this->ammo--;
                this->setEnergy(this->getEnergy()-(this->getEnergy()*0.2));

            }
        }

    }
    return true;
}


bool plainGun::readyToShoot()
{
    return (this->shot<this->getCntr()); // the gun will jam on counter reset, but only this instance
}


bool plainGun::mechanics()
{
    bool res=usable::mechanics();
    if(this->getEnergy()<this->maxEnergy)
    {
        if (bElem::getCntr()%5==0)
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



