#ifndef BELEM_H
#define BELEM_H
#include "commons.h"
#include "objectTypes.h"


#include "inventory.h"
//#include "rubbish.h"
#include "chamber.h"
#include "gCollect.h"
#include "videoElementDef.h"
#include <chrono>
#include <random>
//class chamber;
//class gCollect; // we will keep objects to be removed from the memory here. this is convenient because we would not fore an object to commit suicide, but instead we would "park" it here for later cleanup

#include "gCollect.h"

#include "elemStats.h"
namespace videoElement
{


class videoElementDef;
}
//class gCollect;
class chamber;

//class inventory
class nonSteppable;

typedef enum { DISPOSED=0,nullptrREACHED=1,ERROR=2} oState;


class bElem
{
public:
    bElem();
    bElem(chamber *board);

    static videoElement::videoElementDef* vd;

    virtual int getInstanceid();
    static void resetInstances();
    virtual ALLEGRO_MUTEX* getMyMutex();
    void registerLiveElement(bElem* who);
    void deregisterLiveElement(bElem* who);
    static void runLiveElements();

    virtual sNeighboorhood getSteppableNeighboorhood();
    virtual ~bElem();
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual void setBoard(chamber *board);
    virtual void setCoords(int x, int y);
    virtual void setCoords(coords point);

    virtual void setActive(bool active);
    virtual constexpr bool selfAlign()
    {
        return false;
    };
    virtual bool setSubtype(int st);
    virtual bool stepOnElement(bElem *step);
    virtual bool isLiveElement();

    virtual bElem* getStomper();
    virtual void stomp(bElem* who); //Notifies an object that other element is stepping on it, we get a stepper's reference
    virtual void unstomp();         //the object was released
    virtual constexpr bElem* getCollector()
    {
        return this->state.collector;
    };
    virtual void setCollected(bElem* who); //notify the object that it got collected
    virtual void setDropped(); // notify it got dropped

    virtual bool moveInDirection(direction d);
    virtual bool moveInDirectionSpeed(direction d,int speed);
    virtual bool use(bElem *use);

    virtual bool interact(bElem *who);
    virtual bool destroy();
    virtual bool kill();
    virtual constexpr bool hurt(int points)
    {
        return false;
    };
    virtual bool isSteppable();
    virtual bool isSteppableDirection(direction di);
    virtual bElem *getElementInDirection(direction di);

    virtual bool canBeKilled();
    virtual bool canBeDestroyed();

    virtual constexpr coords getCoords()
    {
        return this->state.myPosition;
    };
    virtual coords getAbsCoords(direction dir);
    virtual direction getDirection();
    virtual bool setDirection(direction dir);
    virtual constexpr int getType() { return _belemType; };
    virtual int getSubtype();
    virtual int getAnimPh();
    virtual int getSwitchId();
    virtual constexpr int getEnergy()
    {
        return (this->getStats()!=nullptr)?this->getStats()->getEnergy():555;
    };
    virtual bool setEnergy(int points);
    virtual bool isDying();
    virtual bool isDestroyed();
    virtual bool isTeleporting();
    virtual void setTeleporting(int time);
    virtual bool isProvisioned();
    virtual bool isMovable();

    virtual constexpr bool isCollectible()
    {
        return false;
    };
    virtual bool canCollect();
    virtual bool collect(bElem* collectible);
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
    virtual elemStats* getStats();
    virtual void setStats(elemStats* stat); // warning, unsafe method
    virtual bool isMod(); // is the element a mod of another element? - mod is an object that changes other object's behavior
    virtual modType getModType();
    virtual bElem* removeElement(); // removes element from the board, and returns it for further processing, usefull for eg. for collecting stuff
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual coords getOffset();
    virtual bool isWaiting();
    virtual void setWait(int time);
    virtual int getWait();
    virtual void stopWaiting();
    static std::mt19937 randomNumberGenerator;
    static bool randomNumberGeneratorInitialized;
    virtual bool mechanics();
    static std::vector<bElem*> liveElems;

    static void tick();
    virtual  unsigned int getCntr();
    virtual chamber* getBoard();
    virtual constexpr bool isFading()
    {
        return false;
    };
    virtual bool isLocked();
    virtual bool lockThisObject(bElem* who);
    virtual bool unlockThisObject(bElem* who);
    constexpr bool isDisposed()
    {
        return this->state.disposed;
    };
    constexpr inventory* getInventory()
    {
        return this->eConfig.myInventory;
    };
    constexpr bElem* getSteppingOnElement()
    {
        return this->state.steppingOn;
    };
    void setInventory(inventory* inv);
    virtual int getTypeInDirection(direction di);

private:
    chamber *attachedBoard=nullptr;
    ALLEGRO_MUTEX* elementMutex=nullptr;
    static int instances;
    static unsigned int sTaterCounter;
    struct _cfg
    {
        bool amIUsable=false;
        int subtype=0;
        elemStats* myStats=nullptr;
        inventory *myInventory=nullptr;
        int instance;

    } eConfig;
    struct _st
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
        bElem *steppingOn=nullptr;
        bElem *stomping=nullptr;
        bElem *collector=nullptr;
        unsigned int interacted=0;
        int waiting=0;
        int destroyed=0;
        int animPhase=0;
        int taterCounter=0; //Internal counter
        coords myPosition=NOCOORDS;
        direction myDirection=NODIRECTION;
        int killed=0;
    } state;
    std::vector<bElem*> lockers;


};

#endif // BELEM_H
