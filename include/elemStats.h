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
protected:

private:
    void updateDexterity();
    unsigned int globalPoints;
    unsigned int _dPoints;
    unsigned int dexterity;
    unsigned int strength;
    unsigned int energy;
};

#endif // ELEMSTATS_H
