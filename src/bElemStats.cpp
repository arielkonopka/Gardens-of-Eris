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

#include "bElemStats.h"
#include "../include/bElem.h"

unsigned long int bElemStats::currentInstance=0;

unsigned long int bElemStats::getInstanceId() const
{
    return this->instanceId;
}




bElemStats::bElemStats():instanceId(++bElemStats::currentInstance)
{

}

bool bElemStats::isDisposed() const
{
    return this->disposed;
}

void bElemStats::setDisposed(bool value)
{
    if (!this->disposed) // operation is one way, beware, if you set it to disposed, it will stay this way
        this->disposed = value;

    if (this->disposed)
        this->setMyPosition(NOCOORDS);

}

bool bElemStats::hasActivatedMechanics() const
{
    return this->activatedMechanics;
}

void bElemStats::setActivatedMechanics(bool value)
{
    this->activatedMechanics=value;

}

unsigned int bElemStats::getTelTimeReq() const
{
    return this->telTimeReq;
}

void bElemStats::setTelTimeReq(unsigned int value)
{
    this->telTimeReq=value;
}

unsigned int bElemStats::getTelReqTime() const
{
    return this->telReqTime;
}

void bElemStats::setTelReqTime(unsigned int value)
{
    this->telReqTime=value;
}

unsigned int bElemStats::getKillTimeReq() const
{
    return this->killTimeReq;
}

void bElemStats::setKillTimeReq(unsigned int value)
{
    this->killTimeReq=value;
}

unsigned int bElemStats::getKillTimeBeg() const
{
    return this->killTimeBeg;
}

void bElemStats::setKillTimeBeg(unsigned int value)
{
    this->killTimeBeg=value;
}

bool bElemStats::isDying() const
{
    return this->getKilled()>0;
}

unsigned int bElemStats::getDestTimeReq() const
{
    return this->destTimeReq;
}

void bElemStats::setDestTimeReq(unsigned int value)
{
    this->destTimeReq=value;
}

unsigned int bElemStats::getDestTimeBeg() const
{
    return this->destTimeBeg;
}

void bElemStats::setDestTimeBeg(unsigned int value)
{
    this->destTimeBeg=value;
}
bool bElemStats::isTeleporting() const
{
    return this->getTelInProgress()>0;
}

int bElemStats::getTelInProgress() const
{
    return this->getValueInTime(this->telInProgress);
}

void bElemStats::setTelInProgress(unsigned int value)
{
    this->telInProgress=this->calculateValueInTime(value);
    this->setTelReqTime(bElem::getCntr());
    this->setTelTimeReq(value);
}

std::shared_ptr<bElem> bElemStats::getSteppingOn() const
{
    return this->steppingOn;
}

void bElemStats::setSteppingOn(std::shared_ptr<bElem>value)
{
    this->steppingOn=value;
}

bool bElemStats::hasParent() const
{
    return this->parent && !this->standingOn.expired();
}

void bElemStats::setHasParent(bool value)
{
    this->parent=value;
}

std::weak_ptr<bElem> bElemStats::getStandingOn()
{
    if(this->hasParent() && !this->standingOn.expired())
        return this->standingOn;
    std::shared_ptr<bElem> b=nullptr;
    this->setHasParent(false);
    return b;
}

void bElemStats::setStandingOn(std::weak_ptr<bElem>value)
{
    this->standingOn=value;
    if(!value.expired())
        this->setHasParent(true);
    else
        this->setHasParent(false);
}

bool bElemStats::isCollected() const
{
    return this->collected;
}

void bElemStats::setCollected(bool value)
{
    this->collected=value;
}
void bElemStats::setDopped()
{

    this->setCollected(false);

}

std::weak_ptr<bElem> bElemStats::getCollector()
{
    if(this->isCollected() )
    {
        if(this->collector.expired())
            this->setCollected(false);
        return this->collector;
    }
    if(!this->collector.expired())
    {
        std::shared_ptr<bElem> ne=nullptr;
        this->setCollector(ne);
    }
    return this->collector;
}

void bElemStats::setCollector(std::weak_ptr<bElem>value)
{
    this->collector=value;
    if(value.expired())
        this->setCollected(false);
    else
        this->setCollected(true);
}
int bElemStats::getMovingTotalTime() const
{
    return this->movingTotalTime;
}

bool bElemStats::isMoving() const
{
    return this->getMoved()>0;
}

int bElemStats::getInteracted() const
{
    return this->getValueInTime(this->interacted);
}

void bElemStats::setInteracted(unsigned int value)
{
    this->interacted=this->calculateValueInTime(value);
}

int bElemStats::getWaiting() const
{
    return this->getValueInTime(this->waiting);
}

void bElemStats::setWaiting(int value)
{
    this->waiting=this->calculateValueInTime(value);
}
bool bElemStats::isWaiting() const
{
    return this->getWaiting()>0;
}

void bElemStats::stopWaiting()
{
    this->setWaiting(0);
}

int bElemStats::getDestroyed() const
{
    return this->getValueInTime(this->destroyed);
}

int bElemStats::isDestroying() const
{
    return this->getDestroyed()>0;
}


