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
#include "../include/bElem.h"
#include "floorElement.h"
#include "rubbish.h"
#include "elements.h"
#include <algorithm>
//std::vector<std::shared_ptr<bElem>> bElem::liveElems;
std::vector<std::shared_ptr<bElem>> bElem::toDispose;
//std::vector<unsigned long int> bElem::toDeregister;
unsigned int bElem::sTaterCounter = 5;
int bElem::instances = 0;
bool bElem::randomNumberGeneratorInitialized = false;
std::mt19937 bElem::randomNumberGenerator;
std::mutex bElem::mechanicMutex;

bElem::bElem() : std::enable_shared_from_this<bElem>(), elementMutex(al_create_mutex_recursive())
{
    static std::once_flag _of;
    this->status=std::make_shared<bElemStats>();
    this->getStats()->setMyDirection(dir::direction::UP);
    this->getStats()->setFacing(this->getStats()->getMyDirection());
    std::call_once(_of,[]()
    {
        std::random_device rd;
        std::array<int,4> seedData;
        std::generate_n(seedData.data(), seedData.size(), std::ref(rd));
        std::seed_seq seq(std::begin(seedData), std::end(seedData));
        bElem::randomNumberGenerator.seed(seq);
        bElem::randomNumberGeneratorInitialized=true;
    });
}





coords bElem::getOffset() const
{

    coords res= {0,0};
    int rx=configManager::getInstance()->getConfig()->tileWidth;
    int ry=configManager::getInstance()->getConfig()->tileHeight;
    if(this->getStats()->isMoving() && this->getStats()->getMovingTotalTime()>0)
    {
        coords interm={(this->getStats()->getMoved()*rx)/this->getStats()->getMovingTotalTime(),(this->getStats()->getMoved()*ry)/this->getStats()->getMovingTotalTime()};
        res=dir::dirToCoords(this->getStats()->getMyDirection())*interm*(-1);
    }
    return res;
}

bool bElem::collectOnAction(bool collected, std::shared_ptr<bElem>who)
{
    if (collected && who && who->getType()==_player)
    {
        this->playSound("Found","Collect");
    }
    return true;
}




std::shared_ptr<chamber> bElem::getBoard() const
{
    return this->attachedBoard.lock();
}

void bElem::setBoard(std::shared_ptr<chamber> board)
{
    this->attachedBoard = board;
    if (this->getAttrs() && this->getAttrs()->canCollect())
    {
        this->getAttrs()->getInventory()->updateBoard();
    }
}

bool bElem::dropItem(unsigned long int  instanceId)
{
    if(!this->getAttrs()->canCollect())
        return false;

    std::shared_ptr<bElem> item=this->getAttrs()->getInventory()->retrieveCollectibleFromInventory(instanceId,true);
    if(!item)
        return false;
    item->collectOnAction(false,shared_from_this());
    for(auto d:dir::allDirections)
    {
        coords dir=dir::dirToCoords(d);

        if(this->isSteppableDirection(dir))
        {
            if (this->getType()==_player)
                item->playSound("Drop","Item");
            item->stepOnElement(this->getElementInDirection(dir));
            return true;
        }
    }
    return this->collect(item); // re-collect, since there was no place to drop it.
}



