#ifndef PLAINMISSILE_H
#define PLAINMISSILE_H
#include "movableElements.h"
#include "videoElementDef.h"

class plainMissile : public movableElements
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    virtual int getSubtype();
    plainMissile(chamber *mychamber,gCollect *garbage);
    plainMissile(chamber *mychamber,gCollect *garbage,int energy);
    virtual ~plainMissile();
    virtual bool mechanics(bool collected);
    virtual bool canBeKilled();

protected:


    bool killing;
private:
};

#endif // PLAINMISSILE_H
