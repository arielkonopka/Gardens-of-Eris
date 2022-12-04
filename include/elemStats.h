#ifndef ELEMSTATS_H
#define ELEMSTATS_H
#include <math.h>
#include "commons.h"
class bElem;
class elemStats
{
public:
    elemStats(int energy);
    virtual ~elemStats();

    void countHit(bElem* what);
    void countCollect(bElem* what);
    void countKill(bElem* what);
    unsigned int getGlobalPoints();
    unsigned int getDexterity();
    unsigned int getEnergy();
    void setEnergy(int en);
    unsigned int getStrength();

private:
    void updateDexterity();
    unsigned int globalPoints=0;
    unsigned int _dPoints=1;
    unsigned int dexterity=1;
    unsigned int strength;
    unsigned int energy;
};

#endif // ELEMSTATS_H