/*
   This is the most basic method for dealing with all objects. Because all of them must be put to a place on a boart at some point,
   and this is the method that is to be used for that purpose.
   It should work on all types of configurations, except standing on an empty point
   If places on an object that already is covered by another one, the newly placed object is placed in between
   This method takes "steppable" flag into consideration
*/
bool bElem::stepOnElement(std::shared_ptr<bElem> step)
{
    auto elig = [](std::shared_ptr<bElem> step) -> bool
    {
        if (!step || !step->getAttrs()->isSteppable() || !step->getBoard() || step->getStats()->isDisposed() || step->getStats()->getMyPosition() == NOCOORDS)
            return false;
        else
            return true;
    };
    if(this->getStats()->isDisposed() || !elig(step))
        return false;
    bool chamberChange=!(step->getBoard()==this->getBoard());
    std::shared_ptr<bElem> s0;
    if(step->getAttrs()->isCollectible() && this->getAttrs()->canCollect())
    {
        std::shared_ptr<bElem> s2=step->getStats()->getSteppingOn();
        if(!elig(s2))
            return false;
        this->collect(step);
        step=s2;
        if (step) {
            return this->stepOnElement(step);
        } else {
            return false;
        }
    }
    std::shared_ptr<bElem> st=this->getStats()->getSteppingOn();
    if(chamberChange && this->getStats()->hasActivatedMechanics())
        this->deregisterLiveElement(this->getStats()->getInstanceId());
    else
        chamberChange=false;
    this->removeElement();
    if(st)
        st->stepOnAction(false,shared_from_this());
    bool hp=step->getStats()->hasParent();
    this->setBoard(step->getBoard());
    this->getStats()->setMyPosition(step->getStats()->getMyPosition());
    this->getStats()->setSteppingOn(step);
    s0=step->getStats()->getStandingOn().lock();
    step->getStats()->setStandingOn(shared_from_this());
    if(hp)
    {
        s0->getStats()->setSteppingOn(shared_from_this());
        this->getStats()->setStandingOn(s0);
    }
    else
    {
        this->getBoard()->setElement(this->getStats()->getMyPosition(),shared_from_this());
    }
    if(chamberChange)
        this->registerLiveElement(shared_from_this());
    step->stepOnAction(true,shared_from_this());
    return true;
}

oState bElem::disposeElementUnsafe()
{
    oState res = DISPOSED;
    std::shared_ptr<chamber> myBoard = this->getBoard();
    coords mycoords = this->getStats()->getMyPosition();
    if (this->getStats()->isDisposed() )
        return ERROR;
    if (this->getStats()->hasActivatedMechanics())
        this->deregisterLiveElement(this->getStats()->getInstanceId());

    this->getStats()->setDisposed(true);
    if (mycoords.x >= 0 && mycoords.y >= 0 && this->getBoard() != nullptr) // object on a board? need extra steps
    {
        if (this->getStats()->getSteppingOn() || this->getStats()->hasParent())
        {
            this->removeElement();
            res = DISPOSED;
        }
        else
        {
            this->getBoard()->setElement(this->getStats()->getMyPosition(), nullptr);
            res = nullptrREACHED;
        }

        if (this->getType() != _stash && this->getAttrs()->canCollect() && !this->getAttrs()->getInventory()->isEmpty()  && this->getType() != _rubishType && this->getType() != _plainMissile && this->getType() != _plainGun)
        {
            std::shared_ptr<bElem> stash = elementFactory::generateAnElement<rubbish>(myBoard,0);
            stash->getAttrs()->setInventory(this->getAttrs()->getInventory());
            stash->getAttrs()->getInventory()->changeOwner(stash);
            this->getAttrs()->setInventory(nullptr);
            if (myBoard->getElement(mycoords)->getAttrs()->isSteppable())
            {
                stash->stepOnElement(myBoard->getElement(mycoords));
            }
            else
            {
                bool stashed = false;
                for (int c = 0; c < 4; c++)
                {
                    if (myBoard->getElement(mycoords)->isSteppableDirection((dir::direction)c))
                    {
                        stash->stepOnElement(myBoard->getElement(mycoords)->getElementInDirection((dir::direction)c));
                        stashed = true;
                        break;
                    }
                }
                if (!stashed)
                    stash->disposeElement(); // no place for the stash? Burn!
            }
        }
    }
    al_destroy_mutex(this->elementMutex);
    soundManager::getInstance()->stopSoundsByElementId(this->getStats()->getInstanceId());
    this->getStats()->setDisposed(true);
    this->getStats()->setMyPosition(NOCOORDS);
    this->attachedBoard.reset();
    return res; // false means that there is no more elements to go.

}

