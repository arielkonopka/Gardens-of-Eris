#include "patrollingDrone.h"


patrollingDrone::patrollingDrone(std::shared_ptr<chamber> board) : patrollingDrone()
{
    this->setBoard(board);
}
bool patrollingDrone::additionalProvisioning(int subtype,int typeId)
{
    bool res= bElem::additionalProvisioning(subtype,typeId);
    this->getAttrs()->setCollect(true);
    this->getAttrs()->setEnergy((1024*bElem::randomNumberGenerator())%155);
    return res;
}
bool patrollingDrone::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

patrollingDrone::patrollingDrone() : killableElements(), movableElements()
{

    //  this->getAttrs()->setSubtype(0);
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
float patrollingDrone::getViewRadius() const
{
    return 5.0;
}


/*
    We will request brain on interaction
*/
bool patrollingDrone::interact(std::shared_ptr<bElem> who)
{
    bool res = bElem::interact(who);
    if (res && !this->brained && this->getAttrs()->getSubtype() == 0 && who->getAttrs()->canCollect())
    {
        std::shared_ptr<bElem> token = who->getAttrs()->getInventory()->requestToken(_puppetMasterType, -1);
        if (token)
        {
            this->playSound("Boot", "Success");
            this->brained = true;
            this->brainModule = token;
            token->getStats()->setCollector(shared_from_this());
            token->collectOnAction(true,shared_from_this()); // since we collect the object ourselves, we should also trigger the action
            token->getStats()->setWaiting(55);
            if(who->getType()==_player)
                viewPoint::get_instance()->setOwner(shared_from_this());
            return true;
        }
        this->playSound("Boot", "Failure");
    }
    return false;
}

int patrollingDrone::getType() const
{
    return _patrollingDrone;
}


