#include "simpleBomb.h"

videoElement::videoElementDef *simpleBomb::vd = nullptr;

simpleBomb::simpleBomb(std::shared_ptr<chamber>board):explosives(board),killableElements(board),movableElements(board)
{

}

bool simpleBomb::additionalProvisioning(int subtype, std::shared_ptr<simpleBomb>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool simpleBomb::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool simpleBomb::additionalProvisioning(int subtype,int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}

bool simpleBomb::hurt(int points)
{
    return this->destroy();
}

bool simpleBomb::kill()
{
    return this->destroy();
}

bool simpleBomb::destroy()
{
    bElem::destroy();
    if (this->triggered)
    {
        return false;
    }
    this->registerLiveElement(shared_from_this());
    this->triggered = true;
    this->getStats()->setWaiting(5); /* magic number */
    return true;
}

bool simpleBomb::mechanics()
{
    bElem::mechanics();
    if (this->getStats()->getWaiting() > 1)
        return false;
    this->explode(1.5);
    return true;
}

videoElement::videoElementDef *simpleBomb::getVideoElementDef()
{
    return simpleBomb::vd;
}

int simpleBomb::getType() const
{
    return _simpleBombType;
}
