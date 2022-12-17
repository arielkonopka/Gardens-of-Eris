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
    plainMissile();
    plainMissile(std::shared_ptr<chamber> mychamber);
    plainMissile(std::shared_ptr<chamber> mychamber,int energy);
    void setStatsOwner(std::shared_ptr<bElem> owner);
    virtual ~plainMissile();
    virtual bool mechanics();
    virtual void stomp(std::shared_ptr<bElem> who);
    virtual bool setEnergy(int points);

private:
    std::shared_ptr<bElem> statsOwner;

};

#endif // PLAINMISSILE_H
