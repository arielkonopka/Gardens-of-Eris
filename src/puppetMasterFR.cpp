#include "puppetMasterFR.h"

videoElement::videoElementDef *puppetMasterFR::vd = nullptr;

videoElement::videoElementDef *puppetMasterFR::getVideoElementDef()
{
    return puppetMasterFR::vd;
}

puppetMasterFR::puppetMasterFR(std::shared_ptr<chamber> board) : collectible(board), mechanical(board, false), killableElements(board), nonSteppable(board)
{
    // ctor
}

puppetMasterFR::~puppetMasterFR()
{
    // dtor
}
puppetMasterFR::puppetMasterFR() : collectible(), mechanical(), killableElements(), nonSteppable()
{
}
/*
void puppetMasterFR::setCollected(std::shared_ptr<bElem> who)
{
    collectible::setCollected(who);
    if (who.get() != nullptr)
    {
        if (who->getType() == _patrollingDrone) // if collected by the patrolling drone, activate the mechanics
        {
            if (this->attrs->getSubtype() == 0) // if subtype not set, set one randomly
            {
                this->attrs->setSubtype(2);
                this->attrs->setSubtype(this->randomNumberGenerator()%2);
            }
            this->registerLiveElement(shared_from_this());
        }
        else if (who->getType() == _player)
            this->playSound("Collect", "Player");
    }
    else if (this->isLiveElement())
        this->deregisterLiveElement(this->status->getInstanceId()); // if not, we deactivate the elements mechanics, if it is not already disactivated
}
*/
bool puppetMasterFR::mechanics()
{
    bool res = mechanical::mechanics();

    std::shared_ptr<bElem> clc = this->status->getCollector().lock();
    if (res && clc.get() != nullptr && clc->getType() == _patrollingDrone && clc->status->getMoved() == 0 && !clc->status->isWaiting())
    {
        switch (this->attrs->getSubtype()) // here we will route all the mechanics, when we are in the monster
        {
        case 0:
            return this->mechanicsPatrollingDrone();
            break;
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
    std::shared_ptr<bElem> _collector=this->status->getCollector().lock();
    for(int c=0;c<4;c++)
    {
        direction d=this->status->getCollector().lock()->status->getMyDirection();
        d=(direction)(((int)d+c)%4);
        std::shared_ptr<bElem> check=this->findObjectInDirection(d);

        if(check && check->attrs->isCollectible())
        {
            if(_collector->status->getMyDirection()!=d)
            {
                _collector->status->setMyDirection(d);
                _collector->status->setFacing(d);
                this->status->setWaiting(5);
                return true;

            }
            return _collector->moveInDirection(d);
        }
    }

return false;
}

std::shared_ptr<bElem> puppetMasterFR::findObjectInDirection(direction dir)
{
    std::shared_ptr<bElem> b = this->status->getCollector().lock();
    b=b->getElementInDirection(dir);
    while(b!=nullptr && b->attrs->isSteppable())
    {
        b=b->getElementInDirection(dir);
    }
    return b;
}

bool puppetMasterFR::mechanicsPatrollingDrone()
{
    std::shared_ptr<bElem> collector = this->status->getCollector().lock();
    direction cdir = collector->status->getMyDirection();
    direction pdir1 = (direction)((((int)cdir) + 1) % 4);
    direction pdir2 = (direction)((((int)cdir) + 3) % 4);
    bool b1 = false, b2 = false;
    if (collector->getElementInDirection(pdir1))
        b1 = collector->getElementInDirection(pdir1)->attrs->isSteppable();
    if (collector->getElementInDirection(pdir2))
        b2 = collector->getElementInDirection(pdir2)->attrs->isSteppable();
    int roulette=this->randomNumberGenerator() %55;
    if (b1 && roulette == 5) // same probablility for each
    {
        collector->status->setMyDirection(pdir1);
        collector->status->setWaiting(3);
        return true;
    }
    else if (b2 && roulette==25)
        {
            collector->status->setMyDirection(pdir1);
            collector->status->setWaiting(3);
            return true;
        }
    bool r = collector->moveInDirection(cdir);
    if (r == false)
    {
        int f = (this->randomNumberGenerator() % 2 == 0) ? 1 : 3;
        cdir = (direction)((((int)cdir) + f) % 4);
        collector->status->setMyDirection(cdir);
        collector->status->setWaiting(3);
        return true;
    }
    return r;
}

int puppetMasterFR::getType()
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










