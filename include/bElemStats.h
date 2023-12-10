#ifndef BELEMSTATS_H
#define BELEMSTATS_H
#include "../include/commons.h"
#include <memory>
#include <unordered_map>
class bElem;



class bElemStats
{
public:
    bElemStats();
    ~bElemStats() =default;

    // get methods
    unsigned long int getInstanceId() const;
    int getAmmo() const;
    int getAnimPhase() const;
    std::weak_ptr<bElem> getCollector() ;
    int getDestroyed() const;
    unsigned int getDestTimeBeg() const;
    unsigned int getDestTimeReq() const;
    direction getFacing() const;
    int getInteracted() const;
    unsigned int getKillTimeBeg() const;
    unsigned int getKillTimeReq() const;
    int getKilled() const;
    int getTelInProgress() const;
    unsigned int getTelReqTime() const;
    unsigned int getTelTimeReq() const;
    std::shared_ptr<bElem> getSteppingOn() const;
    std::weak_ptr<bElem> getStandingOn();
    std::weak_ptr<bElem> getStatsOwner() const;
    int getMoved() const;
    direction getMyDirection() const;
    coords getMyPosition() ;
    int getTaterCounter() const;
    int getWaiting() const;
    int getFading() const;
    int getMovingTotalTime() const;
    int getStats(pointsType t);
    int getPoints(pointsType ptype);


    void setStats(pointsType t,int value );
    // set methods
    void setAmmo(int value);
    void setAnimPhase(int value);
    void setCollector(std::weak_ptr<bElem> value);
    void setDopped();
    void setDestroyed(int value);
    void setDestTimeBeg(unsigned int value);
    void setDestTimeReq(unsigned int value);
    void setFacing(direction value);
    void setInteracted(unsigned int value);
    void setKillTimeBeg(unsigned int value);
    void setKillTimeReq(unsigned int value);
    void setKilled(int value);
    void setTelInProgress(unsigned int value);
    void setTelReqTime(unsigned int value);
    void setTelTimeReq(unsigned int value);
    void setSteppingOn(std::shared_ptr<bElem> value);
    void setStandingOn(std::weak_ptr<bElem> value);
    void setMoved(int value);
    void setMyDirection(direction value);
    void setMyPosition(coords value);
    void setTaterCounter(int value);
    void setWaiting(int value);
    void setFading(int value);
    void setPoints(pointsType ptype,int val);
    // has methods
    bool hasActivatedMechanics() const;
    bool hasParent() const;
    bool isMarked() const;


    // is methods
    bool isActive() const;
    bool isCollected() const;
    bool isDisposed() const;
    bool isDying() const;
    bool isTeleporting() const;
    bool isWaiting() const;
    bool isInteracting() const;
    int isDestroying() const;
    bool isMoving() const;
    bool isFading() const;
    // can methods
    // none present

    // other methods
    void stopWaiting();
    int calculateValueInTime(int value) const;
    int getValueInTime(int value) const;

    void createInstanceId();
    // Set methods for boolean values
    void setActivatedMechanics(bool value);
    void setDisposed(bool value);
    void setHasParent(bool value);
    void setCollected(bool value);
    void setActive(bool value);
    void setMarked(bool value);
    void setStatsOwner(std::shared_ptr<bElem> own);
private:
    static unsigned long int currentInstance;
    unsigned long int instanceId;
    bool disposed = false;
    bool active = false;
    bool marked=false;
    bool activatedMechanics = false;
    bool parent = false;
    bool collected = false;

    unsigned int telTimeReq = 0;
    unsigned int telReqTime = 0;
    unsigned int killTimeReq = 0;
    unsigned int killTimeBeg = 0;
    unsigned int destTimeReq = 0;
    unsigned int destTimeBeg = 0;
    unsigned int telInProgress = 0;
    unsigned int interacted = 0;
    std::unordered_map<pointsType,int> statistics;
    int movingTotalTime=-1;
    int fading =-1;
    int waiting = -1;
    int moved = -1;
    int destroyed = -1;
    int animPhase = 0;
    int taterCounter = 5;
    int ammo = 0;
    int killed = -1;
    coords myPosition = NOCOORDS;
    direction myDirection = NODIRECTION;
    direction facing = NODIRECTION;

    std::shared_ptr<bElem> steppingOn = nullptr;
    std::weak_ptr<bElem> standingOn;
    std::weak_ptr<bElem> collector;
    std::weak_ptr<bElem> statsOwner;
};

#endif // BELEMSTATS_H
