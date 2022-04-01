#ifndef PLAINGUN_H
#define PLAINGUN_H
//#include "commons.h"
#include "usable.h"
#include "videoElementDef.h"
#include "plainMissile.h"

class plainGun : public usable
{
public:
        static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    virtual int getSubtype();
    virtual bool use(bElem *who);
    plainGun(chamber *board,gCollect *garbage);
    virtual bool mechanics(bool collected);
    virtual ~plainGun();
    bool isWeapon();
    virtual bool readyToShoot();
protected:

private:
    int ammo=-1;
    int shot=0;
    int energy=-1;
    int maxEnergy=-1;
};

#endif // PLAINGUN_H
