#include "patrollingDrone.h"

videoElement::videoElementDef *patrollingDrone::vd = nullptr;

patrollingDrone::patrollingDrone(std::shared_ptr<chamber> board) : killableElements(board), nonSteppable(board), movableElements(board)
{
    this->setSubtype(0);
    this->setInventory(std::make_shared<inventory>());
    this->setEnergy((1024*bElem::randomNumberGenerator())%155);
}

patrollingDrone::patrollingDrone() : killableElements(), nonSteppable(), movableElements()
{
    this->setEnergy((1024*bElem::randomNumberGenerator())%155);
    this->setSubtype(0);
    this->setInventory(std::make_shared<inventory>());

}

patrollingDrone::~patrollingDrone()
{
    this->brainModule=nullptr;
}

bool patrollingDrone::canBeKilled()
{
    return true;
}

/*
    We will request brain on interaction
*/
bool patrollingDrone::interact(std::shared_ptr<bElem> who)
{
    bool res = movableElements::interact(who);
    if (!res)
        return res;
    if (res && !this->brained && this->getSubtype() == 0 && who->getInventory().get() != nullptr)
    {
        std::shared_ptr<bElem> token = who->getInventory()->requestToken(_puppetMasterType, -1);
        if (token)
        {
            this->playSound("Boot", "Success");
            this->brained = true;
            this->brainModule = token;
            token->setCollected(shared_from_this());
            token->setWait(55);
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
