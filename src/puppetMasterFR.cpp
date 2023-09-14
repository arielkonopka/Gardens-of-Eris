#include "puppetMasterFR.h"



puppetMasterFR::puppetMasterFR(std::shared_ptr<chamber> board) :  puppetMasterFR()
{
    this->setBoard(board);
}

puppetMasterFR::~puppetMasterFR()
{
    // dtor
}
puppetMasterFR::puppetMasterFR() :  mechanical(), killableElements()
{
}


bool puppetMasterFR::collectOnAction(bool c, std::shared_ptr<bElem>who)
{
    bool r=bElem::collectOnAction(c,who);
    if(c && r && who )
    {
        if(who->getType() == _patrollingDrone)
        {
            if (this->getAttrs()->getSubtype() == 0) // if subtype not set, set one randomly
            {
                this->getAttrs()->setSubtype(2);
                this->getAttrs()->setSubtype(this->randomNumberGenerator()%2);
            }
            this->registerLiveElement(shared_from_this());
        }
    }
    else if(this->getStats()->hasActivatedMechanics())
        this->deregisterLiveElement(this->getStats()->getInstanceId());

    return true;
}


bool puppetMasterFR::mechanics()
{
    bool res = mechanical::mechanics();

    std::shared_ptr<bElem> clc = this->getStats()->getCollector().lock();
    if (res && clc.get() != nullptr && clc->getType() == _patrollingDrone && !clc->getStats()->isMoving() && !clc->getStats()->isWaiting())
    {
        switch (this->getAttrs()->getSubtype()) // here we will route all the mechanics, when we are in the monster
        {
        case 0:
            return this->mechanicsPatrollingDrone();
        case 1:
            if(!this->collectorMechanics())
                return this->mechanicsPatrollingDrone();
            return true;
        default:
            return false;
        }
    }
    return res;
}

bool puppetMasterFR::collectorMechanics()
{
    std::shared_ptr<bElem> _collector=this->getStats()->getCollector().lock();
    for(int c=0; c<4; c++)
    {
        direction d=_collector->getStats()->getMyDirection();
        d=(direction)(((int)d+c)%4);
        std::shared_ptr<bElem> check=this->findObjectInDirection(d);

        if(check && check->getAttrs()->isCollectible())
        {
            if(_collector->getStats()->getMyDirection()!=d)
            {
                _collector->getStats()->setMyDirection(d);
                _collector->getStats()->setFacing(d);
                this->getStats()->setWaiting(5);
                return true;

            }
            return _collector->moveInDirection(d);
        }
    }

    return false;
}

std::shared_ptr<bElem> puppetMasterFR::findObjectInDirection(direction dir)
{
    std::shared_ptr<bElem> b = this->getStats()->getCollector().lock();
    b=b->getElementInDirection(dir);
    while(b!=nullptr && b->getAttrs()->isSteppable())
    {
        b=b->getElementInDirection(dir);
    }
    return b;
}

bool puppetMasterFR::mechanicsPatrollingDrone()
{
    std::shared_ptr<bElem> collector = this->getStats()->getCollector().lock();
    direction cdir = collector->getStats()->getMyDirection();
    direction pdir1 = (direction)((((int)cdir) + 1) % 4);
    direction pdir2 = (direction)((((int)cdir) + 3) % 4);
    bool b1 = false, b2 = false;
    if (collector->getElementInDirection(pdir1))
        b1 = collector->getElementInDirection(pdir1)->getAttrs()->isSteppable();
    if (collector->getElementInDirection(pdir2))
        b2 = collector->getElementInDirection(pdir2)->getAttrs()->isSteppable();
    int roulette=this->randomNumberGenerator() %55;
    if (b1 && roulette == 5) // same probablility for each
    {
        collector->getStats()->setMyDirection(pdir1);
        collector->getStats()->setWaiting(3);
        return true;
    }
    else if (b2 && roulette==25)
    {
        collector->getStats()->setMyDirection(pdir1);
        collector->getStats()->setWaiting(3);
        return true;
    }
    bool r = collector->moveInDirection(cdir);
    if (r == false)
    {
        int f = (this->randomNumberGenerator() % 2 == 0) ? 1 : 3;
        cdir = (direction)((((int)cdir) + f) % 4);
        collector->getStats()->setMyDirection(cdir);
        collector->getStats()->setWaiting(3);
        return true;
    }
    return r;
}

int puppetMasterFR::getType() const
{
    return _puppetMasterType;
}


bool puppetMasterFR::additionalProvisioning(int subtype, std::shared_ptr<puppetMasterFR>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool puppetMasterFR::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool puppetMasterFR::additionalProvisioning(int subtype,int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}