oState bElem::disposeElement()
{
    //std::shared_ptr<bElem> t = shared_from_this();
    std::shared_ptr<bElem> stash = nullptr;
    std::shared_ptr<chamber> _myBoard=this->getBoard();
    coords oCoords = this->getStats()->getMyPosition();
    if (this->getStats()->isDisposed() )
    {
        return ERROR;
    }
    /// We first deal with the activated mechanics, as it will depend on the board attached to the element.
    if (this->getStats()->hasActivatedMechanics())
    {
        this->deregisterLiveElement(this->getStats()->getInstanceId());
    }
    this->removeElement();

    if(this->getAttrs() && this->getAttrs()->canCollect())
    {
        if (this->getType() == _rubishType )
        {
            this->getAttrs()->getInventory()->weapons.clear();
            this->getAttrs()->getInventory()->keys.clear();
            this->getAttrs()->getInventory()->tokens.clear();
        }
        else if ( oCoords != NOCOORDS && !this->getAttrs()->getInventory()->isEmpty() && _myBoard)
        {
            /*
                Create a rubbish element on the board, so the inventory would not be lost
             */
            stash = elementFactory::generateAnElement<rubbish>(this->getBoard(),0);
            stash->getAttrs()->setCollect(true);
            stash->getAttrs()->getInventory()->mergeInventory(this->getAttrs()->getInventory());
            stash->stepOnElement(_myBoard->getElement(oCoords));
        }
    }
    this->getStats()->setDisposed(true);
    this->setBoard(nullptr);
    this->getStats()->setMyPosition(NOCOORDS);
    al_destroy_mutex(this->elementMutex);
    soundManager::getInstance()->stopSoundsByElementId(this->getStats()->getInstanceId());
    return DISPOSED;
}

/*
 * This method returns absolute coordinates, when asked for coordinates of the next cell in a direction from the elements's standpoint, should be resistant for non-provisioned units
 * better use those on a board though
 */
coords bElem::getAbsCoords(coords dir) const
{
   if(this->getStats()->getMyPosition()==NOCOORDS || !this->getBoard())
       return NOCOORDS;
    coords res=(this->getStats()->getMyPosition()+dir).validate((coords){this->getBoard()->width,this->getBoard()->height});
    return res;
}


coords bElem::getAbsCoords(dir::direction dir) const
{
    return this->getAbsCoords(dir::dirToCoords(dir));
}

std::shared_ptr<bElem> bElem::getElementInDirection(dir::direction di)
{
   return this->getElementInDirection(dir::dirToCoords(di));
}
std::shared_ptr<bElem> bElem::getElementInDirection(coords di)
{
    coords crd=this->getAbsCoords(di);
    if (this->attachedBoard.expired() || crd==NOCOORDS || this->getStats()->getMyPosition()==NOCOORDS)
        return nullptr;
    if (di==(coords){0,0})
        return shared_from_this();
    return this->attachedBoard.lock()->getElement(crd);
}


ALLEGRO_MUTEX *bElem::getMyMutex()
{
    return this->elementMutex;
}



bool bElem::use(std::shared_ptr<bElem> who)
{
    return false;
}

bool bElem::interact(std::shared_ptr<bElem> who)
{
    if (this->getAttrs()->isInteractive() && !this->getStats()->isInteracting()) /* penalty for getting into counter overflow */
    {
        this->getStats()->setInteracted( _interactedTime);
        return true;
    }
    return false;
}

bool bElem::destroy()
{
    if (this->getAttrs()->isDestroyable() || this->getAttrs()->isSteppable() || this->getAttrs()->isKillable())
    {
        if (this->getStats()->isDying())
        {
            this->getStats()->setKilled(0);
            this->getStats()->setKillTimeBeg(0);
            this->getStats()->setKillTimeReq(0);
        }
        this->getStats()->setDestroyed(_defaultDestroyTime);
        if (this->getAttrs()->isDestroyable() || this->getAttrs()->isKillable())
        {
            bElem::toDispose.push_back(shared_from_this());
        }
        return true;
    }
    return false;
}


bool bElem::selfAlign()
{
    return false;
}

int bElem::getAnimPh() const
{
    int base = (int)bElem::getCntr();
    if (this->getStats()->isDying())
    {
        base = (int)(bElem::getCntr() - this->getStats()->getKillTimeBeg());
    }
    if (this->getStats()->isDestroying())
    {
        base = (int)(bElem::getCntr() - this->getStats()->getDestTimeBeg());
    }
    if (this->getStats()->isTeleporting())
    {
        base = (int)(bElem::getCntr() - this->getStats()->getTelReqTime());
    }
    if (this->getStats()->isFadingIn())
    {
        base = (int)(bElem::getCntr() - this->getStats()->getFadingInReq());
    }
    if (this->getStats()->isFadingOut())
    {
        base = (int)(bElem::getCntr() - this->getStats()->getFadingOutReq());
    }
    return base >> 3;
}

