#include "simpleBomb.h"


simpleBomb::simpleBomb(std::shared_ptr<chamber>board):explosives(board),killableElements(board)
{

}

bool simpleBomb::additionalProvisioning(int subtype, std::shared_ptr<simpleBomb>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}
float simpleBomb::getViewRadius() const
{
    return 4.5;
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
    if(this->getStats()->isDestroying() || this->triggered)
        return false;

    this->registerLiveElement(shared_from_this());
    this->triggered = true;
    this->getStats()->setWaiting(15); /* magic number */
    return true;
}

bool simpleBomb::mechanics()
{
    if (bElem::mechanics()) return this->explode(1.5);
    return false;
}



int simpleBomb::getType() const
{
    return _simpleBombType;
}
