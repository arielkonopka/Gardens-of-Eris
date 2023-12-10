#include "bazookaMissile.h"

bazookaMissile::bazookaMissile(std::shared_ptr<chamber> board):explosives(board),movableElements(board)
{

}

bool bazookaMissile::mechanics()
{
    if(!movableElements::mechanics())
        return false;
    if(++this->steps<_bazookaMaxSteps && this->moveInDirectionSpeed(this->getStats()->getMyDirection(),_bazookaMissileSpeed) )
        return true;
    if(this->steps>1)
        return this->explode(1.5);
    std::shared_ptr<bElem> be=this->getElementInDirection(this->getStats()->getMyDirection());
    int beEnergy=be->getAttrs()->getEnergy();

    if(be)
        be->hurt(this->getAttrs()->getEnergy());
    beEnergy=beEnergy-be->getAttrs()->getEnergy();
    if(beEnergy)
    {
        std::shared_ptr<bElem> sowner=this->getStats()->getStatsOwner().lock();
        if(sowner)
        {
            sowner->getStats()->setPoints(SHOOT,sowner->getStats()->getPoints(SHOOT)+1);
            if(beEnergy!=0)
                sowner->getStats()->setPoints(TOTAL,sowner->getStats()->getPoints(TOTAL)+beEnergy);
        }
    }

    return bElem::kill();
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