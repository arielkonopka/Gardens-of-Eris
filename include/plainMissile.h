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
    explicit plainMissile(std::shared_ptr<chamber> mychamber);
    explicit plainMissile(std::shared_ptr<chamber> mychamber,int energy);
    void setStatsOwner(std::shared_ptr<bElem> owner);
    virtual ~plainMissile();
    virtual bool mechanics();
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<plainMissile> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);

private:
    std::shared_ptr<bElem> statsOwner;

};

#endif // PLAINMISSILE_H
