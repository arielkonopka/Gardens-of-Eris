#ifndef PLAINGUN_H
#define PLAINGUN_H
//#include "commons.h"
#include "usable.h"
#include "videoElementDef.h"
#include "plainMissile.h"

class plainGun : public usable, public mechanical, public collectible, public nonSteppable
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    virtual bool use(std::shared_ptr<bElem> who);
    plainGun();
    explicit plainGun(std::shared_ptr<chamber> board);
    explicit plainGun(std::shared_ptr<chamber> board,int newSubtype);
    virtual bool mechanics();
    virtual ~plainGun();
    bool isWeapon();
    virtual bool readyToShoot();
    virtual int getAmmo();
    virtual void setAmmo(int ammo);
    virtual void setMaxEnergy(int me);
    virtual std::shared_ptr<bElem> createProjectible(std::shared_ptr<bElem> who);



private:
    int ammo=1+(bElem::randomNumberGenerator()%_plainGunAmmo);
    unsigned int shot=0;
   int maxEnergy=20*(1+(bElem::randomNumberGenerator()&3));

};

#endif // PLAINGUN_H
