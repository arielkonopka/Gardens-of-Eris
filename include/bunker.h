#ifndef BUNKER_H
#define BUNKER_H
#include "commons.h"
#include "elementFactory.h"
#include "plainGun.h"
#include "bElem.h"
//#include "soundManager.h"


class bunker : public mechanical,public movableElements
{
public:

    bunker();
    explicit bunker(std::shared_ptr<chamber> board);
    virtual ~bunker();
    virtual bool mechanics();
    direction findLongestShot();
    virtual bool selfAlign();
    int getType() const;
    bool interact(std::shared_ptr<bElem> Who) final;
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<bunker> sbe);
private:
    std::shared_ptr<characterStats> backUp=nullptr;
    std::shared_ptr<bElem> activatedBy=nullptr;
    int help=0;
    std::shared_ptr<bElem> myGun;
    int sndHummHandle=-1;
};

#endif // BUNKER_H
