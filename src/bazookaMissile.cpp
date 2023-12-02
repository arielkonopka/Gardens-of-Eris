#include "bazookaMissile.h"

bazookaMissile::bazookaMissile(std::shared_ptr<chamber> board):explosives(board),movableElements(board)
{

}

bool bazookaMissile::mechanics()
{
    if(!movableElements::mechanics())
        return false;
    if(this->moveInDirectionSpeed(this->getStats()->getMyDirection(),_bazookaMissileSpeed))
    {
        this->steps++;
        return true;
    }
    if(steps>1) return this->explode();
    std::shared_ptr<bElem> be=this->getElementInDirection(this->getStats()->getMyDirection());
    if(be)
        be->hurt(this->getAttrs()->getEnergy());
    return this->kill();
}

int bazookaMissile::getType() const
{
    return _bazookaMissileType;
}

bool bazookaMissile::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool bazookaMissile::additionalProvisioning(int subtype, int typeId)
{
    return movableElements::additionalProvisioning(subtype,typeId);;
}

bool bazookaMissile::additionalProvisioning(int subtype, std::shared_ptr<bazookaMissile>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}
