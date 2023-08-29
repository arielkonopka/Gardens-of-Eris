#include "plainGun.h"


videoElement::videoElementDef* plainGun::vd=nullptr;

videoElement::videoElementDef* plainGun::getVideoElementDef()
{
    return plainGun::vd;
}
//public usable, public mechanical, public collectible, public nonSteppable
plainGun::plainGun(std::shared_ptr<chamber> board): plainGun()
{
    this->setBoard(board);
}

plainGun::plainGun(std::shared_ptr<chamber>  board, int newSubtype): plainGun(board)
{
}

std::shared_ptr<bElem> plainGun::createProjectible(std::shared_ptr<bElem> who)
{
    std::shared_ptr<bElem> pm=elementFactory::generateAnElement<plainMissile>(who->getBoard(),0);
    pm->getStats()->setStatsOwner(who);
    who->lockThisObject(pm);
    pm->getStats()->setMyDirection(who->getStats()->getFacing());
    pm->getStats()->setFacing(who->getStats()->getFacing());
    pm->stepOnElement(who->getElementInDirection(who->getStats()->getFacing()));
    pm->getAttrs()->setEnergy(this->getAttrs()->getEnergy());
    return pm;
}

plainGun::plainGun():mechanical()
{
    //this->setStats(std::make_shared<elemStats>(((bElem::randomNumberGenerator()%4)+1)*25));

}
bool plainGun::additionalProvisioning(int subtype, std::shared_ptr<plainGun>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool plainGun::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool plainGun::additionalProvisioning(int subtype,int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}


int plainGun::getType() const
{
    return _plainGun;
}

plainGun::~plainGun()
{

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
  ma
#endif
   if (this->getStats()->isWaiting())
        return false;
    this->getStats()->setWaiting(_plainGunCharge);
    if (this->getAttrs()->getAmmo()<=0 && (this->getAttrs()->getSubtype()%2)==0) //odd subtypes have infinite shots
        return false;
    myel=who->getElementInDirection(who->getStats()->getFacing());
    if(myel!=nullptr)
    {
        if (this->getAttrs()->getAmmo()>0 || this->getAttrs()->getSubtype()%2==1)
        {

            coords3d c3d;
            c3d.x=who->getStats()->getMyPosition().x*32+who->getOffset().x;
            c3d.z=who->getStats()->getMyPosition().y*32+who->getOffset().y;
            c3d.y=50;
            coords3d vel= {who->getOffset().x,0,who->getOffset().y};
            soundManager::getInstance()->registerSound(who->getBoard()->getInstanceId(),c3d,vel,this->getStats()->getInstanceId(),this->getType(),this->getAttrs()->getSubtype(),"Shoot","Shoot");

            if (myel->getAttrs()->isSteppable()==true)
            {
                this->createProjectible(who);
            }
            else if (myel->getAttrs()->isKillable() )
            {
                /*   if(who->getStats()!=nullptr)
                       who->getStats()->countKill(myel);
                       */
                myel->hurt(this->getAttrs()->getEnergy());

                // this->disposeElement();
            }
            if (this->getAttrs()->getSubtype()%2==0)
            {
                this->getAttrs()->setAmmo(this->getAttrs()->getAmmo()-1);
                this->getAttrs()->setEnergy(this->getAttrs()->getEnergy()-(this->getAttrs()->getEnergy()*0.2));

            }
        }

    }
    return true;
}
/*
void plainGun::setMaxEnergy(int me)
{
    this->maxEnergy=me;
    this->getAttrs()->setEnergy(me);
}

*/



bool plainGun::mechanics()
{
    bool res=mechanical::mechanics();
    if(this->getAttrs()->getEnergy()<this->maxEnergy)
    {
        if (bElem::getCntr()%5==0)
            this->getAttrs()->setEnergy(this->getAttrs()->getEnergy()+1);
    }
    return res;
}
/*
int plainGun::getAmmo()
{
    return this->ammo;
}

void plainGun::setAmmo(int ammo)
{
    this->ammo=ammo;
}

*/

