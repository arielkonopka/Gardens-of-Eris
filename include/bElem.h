#ifndef BELEM_H
#define BELEM_H
#include <chrono>
#include <random>
#include <memory>
#include <array>
#include <algorithm>
#include "commons.h"
#include "objectTypes.h"
#include "inventory.h"
#include "chamber.h"
#include "videoElementDef.h"
#include "elemStats.h"
#include "soundManager.h"
#include "bElemStats.h"
#include "bElemAttr.h"
namespace videoElement
{
class videoElementDef;
}
class chamber;
class nonSteppable;

using oState = enum ost { DISPOSED = 0,
                          nullptrREACHED = 1,
                          ERROR = 2
                        };
/*struct _cfg
{
    bool amIUsable = false;
    int subtype = 0;
    std::shared_ptr<elemStats> myStats = nullptr;
    std::shared_ptr<inventory> myInventory = nullptr;
    int instance;
    bool provisioned = false;
};
*/

class bElem : public virtual std::enable_shared_from_this<bElem>
{
public:
    /*
     * these three templates below generate objects. I added them because I need bidirectional connection with everything
     */

    static videoElement::videoElementDef *vd;
    std::unique_ptr<bElemStats> status;
    std::unique_ptr<bElemAttr> attrs;
    virtual ALLEGRO_MUTEX *getMyMutex();
    void registerLiveElement(std::shared_ptr<bElem> who);
    void deregisterLiveElement(int instanceId);
    static void runLiveElements();

    virtual sNeighboorhood getSteppableNeighboorhood();
    virtual ~bElem();
    virtual videoElement::videoElementDef *getVideoElementDef();
    virtual void setBoard(std::shared_ptr<chamber> board);
    virtual std::shared_ptr<chamber> getBoard();
    virtual constexpr bool selfAlign()
    {
        return false;
    };
    virtual bool stepOnElement(std::shared_ptr<bElem> step);
    // virtual bool isLiveElement();

    // virtual void setDropped();                             // notify it got dropped

    virtual bool moveInDirection(direction d);
    virtual bool moveInDirectionSpeed(direction d, int speed);
    virtual bool use(std::shared_ptr<bElem> use);

    virtual bool interact(std::shared_ptr<bElem> who);
    virtual bool destroy();
    virtual bool kill();
    virtual constexpr bool hurt(int points)
    {
        return false;
    };
    virtual bool isSteppableDirection(direction di);
    virtual std::shared_ptr<bElem> getElementInDirection(direction di);
    virtual coords getAbsCoords(direction dir);
    virtual  int getType() const;

    virtual int getAnimPh();



    virtual const float getViewRadius()
    {
        return 0.0;
    };
    virtual bool collect(std::shared_ptr<bElem> collectible);
    //virtual bool isSwitchOn();

    std::shared_ptr<elemStats> getStats();
    void setStats(std::shared_ptr<elemStats> stat); // warning, unsafe method


    virtual std::shared_ptr<bElem> removeElement(); // removes element from the board, and returns it for further processing, usefull for eg. for collecting stuff
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual coords getOffset();

    virtual constexpr bool readyToShoot()
    {
        return false;
    };

    static std::mt19937 randomNumberGenerator;
    static bool randomNumberGeneratorInitialized;
    virtual bool mechanics();
    static std::vector<std::shared_ptr<bElem>> liveElems;

    static void tick();
    static unsigned int getCntr();


    virtual bool isLocked();
    virtual bool lockThisObject(std::shared_ptr<bElem> who);
    virtual bool unlockThisObject(std::shared_ptr<bElem> who);

    virtual int getTypeInDirection(direction di);
    virtual void setStatsOwner(std::shared_ptr<bElem> owner);
    bElem();
    explicit bElem(std::shared_ptr<chamber> board);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<bElem> sbe);

private:
    bool provisioned=false;
    bElem(const bElem &) = delete;
    std::shared_ptr<chamber> attachedBoard = nullptr;
    ALLEGRO_MUTEX *elementMutex = nullptr;
    static int instances;
    static unsigned int sTaterCounter;
    //  struct _cfg eConfig;
    //  struct _eStatus state;
    std::vector<std::shared_ptr<bElem>> lockers;
    static std::vector<int> toDeregister;
};

#endif // BELEM_H