float bElem::getViewRadius() const
{

    return 2.5;

}

bool bElem::hurt(int points)
{
    if (!this->getAttrs()->isKillable() || this->getStats()->isDying() || this->getStats()->isDestroying()  || this->getStats()->isTeleporting() || this->getStats()->isDisposed())
        return false;
    this->getAttrs()->setEnergy(this->getAttrs()->getEnergy()-points);
    if (this->getAttrs()->getEnergy()<=0)
        this->kill();
    return true;
}



bool bElem::readyToShoot() const
{
    return false;
}


bool bElem::mechanics()
{
    if ((this->getBoard().get() == nullptr || this->getStats()->getMyPosition() == NOCOORDS) && (!this->getStats()->isCollected()))
        return false;
    this->getStats()->setTaterCounter(this->getStats()->getTaterCounter()+1); /// Instances own 'clock'.

    if (this->getStats()->isWaiting() || this->getStats()->isTeleporting() || this->getStats()->isDying() || this->getStats()->isDestroying() || this->getStats()->isMoving() || this->getStats()->isFadingIn() || this->getStats()->isFadingOut() || (this->getAttrs()->isInteractive() && this->getStats()->isInteracting()))
        return false;

    return true;
}

bool bElem::isSteppableInMyDirection() const
{
    return this->isSteppableDirection(this->getStats()->getMyDirection());
}

bool bElem::isSteppableDirection(coords di) const
{
    coords crd=this->getAbsCoords(di);
    if (this->attachedBoard.expired() || crd==NOCOORDS || this->getStats()->getMyPosition()==NOCOORDS)
        return false;
   return (this->attachedBoard.lock()->getElement(crd)) &&
          this->attachedBoard.lock()->getElement(crd)->getAttrs()->isSteppable();
}
bool bElem::isSteppableDirection(dir::direction di) const
{
    return this->isSteppableDirection(dir::dirToCoords(di));
}

/**
 * @brief removes element from its board, or inventory
 *
 * This method check, if the instance is attached to a board or is in an inventory,
 * is so it will act accordingly, if collected, removed from the inventory.
 * If on board, removed, with respect to object stacking
 * @param none
 *
 * @note This method will create a floor element, if the last stacking element is removed from the board.
 */
std::shared_ptr<bElem> bElem::removeElement()
{
    coords _pos=this->getStats()->getMyPosition();
    std::shared_ptr<chamber> _chmbr=this->getBoard();
    if(this->getStats()->isDisposed())
        return nullptr;

    if(this->getStats()->isCollected())
    {
        std::shared_ptr<bElem> collector=this->getStats()->getCollector().lock();
        collector->getAttrs()->getInventory()->removeCollectibleFromInventory(this->getStats()->getInstanceId());
        return shared_from_this();
    }
    if (this->getStats()->getMyPosition()==NOCOORDS  || !_chmbr)
    {
        return shared_from_this(); // it is not yet placed on a board.
    }

    if(this->getStats()->hasParent())
    {
        std::shared_ptr<bElem> p=this->getStats()->getStandingOn().lock();
        p->getStats()->setSteppingOn(this->getStats()->getSteppingOn());
        if(this->getStats()->getSteppingOn())
            this->getStats()->getSteppingOn()->getStats()->setStandingOn(p);
    }
    else
    {
        std::shared_ptr<bElem> _Stp=this->getStats()->getSteppingOn();
        _chmbr->setElement(_pos,_Stp);
        if(_Stp)
        {
            _Stp->getStats()->setHasParent(false); /// this is how we do "unstomp" now.
        }
        else  /// This rather should not happen, but we fix the situation, when we remove the last element, and a null is created, we create a new floor element.
        {
            std::shared_ptr<bElem> nf=elementFactory::generateAnElement<floorElement>(_chmbr,555);
            nf->getStats()->setMyPosition(_pos);
            _chmbr->setElement(_pos,nf);
        }
    }
    this->setBoard(nullptr);
    this->getStats()->setMyPosition(NOCOORDS);
    return shared_from_this();
}



