#include "patrollingDrone.h"

videoElement::videoElementDef *patrollingDrone::vd = nullptr;

patrollingDrone::patrollingDrone(std::shared_ptr<chamber> board) : patrollingDrone()
{
    this->setBoard(board);
}
bool patrollingDrone::additionalProvisioning(int subtype,int typeId)
{
    bool res= bElem::additionalProvisioning(subtype,typeId);
    this->attrs->setCollect(true);
    this->attrs->setEnergy((1024*bElem::randomNumberGenerator())%155);
    return res;
}
bool patrollingDrone::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

patrollingDrone::patrollingDrone() : killableElements(), movableElements()
{

    //  this->attrs->setSubtype(0);
    // this->setInventory(std::make_shared<inventory>());

}

patrollingDrone::~patrollingDrone()
{
    this->brainModule=nullptr;
}
bool patrollingDrone::additionalProvisioning(int subtype, std::shared_ptr<patrollingDrone>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}


/*
    We will request brain on interaction
*/
bool patrollingDrone::interact(std::shared_ptr<bElem> who)
{
    bool res = movableElements::interact(who);
    if (!res)
        return res;
    if (res && !this->brained && this->attrs->getSubtype() == 0 && who->attrs->canCollect())
    {
        std::shared_ptr<bElem> token = who->attrs->getInventory()->requestToken(_puppetMasterType, -1);
        if (token)
        {
            this->playSound("Boot", "Success");
            this->brained = true;
            this->brainModule = token;
            token->status->setCollector(shared_from_this());
            token->status->setWaiting(55);
            return true;
        }
        this->playSound("Boot", "Failure");
    }
    return false;
}

int patrollingDrone::getType()
{
    return _patrollingDrone;
}

videoElement::videoElementDef *patrollingDrone::getVideoElementDef()
{
    return patrollingDrone::vd;
}
