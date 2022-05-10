#include "plainGun.h"


videoElement::videoElementDef* plainGun::vd=NULL;

videoElement::videoElementDef* plainGun::getVideoElementDef()
{
    return plainGun::vd;
}

plainGun::plainGun(chamber *board): usable(board)
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
    if(who->getType()==_player)
    {
        std::cout<<"Shoot: "<<this->readyToShoot()<<" s "<<this->shot<<"\n";
    }

    if (this->readyToShoot()==false)
        return true; //The gun is fine, not ready to shoot though
    this->shot=_mov_delay+_plainMissileSpeed;
    if (this->ammo<=0 || who==NULL) //odd subtypes have infinite shots
        if (this->getSubtype()%2)
            return false;
 //   if(who->getType()==_player)
 //       std::cout<<"energy: "<<this->getEnergy()<<"\n";
    myel=who->getElementInDirection(who->getDirection());
    if(myel!=NULL)
    {

        int ener=this->getEnergy();;
        if (who->getStats().dexterity>0)
        {
            ener= this->getEnergy()-this->randomNumberGenerator()%(_dexterityLevels-who->getStats().dexterity);
        }
        if (this->ammo>0)
            if (this->getSubtype()%2==0)
                this->ammo--;
        this->setEnergy(this->getEnergy()/2);

        if (myel->isSteppable()==true)
        {
            bElem* missile=new plainMissile(this->getBoard(),ener);
            missile->setEnergy(ener);
            missile->stepOnElement(myel);
            missile->setDirection(who->getDirection());
          //  missile->setMoved(_plainMissileSpeed);
            return true;

        }
        else if ( myel->getType()==_plainMissile && myel->getDirection()==who->getDirection())
        {
            myel->setEnergy(myel->getEnergy()+ener);

        }
        else if (myel->canBeKilled() )
        {
            myel->hurt(this->getEnergy());

            // this->disposeElement();
        }
    }

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
     //   std::cout<<"mechanics gun\n";
        this->shot--;
    }
    if(this->getEnergy()<this->maxEnergy)
    {
        if (bElem::getCntr()%3==0)
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



