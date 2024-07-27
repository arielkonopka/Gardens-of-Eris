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
    void deregisterLiveElement(unsigned int instanceId);
    static void runLiveElements();

    virtual sNeighboorhood getSteppableNeighborhood();

    bElem();
    bElem(const bElem &) = delete;
    virtual ~bElem()=default;

    virtual void setBoard(std::shared_ptr<chamber> board);
    virtual std::shared_ptr<chamber> getBoard() const;
    virtual bool selfAlign();
    virtual bool stepOnElement(std::shared_ptr<bElem> step);
    virtual bool moveInDirection(dir::direction d);
    virtual bool moveInDirectionSpeed(dir::direction dir, int speed);
    virtual bool dragInDirection(dir::direction dragIntoDirection);
    virtual bool dragInDirectionSpeed(dir::direction dragIntoDirection,int speed);
    virtual bool destroy();
    virtual bool kill();
    virtual bool hurt(int points);
    virtual bool isSteppableDirection(dir::direction di) const;
    virtual bool isSteppableDirection(coords di) const;
    bool isSteppableInMyDirection() const;
    virtual std::shared_ptr<bElem> getElementInDirection(dir::direction di) ;
    virtual std::shared_ptr<bElem> getElementInDirection(coords di) ;
    virtual coords getAbsCoords(dir::direction dir) const;
    virtual coords getAbsCoords(coords dir) const;
    virtual  int getType() const;
    virtual int getAnimPh() const;
    std::shared_ptr<bElemAttr> getAttrs() const;
    std::shared_ptr<bElemStats> getStats() const;

    virtual  float getViewRadius() const;
    virtual bool collect(std::shared_ptr<bElem> collectible);
    virtual bool dropItem(unsigned long int  instanceId);
    virtual std::shared_ptr<bElem> removeElement(); // removes element from the board, and returns it for further processing, useful for e.g. for collecting stuff
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual coords getOffset() const;
    virtual bool readyToShoot() const;
    static std::mt19937 randomNumberGenerator;

    virtual bool mechanics();
    virtual bool stepOnAction(bool step,std::shared_ptr<bElem> who);
    virtual bool collectOnAction(bool collected,std::shared_ptr<bElem> who);
    virtual bool use(std::shared_ptr<bElem> use);
    virtual bool interact(std::shared_ptr<bElem> who);

    static void tick();
    static unsigned int getCntr() ;

    std::shared_ptr<bElem> findInDir(dir::direction dir);

    virtual bool isLocked();
    virtual bool lockThisObject(std::shared_ptr<bElem> who);
    virtual bool unlockThisObject(std::shared_ptr<bElem> who);

    virtual int getTypeInDirection(dir::direction di);
    virtual void setStatsOwner(std::shared_ptr<bElem> owner);
    virtual bool additionalProvisioning(int subtype);

    void playSound(std::string eventType,std::string event);
    void stopMySounds();

//   static std::vector<unsigned long int> toDeregister;
private:
    std::shared_ptr<bElemStats> status;
    std::shared_ptr<bElemAttr> attrs;

    std::once_flag _provOnce;
    void ps(std::shared_ptr<bElem> who,std::string eventType,std::string event);
    std::weak_ptr<chamber> attachedBoard;
    ALLEGRO_MUTEX *elementMutex = nullptr;
    static std::mutex mechanicMutex;
    static unsigned int sTaterCounter;
    std::vector<std::shared_ptr<bElem>> lockers;
    static std::vector<std::shared_ptr<bElem>> toDispose;
};

#endif // BELEM_H
