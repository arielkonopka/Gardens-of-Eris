#include "bElemStats.h"
#include "../include/bElem.h"

unsigned long int bElemStats::currentInstance=0;

unsigned long int bElemStats::getInstanceId() const
{
    return this->instanceId;
}
void bElemStats::createInstanceId()
{
    this->instanceId=++bElemStats::currentInstance;
}



bElemStats::bElemStats()
{
   this->createInstanceId();
}

bElemStats::~bElemStats()
{
    //dtor
}
bool bElemStats::isDisposed() const
{
    return this->disposed;
}

void bElemStats::setDisposed(bool value)
{
    if(this->disposed==false) // operation is one way, beware, if you set it to disposed, it will stay this way
        this->disposed=value;
    if(this->disposed)
    {
        this->setMyPosition(NOCOORDS);
    }
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

coords bElemStats::getMyPosition() const
{
    return this->myPosition;
}

void bElemStats::setMyPosition(coords value)
{
    this->myPosition=value;
}

direction bElemStats::getMyDirection() const
{
    return this->myDirection;
}

void bElemStats::setMyDirection(direction value)
{
    this->myDirection=value;
}

direction bElemStats::getFacing() const
{
    return this->facing;
}


void bElemStats::setFacing(direction value)
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
int bElemStats::getAmmo() const
{
    return this->ammo;
}

void bElemStats::setAmmo(int value)
{
    this->ammo=value;
}

int bElemStats::getFading() const
{
    return this->getValueInTime(this->fading);
}

void bElemStats::setFading(int value)
{
    this->fading=this->calculateValueInTime(value);
}

bool bElemStats::isFading() const
{
    return this->getFading()>0;
}




