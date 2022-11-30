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

typedef enum { DISPOSED=0,NULLREACHED=1,ERROR=2} oState;


class bElem
{
public:
    bElem();
    bElem(chamber *board);
    bElem(chamber *board,int x, int y);

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
    virtual bool selfAlign();
    virtual bool setSubtype(int st);
    virtual bool stepOnElement(bElem *step);
    virtual bool isLiveElement();

    virtual bElem* getStomper();
    virtual void stomp(bElem* who); //Notifies an object that other element is stepping on it, we get a stepper's reference
    virtual void unstomp();         //the object was released
    virtual bElem* getCollector();
    virtual void setCollected(bElem* who); //notify the object that it got collected
    virtual void setDropped(); // notify it got dropped

    virtual bool moveInDirection(direction d);
    virtual bool moveInDirectionSpeed(direction d,int speed);
    virtual bool use(bElem *use);

    virtual bool interact(bElem *who);
    virtual bool destroy();
    virtual bool kill();
    virtual bool hurt(int points);
    virtual bool isSteppable();
    virtual bool isSteppableDirection(direction di);
    virtual bElem *getElementInDirection(direction di);

    virtual bool canBeKilled();
    virtual bool canBeDestroyed();

    virtual coords getCoords();
    virtual coords getAbsCoords(direction dir);
    virtual direction getDirection();
    virtual bool setDirection(direction dir);
    virtual int getType();
    virtual int getSubtype();
    virtual int getAnimPh();
    virtual int getSwitchId();
    virtual int getEnergy();
    virtual bool setEnergy(int points);
    virtual bool isDying();
    virtual bool isDestroyed();
    virtual bool isTeleporting();
    virtual void setTeleporting(int time);
    virtual bool isInteractive();
    virtual bool isProvisioned();
    virtual bool isMovable();

    virtual bool isCollectible();
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
    virtual unsigned int getCntr();
    virtual chamber* getBoard();
    virtual constexpr bool isFading() { return false; };
    virtual bool isLocked();
    virtual bool lockThisObject(bElem* who);
    virtual bool unlockThisObject(bElem* who);
    constexpr bool isDisposed()
    {
        return this->disposed;
    };
    constexpr inventory* getInventory()
    {
        return this->myInventory ;
    };
    constexpr bElem* getSteppingOnElement()
    {
        return this->steppingOn;
    };
    void setInventory(inventory* inv);

protected:
    unsigned int interacted;
    bElem *steppingOn=NULL;
    int subtype;
    inventory *myInventory;
    std::vector<bElem*> lockers;
    int waiting;
    int instance;
    int destroyed;

    int animPhase=0;
    int taterCounter=0; //Internal counter

    int x,y;
    direction myDirection;
    bool amIUsable;
    int killed;
private:
    ALLEGRO_MUTEX* elementMutex;
    unsigned int telTimeReq;
    unsigned int telReqTime;
    unsigned int killTimeReq;
    unsigned int killTimeBeg;
    unsigned int destTimeReq;
    unsigned int destTimeBeg;
    virtual int getTypeInDirection(direction di);
    static int instances;
    bool disposed;
    elemStats* myStats;
    unsigned int telInProgress;
    bool hasActivatedMechanics;
    static unsigned int sTaterCounter;
    virtual void init();
    //  static videoElement::videoElementDef *def;
    bElem *stomping;
    bElem *collector;
    chamber *attachedBoard;
};

#endif // BELEM_H
