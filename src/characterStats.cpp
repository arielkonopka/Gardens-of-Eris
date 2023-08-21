#include "characterStats.h"
#include "bElem.h"

characterStats::characterStats(int typeId)
{
    this->strength=5+(bElem::randomNumberGenerator()%555);
    this->updateDexterity();
}

characterStats::~characterStats()
{

}

unsigned int characterStats::getStrength()
{
    return this->strength;
}

unsigned int characterStats::getGlobalPoints()
{
    return this->globalPoints;
}

unsigned int characterStats::getDexterity()
{
    return this->dexterity;
}
void characterStats::updateDexterity()
{
    this->dexterity=log2(this->_dPoints)+_initialDexterity;
}

void characterStats::countHit(std::shared_ptr<bElem> what)
{
    if(what==nullptr) return;

    if(what->getAttrs()->isKillable())
    {
        this->globalPoints++;
        if(what->getAttrs()->isCollectible())
        {
            this->_dPoints++;
            this->updateDexterity();
            return;
        }
        if(what->getAttrs()->isMovable())
        {
            this->_dPoints++;

            if(what->getStats()->hasActivatedMechanics())
            {
                this->_dPoints+=3;
            }
            if(what->getStats()->isDestroying())
            {
                this->_dPoints+=2;

            }
            this->updateDexterity();
            return;
        }
    }
}

void characterStats::countCollect(std::shared_ptr<bElem> what)
{
    this->globalPoints++;
    if(what->getType()==_goldenAppleType)
        this->globalPoints+=4;

}

void characterStats::countKill(std::shared_ptr<bElem> what)
{
    this->_dPoints+=5;
    this->globalPoints++;
    this->updateDexterity();
}
