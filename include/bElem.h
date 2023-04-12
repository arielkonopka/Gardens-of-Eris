#ifndef BELEM_H
#define BELEM_H
#include <chrono>
#include <random>
#include <memory>
#include "commons.h"
#include "objectTypes.h"
#include "inventory.h"
#include "chamber.h"
#include "videoElementDef.h"
#include "elemStats.h"
#include "soundManager.h"
namespace videoElement
{
class videoElementDef;
}
class chamber;
class nonSteppable;

using  oState=enum ost { DISPOSED=0,nullptrREACHED=1,ERROR=2};
struct _cfg
{
    bool amIUsable=false;
    int subtype=0;
    std::shared_ptr<elemStats> myStats=nullptr;
    std::shared_ptr<inventory> myInventory=nullptr;
    int instance;
    bool provisioned=false;

};
struct _eStatus
{
    bool disposed=false;
    bool hasActivatedMechanics=false;

    unsigned int telTimeReq=0;
    unsigned int telReqTime=0;
    unsigned int killTimeReq=0;
    unsigned int killTimeBeg=0;
    unsigned int destTimeReq=0;
    unsigned int destTimeBeg=0;
    unsigned int telInProgress=0;
    std::shared_ptr<bElem> steppingOn=nullptr;
    std::shared_ptr<bElem> stomping=nullptr;
    std::shared_ptr<bElem> collector=nullptr;
    unsigned int interacted=0;
    int waiting=0;
    int destroyed=0;
    int animPhase=0;
    int taterCounter=0; //Internal counter
    coords myPosition=NOCOORDS;
    direction myDirection=NODIRECTION;
    direction facing=NODIRECTION; /* This is the direction used for selecting the icon */
    int killed=0;
};
class bElem: public virtual std::enable_shared_from_this<bElem>
{
public:

/*
 * these three templates below generate objects. I added them because I need bidirectional connection with everything
 */

    static videoElement::videoElementDef* vd;

    int getInstanceid() const;
    static void resetInstances();
    virtual ALLEGRO_MUTEX* getMyMutex();
    void registerLiveElement(std::shared_ptr<bElem> who);
    void deregisterLiveElement(int instanceId);
    static void runLiveElements();

    virtual sNeighboorhood getSteppableNeighboorhood();
    virtual ~bElem();
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual void setBoard(std::shared_ptr<chamber> board);
    virtual std::shared_ptr<chamber> getBoard();
    virtual void setCoords(int x, int y);
    virtual void setCoords(coords point);

    virtual void setActive(bool active);
    virtual constexpr bool selfAlign()
    {
        return false;
    };
    bool setSubtype(int st);
    virtual bool stepOnElement(std::shared_ptr<bElem> step);
    virtual bool isLiveElement();

    virtual std::shared_ptr<bElem> getStomper();
    virtual void stomp(std::shared_ptr<bElem> who); //Notifies an object that other element is stepping on it, we get a stepper's reference
    virtual void unstomp();         //the object was released
    virtual std::shared_ptr<bElem> getCollector();

    virtual void setCollected(std::shared_ptr<bElem> who); //notify the object that it got collected
    virtual void setDropped(); // notify it got dropped

    virtual bool moveInDirection(direction d);
    virtual bool moveInDirectionSpeed(direction d,int speed);
    virtual bool use(std::shared_ptr<bElem> use);

    virtual bool interact(std::shared_ptr<bElem> who);
    virtual bool destroy();
    virtual bool kill();
    virtual constexpr bool hurt(int points)
    {
        return false;
    };
    virtual bool isSteppable();
    virtual bool isSteppableDirection(direction di);
    virtual std::shared_ptr<bElem> getElementInDirection(direction di);

    virtual bool canBeKilled();
    virtual bool canBeDestroyed();

    virtual constexpr coords getCoords()
    {
        return this->state.myPosition;
    };
    virtual coords getAbsCoords(direction dir);
    virtual direction getDirection();
    virtual direction getFacing();
    virtual bool setDirection(direction dir);
    virtual bool setFacing(direction dir);
    virtual constexpr int getType()
    {
        return _belemType;
    };
    virtual int getSubtype();
    virtual int getAnimPh();
    virtual int getSwitchId();
    virtual int getEnergy();

    virtual bool setEnergy(int points);
    virtual bool isDying();
    virtual bool isDestroyed();
    virtual bool isTeleporting();
    virtual void setTeleporting(int time);
    virtual constexpr bool isProvisioned()
    {
        return this->eConfig.provisioned;
    };
    virtual bool isMovable();
    virtual constexpr bool isCollectible()
    {
        return false;
    };
    virtual const float getViewRadius() { return 0.0; };
    virtual bool canCollect();
    virtual bool collect(std::shared_ptr<bElem> collectible);
    virtual bool canInteract();
    virtual bool isUsable();
    virtual bool isWeapon();
    virtual int getAmmo();
    virtual void setAmmo();
    virtual void setMoved(int time);
    virtual int getMoved();
    virtual bool isActive();
    virtual bool isOpen();
    virtual bool isSwitchOn();
    std::shared_ptr<elemStats> getStats() ;
    void setStats(std::shared_ptr<elemStats> stat); // warning, unsafe method
    virtual bool isMod(); // is the element a mod of another element? - mod is an object that changes other object's behavior
    virtual modType getModType();
    virtual std::shared_ptr<bElem> removeElement(); // removes element from the board, and returns it for further processing, usefull for eg. for collecting stuff
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual coords getOffset();
    virtual bool isWaiting();
    virtual void setWait(int time);
    virtual int getWait() const;
    virtual void stopWaiting();
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

    virtual constexpr bool isFading()
    {
        return false;
    };
    virtual bool isLocked();
    virtual bool lockThisObject(std::shared_ptr<bElem> who);
    virtual bool unlockThisObject(std::shared_ptr<bElem> who);
    constexpr bool isDisposed()
    {
        return this->state.disposed;
    };
    std::shared_ptr<inventory> getInventory();

    std::shared_ptr<bElem> getSteppingOnElement();
    void setInventory(std::shared_ptr<inventory> inv);
    virtual int getTypeInDirection(direction di);
    virtual void setStatsOwner(std::shared_ptr<bElem> owner);
    bElem();
    explicit bElem(std::shared_ptr<chamber> board);
    virtual bool additionalProvisioning() ;

private:
    bElem(const bElem&) = delete;
    std::shared_ptr<chamber> attachedBoard=nullptr;
    ALLEGRO_MUTEX* elementMutex=nullptr;
    static int instances;
    static unsigned int sTaterCounter;
    struct _cfg eConfig;
    struct _eStatus state;
    std::vector<std::shared_ptr<bElem>> lockers;
    static std::vector<int> toDeregister;

};

#endif // BELEM_H
