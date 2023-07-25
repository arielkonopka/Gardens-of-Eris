#include "../include/bElem.h"
#include "floorElement.h"
#include "rubbish.h"
#include "elements.h"

videoElement::videoElementDef *bElem::vd = nullptr;
std::vector<std::shared_ptr<bElem>> bElem::liveElems;
std::vector<int> bElem::toDeregister;
unsigned int bElem::sTaterCounter = 5;
int bElem::instances = 0;
bool bElem::randomNumberGeneratorInitialized = false;
std::mt19937 bElem::randomNumberGenerator;


bElem::bElem() : std::enable_shared_from_this<bElem>(), elementMutex(al_create_mutex())
{

    this->status=std::make_unique<bElemStats>();

    this->status->setMyDirection(UP);
    this->status->setFacing(UP);
    if (!bElem::randomNumberGeneratorInitialized)
    {
        std::random_device rd;
        std::array<int,4> seedData;
        std::generate_n(seedData.data(), seedData.size(), std::ref(rd));
        std::seed_seq seq(std::begin(seedData), std::end(seedData));
        bElem::randomNumberGenerator.seed(seq);
        bElem::randomNumberGeneratorInitialized=true;
    }
}

bElem::bElem(std::shared_ptr<chamber> board) : bElem()
{
//    this->attachedBoard = board;
    this->setBoard(board);
}



coords bElem::getOffset() const
{
    return {0, 0};
}


bool bElem::collectOnAction(bool collected, std::shared_ptr<bElem>who)
{
    return false;
}




std::shared_ptr<chamber> bElem::getBoard() const
{
    return this->attachedBoard;
}

void bElem::setBoard(std::shared_ptr<chamber> board)
{
    this->attachedBoard = board;
    if (this->provisioned && this->attrs->canCollect())
    {
        this->attrs->getInventory()->updateBoard();
    }
}

