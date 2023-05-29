#include "puppetMasterFR.h"


videoElement::videoElementDef* puppetMasterFR::vd=nullptr;

videoElement::videoElementDef* puppetMasterFR::getVideoElementDef()
{
    return puppetMasterFR::vd;
}


puppetMasterFR::puppetMasterFR(std::shared_ptr<chamber> board): collectible(board), mechanical(board,false), killableElements(board),nonSteppable(board)
{
    //ctor
}

puppetMasterFR::~puppetMasterFR()
{
    //dtor
}
puppetMasterFR::puppetMasterFR():collectible(),mechanical(),killableElements(), nonSteppable()
{

}
void puppetMasterFR::setCollected(std::shared_ptr<bElem>who)
{
    collectible::setCollected(who);
    if (who.get()!=nullptr && who->getType()==_patrollingDrone) // if collected by the patrolling drone, activate the mechanics
        this->registerLiveElement(shared_from_this());
    else
        if (this->isLiveElement()) this->deregisterLiveElement(this->getInstanceid()); // if not, we deactivate the elements mechanics, if it is not already disactivated

}

bool puppetMasterFR::mechanics()
{
    bool res=killableElements::mechanics();
    std::shared_ptr<bElem> clc=this->getCollector();
    if(res && clc.get()!=nullptr && clc->getType()==_patrollingDrone && clc->getMoved()==0 )
    {
        switch(this->getSubtype()) // here we will route all the mechanics, when we are in the monster
        {
            case 0:
                return this->mechanicsPatrollingDrone();
                break;
            default:
                return false;
        }

    }
    return res;
}

bool puppetMasterFR::mechanicsPatrollingDrone()
{
    return false;

}


int puppetMasterFR::getType()
{
    return _puppetMasterType;
}