void bElemStats::setDestroyed(int value)
{
    this->destroyed=this->calculateValueInTime(value);
    this->setDestTimeBeg(bElem::getCntr());
    this->setDestTimeReq(value);
}

bool bElemStats::isInteracting() const
{
    return this->getInteracted()>0;
}



int bElemStats::getAnimPhase() const
{
    return this->animPhase;
}

void bElemStats::setAnimPhase(int value)
{
    this->animPhase=value;
}

int bElemStats::getTaterCounter() const
{
    return this->taterCounter; // we have our own time thing
}

void bElemStats::setTaterCounter(int value)
{
    this->taterCounter=value;
}

coords bElemStats::getMyPosition()
{
    if(this->collected)
    {
        std::shared_ptr<bElem> be=(this->getCollector().lock());
        if(be)
        {
            return be->getStats()->getMyPosition();
        }
    }
    return this->myPosition;
}

void bElemStats::setMyPosition(coords value)
{
    this->myPosition=value;
}

dir::direction bElemStats::getMyDirection() const
{
    return this->myDirection;
}

void bElemStats::setMyDirection(dir::direction value)
{
    this->myDirection=value;
}

dir::direction bElemStats::getFacing() const
{
    return this->facing;
}


void bElemStats::setFacing(dir::direction value)
{
    this->facing=value;
}

int bElemStats::getKilled() const
{
    return this->getValueInTime(this->killed);

}

void bElemStats::setKilled(int value)
{
    this->killed=calculateValueInTime(value);
    this->setKillTimeBeg(bElem::getCntr());
    this->setKillTimeReq(value);
}

/**
 * @brief Get the time-adjusted value.
 *
 * This method returns the number of clock ticks remaining for a value that could be in the future.
 *
 * @param value The value to be adjusted.
 * @return The adjusted value. It represents:
 * - future if it's greater than zero
 * - present if it's zero
 * - none or past if it's less than zero (specifically -1)
 */
int bElemStats::getValueInTime(int value) const
{
    return (value > 0 && value >= (int)bElem::getCntr()) ? value - (int)bElem::getCntr() : -1;
}


int bElemStats::calculateValueInTime(int value) const
{
    return bElem::getCntr()+value;
}

bool bElemStats::isActive() const
{
    return this->active;
}

void bElemStats::setActive(bool value)
{
    this->active=value;
}

int bElemStats::getMoved() const
{
    return this->getValueInTime(this->moved);
}

void bElemStats::setMoved(int value)
{
    this->moved=this->calculateValueInTime(value);
    this->movingTotalTime=value;
}

int bElemStats::getFadingOut() const
{
    return this->getValueInTime(this->fadingOut);
}
int bElemStats::getFadingIn() const
{
    return this->getValueInTime(this->fadingIn);
}

void bElemStats::setFadingIn(int value)
{
    this->fadingIn=this->calculateValueInTime(value);
    this->fadingInReq=bElem::getCntr();

}

void bElemStats::setFadingOut(int value)
{
    this->fadingOut=this->calculateValueInTime(value);
    this->fadingOutReq=bElem::getCntr();
}
int bElemStats::getFadingInReq() const
{
    return this->fadingInReq;
}

int bElemStats::getFadingOutReq() const
{
    return this->fadingOutReq;
}

bool bElemStats::isFadingIn() const
{
    return this->getFadingIn()>0;
}

bool bElemStats::isFadingOut() const
{
    return this->getFadingOut()>0;
}

bool bElemStats::isMarked() const
{
    return this->marked;
}

void bElemStats::setMarked(bool value)
{
    this->marked=value;
}


/**
 * @brief Retrieves the statistic value for a given pointsType key.
 *
 * This method returns the statistic value for the provided pointsType key.
 * The key corresponds to one of the enum values defined in pointsType (TOTAL, SHOOT, STEPS, COLLECTS).
 * If the key does not exist in the map, it returns 0.
 *
 * @param t The pointsType key whose associated value is to be returned.
 * @return The value to which the specified pointsType key is mapped, or 0 if this map contains no mapping for the key.
 */
int bElemStats::getStats(pointsType t)
{
    if(this->statistics.find(t)!=this->statistics.end())
        return this->statistics[t];
    return 1;
}

/**
 * @brief Sets the statistic value for a given pointsType key.
 *
 * This method sets the statistic value for the provided pointsType key in the map.
 * The key corresponds to one of the enum values defined in pointsType (TOTAL, SHOOT, STEPS, COLLECTS).
 * If the key does not exist in the map, it is created.
 *
 * @param t The pointsType key whose associated value is to be set.
 * @param value The value to be set.
 */
void bElemStats::setStats(pointsType t, int value)
{
    this->statistics[t]=value;
}


std::weak_ptr<bElem> bElemStats::getStatsOwner() const
{
    return this->statsOwner;
}

void bElemStats::setStatsOwner(std::shared_ptr<bElem>own)
{
    this->statsOwner=own;
}

int bElemStats::getPoints(pointsType ptype)
{
    if(statistics.find(ptype)!=statistics.end())
        return this->statistics[ptype];
    return 0;
}

void bElemStats::setPoints(pointsType ptype, int val)
{
    this->statistics[ptype]=val;
}