bool bElem::dropItem(unsigned long int  instanceId)
{
    if(!this->attrs->canCollect())
        return false;
    std::shared_ptr<bElem> item=this->attrs->getInventory()->retrieveCollectibleFromInventory(instanceId);
    if(!item)
        return false;
    item->collectOnAction(false,shared_from_this());
    for(int c=0;c<4;c++)
    {
        if(this->isSteppableDirection((direction)(c)))
            {
                item->stepOnElement(this->getElementInDirection((direction)(c)));
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
//std::cout<<"SO."<<step->getType()<<","<<this->getType()<<",";
    auto elig = [](std::shared_ptr<bElem> step) -> bool
    {
        if (!step || !step->attrs->isSteppable() || !step->getBoard() || step->status->isDisposed() || step->status->getMyPosition() == NOCOORDS)
            return false;
        else
            return true;
    };
    // std::cout<<" * QualifiedToStep?\n";
    if(this->status->isDisposed())
        return false;
    if(!elig(step))
        return false;


    if(step->attrs->isCollectible() && this->attrs->canCollect())
    {
        //    std::cout<<"  ** isCollectible yes\n";
        std::shared_ptr<bElem> s2=step->status->getSteppingOn();
        if(!elig(s2))
            return false;
        this->collect(step);
        step=s2;
    };
    std::shared_ptr<bElem> myself=shared_from_this();
    if(this->status->getSteppingOn() || this->status->hasParent() || this->status->isCollected())
    {
        std::shared_ptr<bElem> st=this->status->getSteppingOn();
        myself=this->removeElement();
        if(st)
            st->stepOnAction(false,myself);
    }
    std::shared_ptr<bElem> s0;
    bool hp=step->status->hasParent();
    this->setBoard(step->getBoard());
    this->status->setMyPosition(step->status->getMyPosition());
    this->status->setSteppingOn(step);
    s0=step->status->getStandingOn().lock();
    step->status->setStandingOn(shared_from_this());
    if(hp)
    {
        s0->status->setSteppingOn(myself);
        this->status->setStandingOn(s0);
    }
    else
    {
        //std::cout<<"   ** no\n";
        this->getBoard()->setElement(this->status->getMyPosition(),shared_from_this());
    }
    step->stepOnAction(true,shared_from_this());
    return true;
}

oState bElem::disposeElementUnsafe()
{
    oState res = DISPOSED;
    std::shared_ptr<chamber> myBoard = this->getBoard();
    coords mycoords = this->status->getMyPosition();
    if (this->status->isDisposed() == true)
        return ERROR;
    if (this->status->hasActivatedMechanics())
        this->deregisterLiveElement(this->status->getInstanceId());

    this->status->setDisposed(true);
    if (mycoords.x >= 0 && mycoords.y >= 0 && this->getBoard() != nullptr) // object on a board? need extra steps
    {
        if (this->status->getSteppingOn() || this->status->hasParent())
        {
            this->removeElement();
            res = DISPOSED;
        }
        else
        {
            this->getBoard()->setElement(this->status->getMyPosition(), nullptr);
            res = nullptrREACHED;
        }

        if (this->getType() != _stash && this->attrs->canCollect() && this->attrs->getInventory()->isEmpty() == false && this->getType() != _rubishType && this->getType() != _plainMissile && this->getType() != _plainGun)
        {
            std::shared_ptr<bElem> stash = elementFactory::generateAnElement<rubbish>(myBoard,0);
            stash->attrs->setInventory(this->attrs->getInventory());
            stash->attrs->getInventory()->changeOwner(stash);
            this->attrs->setInventory(nullptr);
            if (myBoard->getElement(mycoords)->attrs->isSteppable())
            {
                stash->stepOnElement(myBoard->getElement(mycoords));
            }
            else
            {
                bool stashed = false;
                for (int c = 0; c < 4; c++)
                {
                    if (myBoard->getElement(mycoords)->isSteppableDirection((direction)c))
                    {
                        stash->stepOnElement(myBoard->getElement(mycoords)->getElementInDirection((direction)c));
                        stashed = true;
                        break;
                    }
                }
                if (!stashed)
                    stash->disposeElement(); // no place for the stash? Burn!
            }
        }
    }
    this->status->setDisposed(true);
    this->status->setMyPosition(NOCOORDS);
    this->attachedBoard = nullptr;
    return res; // false means that there is no more elements to go.
}

oState bElem::disposeElement()
{
    std::shared_ptr<bElem> t = shared_from_this();
    std::shared_ptr<bElem> stash = nullptr;
    std::shared_ptr<chamber> _myBoard=this->getBoard();
    coords oCoords = this->status->getMyPosition();
    if (this->status->isDisposed() )
    {
        return ERROR;
    }
    this->removeElement();
    if (this->status->hasActivatedMechanics())
    {
        this->deregisterLiveElement(this->status->getInstanceId());
    }
    if(this->attrs->canCollect())
    {
        if (this->getType() == _rubishType )
        {
            this->attrs->getInventory()->weapons.clear();
            this->attrs->getInventory()->keys.clear();
            this->attrs->getInventory()->tokens.clear();
        }
        else if ( oCoords != NOCOORDS && !this->attrs->getInventory()->isEmpty() && _myBoard)
        {
            /*
                Create a rubbish element on the board, so the inventory would not be lost
             */
            stash = elementFactory::generateAnElement<rubbish>(this->getBoard(),0);
            stash->attrs->setCollect(true);
            stash->attrs->getInventory()->mergeInventory(this->attrs->getInventory());
            stash->stepOnElement(_myBoard->getElement(oCoords));
        }
    }
    this->status->setDisposed(true);
    this->setBoard(nullptr);
    this->status->setMyPosition(NOCOORDS);
    return DISPOSED;
}

/*
 * This method returns absolute coordinates, when asked for coordinates of the next cell in a direction from the elements's standpoint, should be resistant for non-provisioned units
 * better use those on a board though
 */
coords bElem::getAbsCoords(direction dir) const
{
    coords res = this->status->getMyPosition();
    if (this->attachedBoard.get() == nullptr)
        return NOCOORDS;
    switch (dir)
    {
    case UP:
        res.y--;
        break;
    case DOWN:
        res.y++;
        break;
    case LEFT:
        res.x--;
        break;
    case RIGHT:
        res.x++;
        break;
    case NODIRECTION:
        break;
    }
    if (res.y >= this->attachedBoard->height || res.y < 0 || res.x >= this->attachedBoard->width || res.x < 0)
    {
        return NOCOORDS;
    }
    return res;
}

std::shared_ptr<bElem> bElem::getElementInDirection(direction di)
{
    coords mycoords = this->getAbsCoords(di);
    if (mycoords == NOCOORDS)
        return nullptr;
    return this->attachedBoard->getElement(mycoords.x, mycoords.y);
}

bElem::~bElem()
{
    al_destroy_mutex(this->elementMutex);
}

ALLEGRO_MUTEX *bElem::getMyMutex()
{
    return this->elementMutex;
}


/*
std::shared_ptr<elemStats> bElem::getStats()
{
    return this->eConfig.myStats;
}

void bElem::setStats(std::shared_ptr<elemStats> stat)
{
    this->eConfig.myStats = stat;
}
*/

bool bElem::moveInDirection(direction d)
{
    return false;
}

bool bElem::use(std::shared_ptr<bElem> who)
{
    return false;
}

bool bElem::interact(std::shared_ptr<bElem> who)
{
    if (this->attrs->canInteract()) /* penalty for getting into counter overflow */
    {
        this->status->setInteracted( _interactedTime);
        return true;
    }
    return false;
}

bool bElem::destroy()
{
//   std::cout<<" * Destroy:"<<this->getType()<<"\n";
    if (this->attrs->isDestroyable() || this->attrs->isSteppable() || this->status->isDestroying() || this->status->isDying())
    {
        //  std::cout<<"  ** yup.\n";
        if (this->status->isDying())
        {
            this->status->setKilled(0);
            this->status->setKillTimeBeg(0);
            this->status->setKillTimeReq(0);

        }
        if (this->attrs->isDestroyable() || this->attrs->isKillable())
            this->registerLiveElement(shared_from_this());
        this->status->setDestroyed(_defaultDestroyTime);
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
    if (this->status->isDying())
    {
        base = (int)(bElem::getCntr() - this->status->getKillTimeBeg());
    }
    if (this->status->isDestroying())
    {
        base = (int)(bElem::getCntr() - this->status->getDestTimeBeg());
    }
    if (this->status->isTeleporting())
    {
        base = (int)(bElem::getCntr() - this->status->getTelReqTime());
    }
    return base >> 3;
}

float bElem::getViewRadius() const
{

    return 2.5;

}
bool bElem::hurt(int points)
{

    return false;

}

bool bElem::readyToShoot() const
{
    return false;
}


bool bElem::mechanics()
{
    if ((this->getBoard().get() == nullptr || this->status->getMyPosition() == NOCOORDS) && (!this->status->isCollected()))
        return false;
    this->status->setTaterCounter(this->status->getTaterCounter()+1); /// Instances own 'clock'.
    if ( (this->attrs->isDestroyable() && this->status->getDestroyed()==0) ||
            (this->attrs->isKillable() && this->status->getKilled()==0))
        /// isDestroyed returns -1, if the destroying process is in the past, and 0 is the exact time of the object to be gone.
    {
        std::cout<<"dispose\n";
        this->disposeElement();
        return false;
    }
    if (this->status->isWaiting() || this->status->isTeleporting() || this->status->isDying() || this->status->isDestroying() || this->status->isMoving())
        return false;

    return true;
}





bool bElem::isSteppableDirection(direction di) const
{
    coords tmpcoords;
    tmpcoords = this->getAbsCoords(di);
    if (!(tmpcoords == NOCOORDS) && this->attachedBoard->getElement(tmpcoords).get() != nullptr)
    {
        return this->attachedBoard->getElement(tmpcoords)->attrs->isSteppable();
    }
    return false;
}

videoElement::videoElementDef *bElem::getVideoElementDef()
{
    return bElem::vd;
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
#ifdef _VerbousMode_
    std::cout<<" * isDisposed?\n";
#endif
    if(this->status->isDisposed())
        return nullptr;
#ifdef _VerbousMode_
    std::cout<<" * isCollected?\n";
#endif

    if(this->status->isCollected())
    {
        std::shared_ptr<bElem> collector=this->status->getCollector().lock();
        collector->attrs->getInventory()->removeCollectibleFromInventory(this->status->getInstanceId());
        return shared_from_this();
    }
#ifdef _VerbousMode_
    std::cout<<" * has position and board, newly created?\n";
#endif
    if (this->status->getMyPosition()==NOCOORDS  || !this->getBoard())
    {
        return shared_from_this(); // it is not yet placed on a board.
    }
#ifdef _VerbousMode_
    std::cout<<" * remove\n";
#endif
    coords _pos=this->status->getMyPosition();
    std::shared_ptr<chamber> _chmbr=this->getBoard();
#ifdef _VerbousMode_
    std::cout<<" * has parent\n";
#endif
    if(this->status->hasParent())
    {
#ifdef _VerbousMode_
        std::cout<<"  ** yes\n";
#endif
        std::shared_ptr<bElem> p=this->status->getStandingOn().lock();
        p->status->setSteppingOn(this->status->getSteppingOn());
        if(this->status->getSteppingOn())
            this->status->getSteppingOn()->status->setStandingOn(p);
    }
    else
    {
#ifdef _VerbousMode_
        std::cout<<"  ** no\n";
#endif
        std::shared_ptr<bElem> _Stp=this->status->getSteppingOn();
        this->getBoard()->setElement(this->status->getMyPosition(),_Stp);
        //_Stp->stepOnAction(false,nullptr);
        if(_Stp)
            _Stp->status->setHasParent(false); /// this is how we do "unstomp" now.
    }
#ifdef _VerbousMode_
    std::cout<<" * check if we removed the last element on a board\n";
#endif

    if(_chmbr->getElement(_pos).get()==nullptr) /// This rather should not happen, but we fix the situation, when we remove the last element, and a null is created, we create a new floor element.
    {
#ifdef _VerbousMode_
        std::cout<<"  ** yes we did\n";
#endif
        std::shared_ptr<bElem> nf=elementFactory::generateAnElement<floorElement>(_chmbr,1);
        nf->status->setMyPosition(_pos);
        _chmbr->setElement(_pos,nf);
    }
    this->setBoard(nullptr);
    this->status->setMyPosition(NOCOORDS);
    return shared_from_this();
}



// Collect another element. The collectible contains location information. that way,
// we restore the element below the collectible, and the collectible will be stored in a vector structure.
// but for now it does nothing
bool bElem::collect(std::shared_ptr<bElem> collectible)
{
    std::shared_ptr<bElem> collected;
    if (collectible.get() == nullptr || !collectible->attrs->isCollectible() || !this->attrs->canCollect() || collectible->status->isDying() || collectible->status->isTeleporting() || collectible->status->isDestroying())
    {
        return false;
    }

    collected = collectible->removeElement();
    if (collected.get() == nullptr) // this should never happen!
    {
        std::cout << "Collecting failed!\n";
        return false;
    }
#ifdef _VerbousMode_
    std::cout << "Collect " << collected->getType() << " st: " << collected->attrs->getSubtype() << "\n";
#endif
    collectible->status->setCollector(shared_from_this());
    collectible->collectOnAction(true,shared_from_this());
#ifdef _VerbousMode_
    std::cout << "Collected set? " << (collectible->status->isCollected()) << "\n";
#endif
    this->attrs->getInventory()->addToInventory(collectible);
    return true;
}





bool bElem::kill()
{
    if (!this->attrs->isKillable() || this->status->isDying() || this->status->isDestroying() || this->status->isTeleporting())
    {
        return false;
    }
    if (!this->status->hasActivatedMechanics())
    {
        this->registerLiveElement(shared_from_this());
    }
    this->status->setKilled(_defaultKillTime);
    return true;
}


bool bElem::additionalProvisioning(int subtype,int  typeId)
{
    this->attrs=std::make_unique<bElemAttr>(shared_from_this(),typeId,subtype);
    return  true;
}

bool bElem::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool bElem::additionalProvisioning(int subtype, std::shared_ptr<bElem>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

int bElem::getTypeInDirection(direction di)
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
        direction d = (direction)(c1 % 4);
        direction d1 = (direction)((c1 + 1) % 4);
        std::shared_ptr<bElem> e = this->getElementInDirection(d);
        std::shared_ptr<bElem> e1 = nullptr;
        if (e)
        {
            e1 = e->getElementInDirection(d1);
            myNeigh.nTypes[c] = e->getType();
            myNeigh.steppable[c] = e->attrs->isSteppable();
            myNeigh.steppableClose[c / 2] = myNeigh.steppable[c];
            if (e1)
            {
                myNeigh.nTypes[c + 1] = e1->getType();
                myNeigh.steppable[c + 1] = e1->attrs->isSteppable();
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

bool bElem::moveInDirectionSpeed(direction d, int speed)
{
    return false;
}



void bElem::registerLiveElement(std::shared_ptr<bElem> who)
{
#ifdef _VerbousMode_
    std::cout << "Register mechanics by: " << who->getType() << " " << who->status->getInstanceId() << "RL\n";
#endif
    if (this->status->hasActivatedMechanics())
        return;
    this->status->setActivatedMechanics(true);
    bElem::liveElems.push_back(who);
}

void bElem::deregisterLiveElement(int instanceId)
{
    toDeregister.push_back(instanceId);
    this->status->setActivatedMechanics(false);
}

void bElem::runLiveElements()
{
    bElem::tick();
    std::vector<std::shared_ptr<bElem>>::iterator p;
    // First we remove everything that needs to be deregistered
    for(unsigned long int instId: bElem::toDeregister)
    {
        for (p = bElem::liveElems.begin(); p != bElem::liveElems.end();)
        {
            if (instId == (*p)->status->getInstanceId())
            {
                bElem::liveElems.erase(p);
            }
            else
            {
                p++;
            }
        }
    }
    bElem::toDeregister.clear();
    bool gotPlayer=player::getActivePlayer()!=nullptr;
    if(!gotPlayer || (gotPlayer && !player::getActivePlayer()->getBoard())) // No active player? No animation!
    {
        for (unsigned int p = 0; p < bElem::liveElems.size(); p++)
        {
            //  if(bElem::liveElems[p] && bElem::randomNumberGenerator()%55==5)
            bElem::liveElems[p]->mechanics();

        }
        return;
    }
    int chamberId=player::getActivePlayer()->getBoard()->getInstanceId();
    for (unsigned int p = 0; p < bElem::liveElems.size(); p++)
    {
        if(bElem::liveElems[p].get()!=nullptr)
        {
            if (bElem::liveElems[p]->status->getMyPosition() != NOCOORDS && bElem::liveElems[p]->getBoard() && bElem::liveElems[p]->getBoard()->getInstanceId()==chamberId)
            {
                bElem::liveElems[p]->mechanics();
            }
            else if (bElem::liveElems[p]->status->isCollected() && bElem::liveElems[p]->status->getCollector().lock()->getBoard()->getInstanceId()==chamberId)
            {
                bElem::liveElems[p]->mechanics();
            }
            else if (bElem::randomNumberGenerator()%55==5)
                bElem::liveElems[p]->mechanics(); /// once in a while all objects will be moving
        }
    }

    //  if (gCollect::getInstance()->garbageQsize()>0) //clean up, when there is garbage to be cleared
    //      gCollect::getInstance()->purgeGarbage();
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
    for (unsigned int cnt = 0; cnt < this->lockers.size();)
    {
        if (this->lockers.at(cnt)->status->getInstanceId() == who->status->getInstanceId())
        {
            this->lockers.erase(this->lockers.begin() + cnt);
        }
        else
        {
            cnt++;
        }
    }
    return true;
}


void bElem::setStatsOwner(std::shared_ptr<bElem> owner)
{
}
