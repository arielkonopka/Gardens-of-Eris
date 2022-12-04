#include "plainGun.h"


videoElement::videoElementDef* plainGun::vd=nullptr;

videoElement::videoElementDef* plainGun::getVideoElementDef()
{
    return plainGun::vd;
}
//public usable, public mechanical, public collectible, public nonSteppable
plainGun::plainGun(chamber *board): usable(board), mechanical(board), collectible(board), nonSteppable(board),ammo((1+(bElem::randomNumberGenerator()%_plainGunAmmo))*2),maxEnergy(20*(1+(bElem::randomNumberGenerator()&3)))
{
    this->setStats(new elemStats(((bElem::randomNumberGenerator()%4)+1)*25));
}

plainGun::plainGun(chamber* board, int newSubtype): usable(board), mechanical(board), collectible(board), nonSteppable(board),ammo((1+(bElem::randomNumberGenerator()%_plainGunAmmo))*2),maxEnergy(20*(1+(bElem::randomNumberGenerator()&3)))
{
    this->setStats(new elemStats(((bElem::randomNumberGenerator()%4)+1)*25));
    this->setSubtype(newSubtype);
}

bElem* plainGun::createProjectible(bElem *who)
{
    plainMissile* pm=new plainMissile(who->getBoard());
    pm->setStatsOwner(who);
    who->lockThisObject(pm);
    pm->setDirection(who->getDirection());
    pm->stepOnElement(who->getElementInDirection(who->getDirection()));
    pm->setEnergy(this->getEnergy());
    return pm;
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
    if (who==nullptr)
    {
#ifdef _VerbousMode_
        std::cout<<"Who is nullptr for plain gun!";
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
    if(myel!=nullptr)
    {
        if (this->ammo>0 || this->getSubtype()%2==1)
        {
            if (myel->isSteppable()==true)
            {
                this->createProjectible(who);
            }
           /* else if ( myel->getType()==_plainMissile && myel->getDirection()==who->getDirection())
            {
                myel->setEnergy(myel->getEnergy()+this->getEnergy()); // if somehow you hit a missile going in the same direction, the show would be boosted;
            }*/
            else if (myel->canBeKilled() )
            {
                if(who->getStats()!=nullptr)
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
void plainGun::setMaxEnergy(int me)
{
    this->maxEnergy=me;
    this->setEnergy(me);
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



