#ifndef PLAINGUN_H
#define PLAINGUN_H
//#include "commons.h"
#include "videoElementDef.h"
#include "plainMissile.h"
#include "soundManager.h"
//#include "viewPoint.h"
class plainGun : public bElem
{
public:

    virtual int getType() const;
    virtual bool use(std::shared_ptr<bElem> who);
    plainGun();
    explicit plainGun(std::shared_ptr<chamber> board);
    explicit plainGun(std::shared_ptr<chamber> board,int newSubtype);
    virtual bool mechanics();
    virtual ~plainGun() =default;
  //  virtual bool readyToShoot();

    virtual std::shared_ptr<bElem> createProjectible(std::shared_ptr<bElem> who);

    virtual bool additionalProvisioning(int subtype,std::shared_ptr<plainGun> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);

private:
    int ammo=1+(bElem::randomNumberGenerator()%_plainGunAmmo);
    unsigned int shot=0;
    int maxEnergy=20*(1+(bElem::randomNumberGenerator()&3));

};

#endif // PLAINGUN_H