// Collect another element. The collectible contains location information. that way,
// we restore the element below the collectible, and the collectible will be stored in a vector structure.
// but for now it does nothing
bool bElem::collect(std::shared_ptr<bElem> collectible)
{
    std::shared_ptr<bElem> collected;
    if (collectible.get() == nullptr || !collectible->getAttrs()->isCollectible() || !this->getAttrs()->canCollect() || collectible->getStats()->isDying() || collectible->getStats()->isTeleporting() || collectible->getStats()->isDestroying())
    {
        return false;
    }

    collected = collectible->removeElement();
    if (collected.get() == nullptr) // this should never happen!
    {
        std::cout << "Collecting failed, removed null?\n";
        return false;
    }
#ifdef _VerbousMode_
    std::cout << "Collect " << collected->getType() << " st: " << collected->getAttrs()->getSubtype() << "\n";
#endif
    collectible->getStats()->setCollector(shared_from_this());
    this->getAttrs()->getInventory()->addToInventory(collectible);
    collectible->collectOnAction(true,shared_from_this());
    this->getStats()->setPoints(COLLECTS,this->getStats()->getPoints(COLLECTS)+1);
#ifdef _VerbousMode_
    std::cout << "Collected set? " << (collectible->getStats()->isCollected()) << "\n";
#endif

    return true;
}





bool bElem::kill()
{
    if (!this->getAttrs()->isKillable() || this->getStats()->isDying() || this->getStats()->isDestroying() || this->getStats()->isTeleporting())
    {
        return false;
    }
    if(this->getAttrs()->isKillable())
    {
        // viewPoint::get_instance()->addViewPoint(shared_from_this());
        bElem::toDispose.push_back(shared_from_this());
    }
    this->getStats()->setKilled(_defaultKillTime);
    return true;
}
std::shared_ptr<bElemAttr> bElem::getAttrs() const
{
    return this->attrs;
}

std::shared_ptr<bElemStats> bElem::getStats() const
{
    return this->status;
}





bool bElem::additionalProvisioning(int subtype, std::shared_ptr<bElem>sbe)
{
    bool r=false;
    std::call_once(this->_provOnce,[&]()
    {
        this->attrs=std::make_shared<bElemAttr>(shared_from_this(),this->getType(),subtype);
        r=true;
    });
    return  r;
}

int bElem::getTypeInDirection(dir::direction di)
{
    std::shared_ptr<bElem> e = this->getElementInDirection(di);
    if (e.get() != nullptr)
        return e->getType();
    return -1;
}
int bElem::getType() const
{

    return _belemType;

}

/*
Here we want to avoid the duplication of boundary checking, that is why we use getABSCoords, isSteppableInDirection and getElementInDirection;
*/
sNeighboorhood bElem::getSteppableNeighboorhood()
{
    sNeighboorhood myNeigh;
    coords up, left, down, right;

    for (int c = 0; c < 8; c += 2)
    {
        int c1 = c / 2;
        dir::direction d = (dir::direction)(c1 % 4);
        dir::direction d1 = (dir::direction)((c1 + 1) % 4);
        std::shared_ptr<bElem> e = this->getElementInDirection(d);

        if (e)
        {
            auto e1 = e->getElementInDirection(d1);
            myNeigh.nTypes[c] = e->getType();
            myNeigh.steppable[c] = e->getAttrs()->isSteppable();
            myNeigh.steppableClose[c / 2] = myNeigh.steppable[c];
            if (e1)
            {
                myNeigh.nTypes[c + 1] = e1->getType();
                myNeigh.steppable[c + 1] = e1->getAttrs()->isSteppable();
            }
            else
            {
                myNeigh.nTypes[c + 1] = -1;
                myNeigh.steppable[c + 1] = false;
            }
        }
        else
        {
            myNeigh.nTypes[c + 1] = -1;
            myNeigh.steppable[c + 1] = false;
            myNeigh.nTypes[c] = -1;
            myNeigh.steppable[c] = false;
            myNeigh.steppableClose[c / 2] = myNeigh.steppable[c];
        }
    }
    return myNeigh;
}

