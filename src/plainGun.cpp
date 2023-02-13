#include "plainGun.h"


videoElement::videoElementDef* plainGun::vd=nullptr;

videoElement::videoElementDef* plainGun::getVideoElementDef()
{
    return plainGun::vd;
}
//public usable, public mechanical, public collectible, public nonSteppable
plainGun::plainGun(std::shared_ptr<chamber> board): usable(board), mechanical(board), collectible(board), nonSteppable(board)
{
    this->setStats(std::make_shared<elemStats>(((bElem::randomNumberGenerator()%4)+1)*25));
}

plainGun::plainGun(std::shared_ptr<chamber>  board, int newSubtype): usable(board), mechanical(board), collectible(board), nonSteppable(board)
{
    this->setStats(std::make_shared<elemStats>(((bElem::randomNumberGenerator()%4)+1)*25));
    this->setSubtype(newSubtype);
}

std::shared_ptr<bElem> plainGun::createProjectible(std::shared_ptr<bElem> who)
{
    std::shared_ptr<bElem> pm=elementFactory::generateAnElement<plainMissile>(who->getBoard());
    pm->setStatsOwner(who);
    who->lockThisObject(pm);
    pm->setDirection(who->getFacing());
    pm->setFacing(who->getFacing());
    pm->stepOnElement(who->getElementInDirection(who->getFacing()));
    pm->setEnergy(this->getEnergy());
    return pm;
}

plainGun::plainGun():usable(),mechanical(),collectible(),nonSteppable()
{

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
bool plainGun::use(std::shared_ptr<bElem> who)
{
    std::shared_ptr<bElem> myel;
#ifdef _VerbousMode_
    if (who==nullptr)
    {

        std::cout<<"Who is nullptr for plain gun!";
        return false;

    }
    if(who->getType()==_player)
    {

        std::cout<<"Shoot: "<<this->readyToShoot()<<" s "<<this->shot<<" Energy "<<this->getEnergy()<<"\n";

    }
#endif
    if (this->readyToShoot()==false)
        return false; //The gun is fine, not ready to shoot though
    this->shot=this->getCntr()+_plainGunCharge;
    if (this->ammo<=0 && (this->getSubtype()%2)==0) //odd subtypes have infinite shots
        return false;
    myel=who->getElementInDirection(who->getFacing());
    if(myel!=nullptr)
    {
        if (this->ammo>0 || this->getSubtype()%2==1)
        {

            coords3d c3d;
            c3d.x=who->getCoords().x*32+who->getOffset().x;
            c3d.z=who->getCoords().y*32+who->getOffset().y;
            c3d.y=50;
            coords3d vel= {who->getOffset().x,0,who->getOffset().y};
            soundManager::getInstance()->registerSound(who->getBoard()->getInstanceId(),c3d,vel,this->getInstanceid(),this->getType(),this->getSubtype(),"Shoot","Shoot");

            if (myel->isSteppable()==true)
            {
                this->createProjectible(who);
            }
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



