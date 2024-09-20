
/*
 * Copyright (c) 2023, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
    int getAnimPhase() const;
    std::weak_ptr<bElem> getCollector() ;
    int getDestroyed() const;
    unsigned int getDestTimeBeg() const;
    unsigned int getDestTimeReq() const;
    dir::direction getFacing() const;
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
    dir::direction getMyDirection() const;
    coords getMyPosition() ;
    int getTaterCounter() const;
    int getWaiting() const;
    int getFadingIn() const;
    int getFadingInReq() const;
    int getFadingOutReq() const;
    int getFadingOut() const;
    int getMovingTotalTime() const;
    int getStats(pointsType t);
    int getPoints(pointsType ptype);


    void setStats(pointsType t,int value );
    // set methods
    void setAnimPhase(int value);
    void setCollector(std::weak_ptr<bElem> value);
    void setDopped();
    void setDestroyed(int value);
    void setDestTimeBeg(unsigned int value);
    void setDestTimeReq(unsigned int value);
    void setFacing(dir::direction value);
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
    void setMyDirection(dir::direction value);
    void setMyPosition(coords value);
    void setTaterCounter(int value);
    void setWaiting(int value);
    void setFadingIn(int value);
    void setFadingOut(int value);
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
    bool isFadingOut() const;
    bool isFadingIn() const;
    // can methods
    // none present

    // other methods
    void stopWaiting();
    int calculateValueInTime(int value) const;
    int getValueInTime(int value) const;

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
    int fadingOut =-1;
    int fadingIn=-1;
    unsigned int fadingInReq=0;
    unsigned int fadingOutReq=0;
    int waiting = -1;
    int moved = -1;
    int destroyed = -1;
    int animPhase = 0;
    int taterCounter = 5;
    int ammo = 0;
    int killed = -1;
    coords myPosition = NOCOORDS;
    dir::direction myDirection = dir::direction::NODIRECTION;
    dir::direction facing = dir::direction::NODIRECTION;

    std::shared_ptr<bElem> steppingOn = nullptr;
    std::weak_ptr<bElem> standingOn;
    std::weak_ptr<bElem> collector;
    std::weak_ptr<bElem> statsOwner;
};

#endif // BELEMSTATS_H
