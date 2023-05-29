#include "patrollingDrone.h"

videoElement::videoElementDef* patrollingDrone::vd=nullptr;

patrollingDrone::patrollingDrone(std::shared_ptr<chamber> board): killableElements(board), nonSteppable(board),  movableElements(board)
{
    this->setSubtype(0);
    this->setInventory(std::make_shared<inventory>());
 
}

patrollingDrone::patrollingDrone() : killableElements(),nonSteppable(),movableElements()
{

}


patrollingDrone::~patrollingDrone()
{

}


/*
    We will request brain on interaction
*/
bool patrollingDrone::interact(std::shared_ptr<bElem> who)
{
    bool res=killableElements::interact(who);
    if(res && !this->brained && this->getSubtype()==0 && who->getInventory().get()!=nullptr)
    {
        std::shared_ptr<bElem> token=who->getInventory()->requestToken(_puppetMasterType,-1);
        if(token.get()!=nullptr)
        {
            this->brained=true;
            this->brainModule=token;
            token->setCollected(shared_from_this());
            return true;
        }
    }
    return false;

}



int patrollingDrone::getType()
{
    return _patrollingDrone;
}

videoElement::videoElementDef* patrollingDrone::getVideoElementDef()
{
    return patrollingDrone::vd;
}
