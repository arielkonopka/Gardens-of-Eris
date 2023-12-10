#ifndef BELEM_H
#define BELEM_H
#include <chrono>
#include <random>
#include <memory>
#include <mutex>
#include <array>
#include <algorithm>
#include "commons.h"
#include "objectTypes.h"
#include "inventory.h"
#include "chamber.h"
#include "videoElementDef.h"
//
#include "soundManager.h"
#include "bElemStats.h"
#include "bElemAttr.h"
namespace videoElement
{
class videoElementDef;
}
class chamber;

using oState = enum ost { DISPOSED = 0,
                          nullptrREACHED = 1,
                          ERROR = 2
                        };


class bElem : public  std::enable_shared_from_this<bElem>
{
public:
    /*
     * these three templates below generate objects. I added them because I need bidirectional connection with everything
     */

    static void mechLock();
    static void mechUnlock();
    virtual ALLEGRO_MUTEX *getMyMutex();
    void registerLiveElement(std::shared_ptr<bElem> who);
    void deregisterLiveElement(int instanceId);
    static void runLiveElements();

    virtual sNeighboorhood getSteppableNeighboorhood();

    bElem();
    explicit bElem(std::shared_ptr<chamber> board);
    bElem(const bElem &) = delete;
    virtual ~bElem()=default;

    virtual void setBoard(std::shared_ptr<chamber> board);
    virtual std::shared_ptr<chamber> getBoard() const;
    virtual bool selfAlign();
    virtual bool stepOnElement(std::shared_ptr<bElem> step);
    virtual bool moveInDirection(direction d);
    virtual bool moveInDirectionSpeed(direction d, int speed);
    virtual bool destroy();
    virtual bool kill();
    virtual bool hurt(int points);
    virtual bool isSteppableDirection(direction di) const;
    virtual std::shared_ptr<bElem> getElementInDirection(direction di);
    virtual coords getAbsCoords(direction dir) const;
    virtual  int getType() const;
    virtual int getAnimPh() const;
    std::shared_ptr<bElemAttr> getAttrs() const;
    std::shared_ptr<bElemStats> getStats() const;

    virtual  float getViewRadius() const;
    virtual bool collect(std::shared_ptr<bElem> collectible);
    virtual bool dropItem(unsigned long int  instanceId);
    virtual std::shared_ptr<bElem> removeElement(); // removes element from the board, and returns it for further processing, usefull for eg. for collecting stuff
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual coords getOffset() const;
    virtual bool readyToShoot() const;
    static std::mt19937 randomNumberGenerator;
    static bool randomNumberGeneratorInitialized;

    virtual bool mechanics();
    virtual bool stepOnAction(bool step,std::shared_ptr<bElem> who);
    virtual bool collectOnAction(bool collected,std::shared_ptr<bElem> who);
    virtual bool use(std::shared_ptr<bElem> use);
    virtual bool interact(std::shared_ptr<bElem> who);



    static std::vector<std::shared_ptr<bElem>> liveElems;

    static void tick();
    static unsigned int getCntr() ;


    virtual bool isLocked();
    virtual bool lockThisObject(std::shared_ptr<bElem> who);
    virtual bool unlockThisObject(std::shared_ptr<bElem> who);

    virtual int getTypeInDirection(direction di);
    virtual void setStatsOwner(std::shared_ptr<bElem> owner);

    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<bElem> sbe);

    void playSound(std::string eventType,std::string event);
private:
    std::shared_ptr<bElemStats> status;
    std::shared_ptr<bElemAttr> attrs;


    void ps(std::shared_ptr<bElem> who,std::string eventType,std::string event);
    bool provisioned=false;

    std::shared_ptr<chamber> attachedBoard = nullptr;
    ALLEGRO_MUTEX *elementMutex = nullptr;
    static std::mutex mechanicMutex;
    static int instances;
    static unsigned int sTaterCounter;
    //  struct _cfg eConfig;
    //  struct _eStatus state;
    std::vector<std::shared_ptr<bElem>> lockers;
    static std::vector<int> toDeregister;
    static std::vector<std::shared_ptr<bElem>> toDispose;
};

#endif // BELEM_H
