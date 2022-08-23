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
    virtual bool use(bElem *who);
    plainGun(chamber *board);
    plainGun(chamber *board,int newSubtype);
    virtual bool mechanics(bool collected);
    virtual ~plainGun();
    bool isWeapon();
    virtual bool readyToShoot();
    virtual int getAmmo();
    virtual void setAmmo(int ammo);
protected:

private:
    int ammo=-1;
    unsigned int shot=0;
    int energy=-1;
    int maxEnergy=-1;
};

#endif // PLAINGUN_H