bool bElem::moveInDirectionSpeed(dir::direction dir, int speed)
{
    std::shared_ptr<bElem> stepOn=this->getElementInDirection(dir);
    if (stepOn.get()==nullptr || this->getStats()->isMoving() || this->getStats()->isDying() || this->getStats()->isTeleporting() || this->getStats()->isDestroying() || dir==dir::direction::NODIRECTION)
        return false;
    std::shared_ptr<bElem> stepOn2=stepOn->getElementInDirection(dir);
    this->getStats()->setMyDirection(dir);
    if (stepOn->getAttrs()->isSteppable()==true)
    {
        this->stepOnElement(stepOn);
        this->getStats()->setMoved(speed);
        this->playSound("Move","StepOn");
        return true;
    }
    else if (this->getAttrs()->canCollect() && stepOn->getAttrs()->isCollectible()==true && this->collect(stepOn))
    {
        return true;
    }
    else if (this->getAttrs()->canPush()==true && stepOn->getAttrs()->canBePushed()==true && stepOn->getAttrs()->isMovable()==true && stepOn2 && stepOn2->getAttrs()->isSteppable() && stepOn->moveInDirectionSpeed(dir,speed+1))
    {

        this->stepOnElement(this->getElementInDirection(dir));  // move the initiating object
        this->getStats()->setMoved(speed+1);
        this->playSound("Move","StepOn");
        return true;
    }
    else  if (this->getAttrs()->isInteractive()==true && stepOn->interact(shared_from_this()))
    {
        return true;
    }
    return false;
}
bool bElem::moveInDirection(dir::direction d)
{
    return this->moveInDirectionSpeed(d,_mov_delay);
}
bool bElem::dragInDirection(dir::direction dragIntoDirection)
{
    return this->dragInDirectionSpeed(dragIntoDirection,_mov_delay*2);
}

bool bElem::dragInDirectionSpeed(dir::direction dragIntoDirection, int speed)
{
    dir::direction objFromDir=(dir::direction)((((int)dragIntoDirection)+2)%4);
    dir::direction d2=dragIntoDirection;
    std::shared_ptr<bElem> draggedObj=this->getElementInDirection(objFromDir);
    if(draggedObj.get()==nullptr)
        return false;
    if(!draggedObj->getAttrs()->isMovable())
    {
        d2=(dir::direction)((((int)this->getStats()->getMyDirection())+2)%4);
        draggedObj=this->getElementInDirection(d2);
        d2=this->getStats()->getMyDirection();
        if(draggedObj.get()==nullptr || !draggedObj->getAttrs()->isMovable())
            return false;
    }

    this->moveInDirectionSpeed(dragIntoDirection,speed);
    return draggedObj->moveInDirectionSpeed(d2,speed);

}


void bElem::registerLiveElement(std::shared_ptr<bElem> who)
{
    if (who->getStats()->hasActivatedMechanics())
        return;
    who->getStats()->setActivatedMechanics(true);
    if(who->getBoard())
        who->getBoard()->registerLiveElem(who);

}

void bElem::deregisterLiveElement(int instanceId)
{

    if(this->getStats()->hasActivatedMechanics() && this->getBoard())
    {
        this->getBoard()->toDeregister.push_back(instanceId);
        this->getStats()->setActivatedMechanics(false);
    }
}

void bElem::mechLock()
{
    bElem::mechanicMutex.lock();
}

void bElem::mechUnlock()
{
    bElem::mechanicMutex.unlock();
}

void bElem::runLiveElements()
{
    bElem::tick();
    std::shared_ptr<bElem> ap=player::getActivePlayer();
    std::shared_ptr<chamber> cchmbr=(player::getActivePlayer())?player::getActivePlayer()->getBoard():nullptr;
    std::vector<std::shared_ptr<bElem>>::iterator p;
    /// No active player, no game
    if(!cchmbr)
        return;
    // We will check the elements, that are dying, and chek, if should we remove the ones, that are stale.
    for(unsigned int  c=0 ; c<bElem::toDispose.size();)
    {
        if(!bElem::toDispose[c]->getStats()->isDying() && !bElem::toDispose[c]->getStats()->isDestroying() && !bElem::toDispose[c]->getStats()->isTeleporting())
        {
            if(!bElem::toDispose[c]->getStats()->isDisposed())
            {
                bElem::toDispose[c]->disposeElement(); // we remove the element, which stopped being dead - its time has passed.
            }
            bElem::toDispose.erase(bElem::toDispose.begin()+c);
        }
        else c++;
    }
    for (unsigned long int c=0; c<cchmbr->toDeregister.size();)
    {
        auto instId=cchmbr->toDeregister[c];
        auto it = std::ranges::find_if(cchmbr->liveElems.begin(), cchmbr->liveElems.end(), [instId](std::shared_ptr<bElem> elem)
        {
            return elem && (elem->getStats()->getInstanceId() == instId);
        });

        if (it != cchmbr->liveElems.end())
        {
            cchmbr->liveElems.erase(it);
            cchmbr->toDeregister.erase(cchmbr->toDeregister.begin()+c);
        }
        else c++;
    }
    cchmbr->toDeregister.clear();
    for (unsigned int p = 0; p < cchmbr->liveElems.size();)
    {
        if(cchmbr->liveElems[p]->getStats()->isDisposed() || cchmbr->liveElems[p]->getType()==_player)
            cchmbr->liveElems.erase(cchmbr->liveElems.begin()+p);
        else
        {
            cchmbr->liveElems[p]->mechanics();
            if(cchmbr->liveElems[p]->getAttrs()->canCollect())
                cchmbr->liveElems[p]->getAttrs()->getInventory()->runLives();
            p++;
        }


    }
    ap->mechanics();
    if(ap->getAttrs()->canCollect())
        ap->getAttrs()->getInventory()->runLives();
    return;



}

