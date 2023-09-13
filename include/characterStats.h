#ifndef ELEMSTATS_H
#define ELEMSTATS_H
#include <math.h>
#include "commons.h"
class bElem;
class characterStats : public std::enable_shared_from_this<characterStats>
{
public:
    characterStats(int typeId);
    virtual ~characterStats();

    void countHit(std::shared_ptr<bElem> what);
    void countCollect(std::shared_ptr<bElem> what);
    void countKill(std::shared_ptr<bElem> what);
    unsigned int getGlobalPoints();
    unsigned int getDexterity();
    unsigned int getStrength();

private:
    void updateDexterity();
    unsigned int globalPoints=0;
    unsigned int _dPoints=1;
    unsigned int dexterity=1;
    unsigned int strength;
//    unsigned int energy;
};

#endif // ELEMSTATS_H
