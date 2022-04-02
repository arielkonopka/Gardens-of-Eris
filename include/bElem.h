#ifndef BELEM_H
#define BELEM_H
#include <stddef.h>
#include "objectTypes.h"
//#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include "gCollect.h"
#include "videoElementDef.h"
#include <chrono>
#include <random>
//class chamber;
//class gCollect; // we will keep objects to be removed from the memory here. this is convenient because we would not fore an object to commit suicide, but instead we would "park" it here for later cleanup

#include "gCollect.h"


class gCollect;
class chamber;



typedef enum { DISPOSED=0,NULLREACHED=1,ERROR=2} oState;

#define NOCOORDS   ((coords){-1,-1})
class bElem
{
public:
    static videoElement::videoElementDef* vd;
    virtual int getInstanceid();
    static void resetInstances();
    static int instances;


    bElem();
    bElem(chamber *board,gCollect *garbage);
    bElem(chamber *board,gCollect *garbage,int x, int y);

    virtual ~bElem();
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual void setBoard(chamber *board);
    virtual void setGarbageBin(gCollect *garbage);
    virtual void setCoords(int x, int y);


    virtual bool selfAlign();
    virtual bool setSubtype(int st);
    virtual bool stepOnElement(bElem *step);
    virtual bool moveInDirection(direction d);
    virtual bool use(bElem *use);
    virtual bool getActive();
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
    virtual int getCnt();
    virtual int getAnimPh();
    virtual int getSwitchId();
    virtual int getEnergy();
    virtual bool setEnergy(int points);
    virtual bool isDying();
    virtual bool isDestroyed();
    virtual bool isTeleporting();

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

    virtual bool isActive();
    virtual bool isOpen();
    virtual bool isSwitchOn();

    virtual bElem* removeElement(); // removes element from the board, and returns it for further processing, usefull for eg. for collecting stuff
    oState disposeElement();
    bElem *steppingOn=NULL;
    std::vector<bElem *> collectedItems;
    bool removeFromcollection(int position);
    virtual bElem* getCollector();
    virtual void setCollector(bElem* collector);
    int interacted;
    std::mt19937 randomNumberGenerator;
    /*
        @mechanics(bool collected) - takes care both of time passing (all the timers and so on along with the mechanics itself - every object type can have its own rules
        collected==true when the method is invoked from an objects inventory. useful for objects that do something when collected like mines, automatic weapons and so on
    */
    virtual bool mechanics(bool collected);
protected:
    bElem* collectedBy;
    int instance;
    int destroyed;
    int energy;
    int subtype=0;
    int animPhase=0;
    int taterCounter=0; //Internal counter
    chamber *attachedBoard;
    gCollect *garbageBin;
    int x,y;
    direction myDirection;
    bool amIUsable;
    int killed;
private:
    virtual void init();
    //  static videoElement::videoElementDef *def;

};

#endif // BELEM_H