/**
 * @brief performs the action, when an element steps on this
 *
 * This method is just a stub, at this type does absolutely nothing
 * @param step
 * true - stepping on, false - unstepping
 * @param who
 * who steps/unsteps
 *
 * @note This should be used in stepOn methods.
 */
bool bElem::stepOnAction(bool step, std::shared_ptr<bElem>who)
{
    return false;
}



void bElem::tick()
{
    bElem::sTaterCounter++;
}

unsigned int bElem::getCntr()
{
    return bElem::sTaterCounter;
}



bool bElem::isLocked()
{
    return this->lockers.size() != 0;
}

bool bElem::lockThisObject(std::shared_ptr<bElem> who)
{
    this->lockers.push_back(who);
    return true;
}

bool bElem::unlockThisObject(std::shared_ptr<bElem> who)
{
    static ALLEGRO_MUTEX *elementMutex=al_create_mutex_recursive();
    al_lock_mutex(elementMutex);
    for (unsigned int cnt = 0; cnt < this->lockers.size();)
    {
        if (!this->lockers.at(cnt) || this->lockers.at(cnt)->getStats()->getInstanceId() == who->getStats()->getInstanceId())
        {
            this->lockers.erase(this->lockers.begin() + cnt);
        }
        else
        {
            cnt++;
        }
    }
    al_unlock_mutex(elementMutex);
    return true;
}


void bElem::setStatsOwner(std::shared_ptr<bElem> owner)
{
    static ALLEGRO_MUTEX *SEMutex=al_create_mutex_recursive();
    al_lock_mutex(SEMutex);
    this->getStats()->setStatsOwner(owner);
    al_unlock_mutex(SEMutex);
}


void bElem::playSound(std::string eventType, std::string event)
{
    if (!this->getStats()->getCollector().expired())
    {
        this->ps(this->getStats()->getCollector().lock(),eventType,event);
    }
    else if(this->getBoard())
    {
        this->ps(shared_from_this(),eventType,event);
    }
    else if (this->getStats()->hasParent())
    {
        this->ps(this->getStats()->getStandingOn().lock(),eventType,event);
    }
    else if (this->getStats()->getSteppingOn())
    {
        this->ps(this->getStats()->getSteppingOn(),eventType,event);
    }
}

void bElem::ps(std::shared_ptr<bElem>who, std::string eventType, std::string event)
{
    coords3d c3d;
    c3d.x=who->getStats()->getMyPosition().x*32+who->getOffset().x;
    c3d.z=who->getStats()->getMyPosition().y*32+who->getOffset().y;
    c3d.y=0;
    coords3d vel= {(who->getOffset().x)?32:0, 0,(who->getOffset().y>0)?32:0};
    if(!who || !who->getBoard()) return;
    soundManager::getInstance()->registerSound(who->getBoard()->getInstanceId(),c3d,vel,this->getStats()->getInstanceId(),this->getType(),this->getAttrs()->getSubtype(),eventType,event);
}

void bElem::stopMySounds()
{
    soundManager::getInstance()->stopSoundsByElementId(this->getStats()->getInstanceId());
}









