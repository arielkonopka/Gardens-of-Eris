#include "bazookaMissile.h"

bazookaMissile::bazookaMissile(std::shared_ptr<chamber> board):explosives(board),movableElements(board)
{
    this->setBoard(board);
}

bazookaMissile::~bazookaMissile()
{
    //dtor
}

bool bazookaMissile::mechanics()
{
    if(!movableElements::mechanics()) return false;
    if(this->getElementInDirection(this->getStats()->getMyDirection())->getAttrs()->isSteppable())
      return this->moveInDirectionSpeed(this->getStats()->getMyDirection(),_bazookaMissileSpeed);
    return this->explode();
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
    this->registerLiveElement(shared_from_this());
    return movableElements::additionalProvisioning(subtype,typeId);;
}

bool bazookaMissile::additionalProvisioning(int subtype, std::shared_ptr<bazookaMissile>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}
