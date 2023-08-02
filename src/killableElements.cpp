#include "killableElements.h"

killableElements::killableElements(std::shared_ptr<chamber> board) :bElem(board)
{
  //  this->setStats(std::make_shared<elemStats>(555));
}



killableElements::killableElements():bElem()
{
   // this->setStats(std::make_shared<elemStats>(555));
}


bool killableElements::additionalProvisioning(int subtype, std::shared_ptr<killableElements>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool killableElements::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool killableElements::additionalProvisioning(int subtype, int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}





bool killableElements::hurt(int points)
{
    if (this->attrs->isKillable()==false || this->status->isTeleporting() || this->status->isDying() || this->status->isDestroying())
    {
        return false;
    }

    this->attrs->setEnergy(this->attrs->getEnergy()-points);
    if (this->attrs->getEnergy()<=0)
        this->kill();
    return true;
}
