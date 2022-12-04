#ifndef PLAINMISSILE_H
#define PLAINMISSILE_H
#include "killableElements.h"
#include "videoElementDef.h"
#include <math.h>
class plainMissile : public killableElements, public movableElements, public mechanical
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    plainMissile(chamber *mychamber);
    plainMissile(chamber *mychamber,int energy);
    void setStatsOwner(bElem* owner);
    virtual ~plainMissile();
    virtual bool mechanics();
    virtual void stomp(bElem *who);
    virtual bool setEnergy(int points);

private:
    bElem* statsOwner;
    bool killing;

};

#endif // PLAINMISSILE_H
