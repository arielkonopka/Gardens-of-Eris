#ifndef BUNKER_H
#define BUNKER_H
#include "commons.h"
#include "elementFactory.h"
#include "plainGun.h"
#include "bElem.h"


class bunker : public movableElements
{
public:

    bunker();
    explicit bunker(std::shared_ptr<chamber> board);
    virtual ~bunker() = default;
    bool mechanics() final;
    direction findLongestShot();
    virtual bool selfAlign();
    int getType() const;
    bool interact(std::shared_ptr<bElem> Who) final;
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<bunker> sbe);
private:
    std::shared_ptr<bElem> activatedBy=nullptr;
    int help=0;
    std::shared_ptr<bElem> myGun;
    int sndHummHandle=-1;
    const int brange=10;
};

#endif // BUNKER_H
