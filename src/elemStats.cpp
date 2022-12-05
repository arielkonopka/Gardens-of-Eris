#include "elemStats.h"
#include "bElem.h"

elemStats::elemStats(int energy): strength(energy),energy(energy)
{
    this->updateDexterity();
}

elemStats::~elemStats()
{

}
unsigned int elemStats::getEnergy()
{
    return this->energy;
}

void elemStats::setEnergy(int en)
{
    this->energy=(unsigned int)(en>0)?en:0; //if energy is negative, we set 0;
}

unsigned int elemStats::getStrength()
{
    return this->strength;
}

unsigned int elemStats::getGlobalPoints()
{
    return this->globalPoints;
}

unsigned int elemStats::getDexterity()
{
    return this->dexterity;
}
void elemStats::updateDexterity()
{
    this->dexterity=log2(this->_dPoints)+_initialDexterity;
}

void elemStats::countHit(bElem* what)
{
    if(what==nullptr) return;

    if(what->canBeKilled())
    {
        this->globalPoints++;
        if(what->isCollectible())
        {
            this->_dPoints++;
            this->updateDexterity();
            return;
        }
        if(what->isMovable())
        {
            this->_dPoints++;

            if(what->isLiveElement())
            {
                this->_dPoints+=3;
            }
            if(what->isDestroyed())
            {
                this->_dPoints+=2;

            }
            this->updateDexterity();
            return;
        }
    }
}

void elemStats::countCollect(bElem* what)
{
    this->globalPoints++;
    if(what->getType()==_goldenAppleType)
        this->globalPoints+=4;

}

void elemStats::countKill(bElem* what)
{
    this->_dPoints+=5;
    this->globalPoints++;
    this->updateDexterity();
}
