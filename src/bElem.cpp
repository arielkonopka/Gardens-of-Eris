#include "../include/bElem.h"
#include "floorElement.h"
#include "rubbish.h"
#include "elements.h"

std::vector<std::shared_ptr<bElem>> bElem::liveElems;
std::vector<std::shared_ptr<bElem>> bElem::toDispose;
std::vector<int> bElem::toDeregister;
unsigned int bElem::sTaterCounter = 5;
int bElem::instances = 0;
bool bElem::randomNumberGeneratorInitialized = false;
std::mt19937 bElem::randomNumberGenerator;


bElem::bElem() : std::enable_shared_from_this<bElem>(), elementMutex(al_create_mutex())
{

    this->status=std::make_shared<bElemStats>();

    this->getStats()->setMyDirection(UP);
    this->getStats()->setFacing(UP);
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
    if (collected && who && who->getType()==_player)
    {
        this->playSound("Found","Collect");
    }
    return true;
}




std::shared_ptr<chamber> bElem::getBoard() const
{
    return this->attachedBoard;
}

void bElem::setBoard(std::shared_ptr<chamber> board)
{
    this->attachedBoard = board;
    if (this->provisioned && this->getAttrs()->canCollect())
    {
        this->getAttrs()->getInventory()->updateBoard();
    }
}

bool bElem::dropItem(unsigned long int  instanceId)
{
    if(!this->getAttrs()->canCollect())
        return false;
    std::shared_ptr<bElem> item=this->getAttrs()->getInventory()->retrieveCollectibleFromInventory(instanceId);
    if(!item)
        return false;
    item->collectOnAction(false,shared_from_this());
    for(int c=0; c<4; c++)
    {
        if(this->isSteppableDirection((direction)(c)))
        {
            if (this->getType()==_player)
                item->playSound("Drop","Item");
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
        if (!step || !step->getAttrs()->isSteppable() || !step->getBoard() || step->getStats()->isDisposed() || step->getStats()->getMyPosition() == NOCOORDS)
            return false;
        else
            return true;
    };
    // std::cout<<" * QualifiedToStep?\n";
    if(this->getStats()->isDisposed())
        return false;
    if(!elig(step))
        return false;


    if(step->getAttrs()->isCollectible() && this->getAttrs()->canCollect())
    {
        //    std::cout<<"  ** isCollectible yes\n";
        std::shared_ptr<bElem> s2=step->getStats()->getSteppingOn();
        if(!elig(s2))
            return false;
        this->collect(step);
        step=s2;

        return (step)?this->stepOnElement(step):false;
    };
    std::shared_ptr<bElem> myself=shared_from_this();
    if(this->getStats()->getSteppingOn() || this->getStats()->hasParent() || this->getStats()->isCollected())
    {
        std::shared_ptr<bElem> st=this->getStats()->getSteppingOn();
        myself=this->removeElement();
        if(st)
            st->stepOnAction(false,myself);
    }
    std::shared_ptr<bElem> s0;
    bool hp=step->getStats()->hasParent();
    this->setBoard(step->getBoard());
    this->getStats()->setMyPosition(step->getStats()->getMyPosition());
    this->getStats()->setSteppingOn(step);
    s0=step->getStats()->getStandingOn().lock();
    step->getStats()->setStandingOn(shared_from_this());
    if(hp)
    {
        s0->getStats()->setSteppingOn(myself);
        this->getStats()->setStandingOn(s0);
    }
    else
    {
        //std::cout<<"   ** no\n";
        this->getBoard()->setElement(this->getStats()->getMyPosition(),shared_from_this());
    }
    step->stepOnAction(true,shared_from_this());
    return true;
}

oState bElem::disposeElementUnsafe()
{
    oState res = DISPOSED;
    std::shared_ptr<chamber> myBoard = this->getBoard();
    coords mycoords = this->getStats()->getMyPosition();
    if (this->getStats()->isDisposed() == true)
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

        if (this->getType() != _stash && this->getAttrs()->canCollect() && this->getAttrs()->getInventory()->isEmpty() == false && this->getType() != _rubishType && this->getType() != _plainMissile && this->getType() != _plainGun)
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
    this->getStats()->setDisposed(true);
    this->getStats()->setMyPosition(NOCOORDS);
    this->attachedBoard = nullptr;
    return res; // false means that there is no more elements to go.
}

oState bElem::disposeElement()
{
    std::shared_ptr<bElem> t = shared_from_this();
    std::shared_ptr<bElem> stash = nullptr;
    std::shared_ptr<chamber> _myBoard=this->getBoard();
    coords oCoords = this->getStats()->getMyPosition();
    if (this->getStats()->isDisposed() )
    {
        return ERROR;
    }
    this->removeElement();
    if (this->getStats()->hasActivatedMechanics())
    {
        this->deregisterLiveElement(this->getStats()->getInstanceId());
    }
    if(this->getAttrs()->canCollect())
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
    return DISPOSED;
}

/*
 * This method returns absolute coordinates, when asked for coordinates of the next cell in a direction from the elements's standpoint, should be resistant for non-provisioned units
 * better use those on a board though
 */
coords bElem::getAbsCoords(direction dir) const
{
    coords res = this->getStats()->getMyPosition();
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
    soundManager::getInstance()->stopSoundsByElementId(this->getStats()->getInstanceId());
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
    if (this->getAttrs()->isInteractive() && !this->getStats()->isInteracting()) /* penalty for getting into counter overflow */
    {
        this->getStats()->setInteracted( _interactedTime);
        return true;
    }
    return false;
}

bool bElem::destroy()
{
//   std::cout<<" * Destroy:"<<this->getType()<<"\n";
    if (this->getAttrs()->isDestroyable() || this->getAttrs()->isSteppable() || this->getStats()->isDestroying() || this->getStats()->isDying())
    {
        //  std::cout<<"  ** yup.\n";
        if (this->getStats()->isDying())
        {
            this->getStats()->setKilled(0);
            this->getStats()->setKillTimeBeg(0);
            this->getStats()->setKillTimeReq(0);

        }
        this->getStats()->setDestroyed(_defaultDestroyTime);
        if (this->getAttrs()->isDestroyable() || this->getAttrs()->isKillable())
            bElem::toDispose.push_back(shared_from_this());
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
    if ((this->getBoard().get() == nullptr || this->getStats()->getMyPosition() == NOCOORDS) && (!this->getStats()->isCollected()))
        return false;
    this->getStats()->setTaterCounter(this->getStats()->getTaterCounter()+1); /// Instances own 'clock'.

    if (this->getStats()->isWaiting() || this->getStats()->isTeleporting() || this->getStats()->isDying() || this->getStats()->isDestroying() || this->getStats()->isMoving())
        return false;

    return true;
}





bool bElem::isSteppableDirection(direction di) const
{
    coords tmpcoords;
    tmpcoords = this->getAbsCoords(di);
    if (!(tmpcoords == NOCOORDS) && this->attachedBoard->getElement(tmpcoords).get() != nullptr)
    {
        return this->attachedBoard->getElement(tmpcoords)->getAttrs()->isSteppable();
    }
    return false;
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
    if(this->getStats()->isDisposed())
        return nullptr;
#ifdef _VerbousMode_
    std::cout<<" * isCollected?\n";
#endif

    if(this->getStats()->isCollected())
    {
        std::shared_ptr<bElem> collector=this->getStats()->getCollector().lock();
        collector->getAttrs()->getInventory()->removeCollectibleFromInventory(this->getStats()->getInstanceId());
        return shared_from_this();
    }
#ifdef _VerbousMode_
    std::cout<<" * has position and board, newly created?\n";
#endif
    if (this->getStats()->getMyPosition()==NOCOORDS  || !this->getBoard())
    {
        return shared_from_this(); // it is not yet placed on a board.
    }
#ifdef _VerbousMode_
    std::cout<<" * remove\n";
#endif
    coords _pos=this->getStats()->getMyPosition();
    std::shared_ptr<chamber> _chmbr=this->getBoard();
#ifdef _VerbousMode_
    std::cout<<" * has parent\n";
#endif
    if(this->getStats()->hasParent())
    {
#ifdef _VerbousMode_
        std::cout<<"  ** yes\n";
#endif
        std::shared_ptr<bElem> p=this->getStats()->getStandingOn().lock();
        p->getStats()->setSteppingOn(this->getStats()->getSteppingOn());
        if(this->getStats()->getSteppingOn())
            this->getStats()->getSteppingOn()->getStats()->setStandingOn(p);
    }
    else
    {
#ifdef _VerbousMode_
        std::cout<<"  ** no\n";
#endif
        std::shared_ptr<bElem> _Stp=this->getStats()->getSteppingOn();
        this->getBoard()->setElement(this->getStats()->getMyPosition(),_Stp);
        //_Stp->stepOnAction(false,nullptr);
        if(_Stp)
            _Stp->getStats()->setHasParent(false); /// this is how we do "unstomp" now.
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
        nf->getStats()->setMyPosition(_pos);
        _chmbr->setElement(_pos,nf);
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


bool bElem::additionalProvisioning(int subtype,int  typeId)
{
    this->attrs=std::make_shared<bElemAttr>(shared_from_this(),typeId,subtype);
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

bool bElem::moveInDirectionSpeed(direction d, int speed)
{
    return false;
}



void bElem::registerLiveElement(std::shared_ptr<bElem> who)
{

    if (this->getStats()->hasActivatedMechanics())
        return;
    this->getStats()->setActivatedMechanics(true);
    bElem::liveElems.push_back(who);
}

void bElem::deregisterLiveElement(int instanceId)
{
    toDeregister.push_back(instanceId);
    this->getStats()->setActivatedMechanics(false);
}

void bElem::runLiveElements()
{
    bElem::tick();
    std::vector<std::shared_ptr<bElem>>::iterator p;
    // We will check the elements, that are dying, and chek, if syoud le remove the ones, that are stale.
    for(unsigned int  c=0 ; c<bElem::toDispose.size();)
    {
        if(!bElem::toDispose[c]->getStats()->isDying() && !bElem::toDispose[c]->getStats()->isDestroying() && !bElem::toDispose[c]->getStats()->isTeleporting())
        {
            if(!bElem::toDispose[c]->getStats()->isDisposed())
            {
              //  bElem::toDispose[c]->playSound("Element","Disposed"); // Snd: Element->Disposed - play sound on element disposal during the game.
                bElem::toDispose[c]->disposeElement(); // we remove the element, which stopped being dead - its time has passed.
            }

            bElem::toDispose.erase(bElem::toDispose.begin()+c);
        }
        else c++;
    }
    // We remove the unregistered elements first, so these would not be executed.
    for(unsigned long int instId: bElem::toDeregister)
    {
        for (p = bElem::liveElems.begin(); p != bElem::liveElems.end();)
        {
            if (instId == (*p)->getStats()->getInstanceId())
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
            std::shared_ptr<bElem> collector=bElem::liveElems[p]->getStats()->getCollector().lock();
            if (bElem::liveElems[p]->getStats()->getMyPosition() != NOCOORDS && bElem::liveElems[p]->getBoard() && bElem::liveElems[p]->getBoard()->getInstanceId()==chamberId)
            {
                bElem::liveElems[p]->mechanics();
            }
            else if (bElem::liveElems[p]->getStats()->isCollected() && collector && collector->getBoard() && collector->getBoard()->getInstanceId()==chamberId)
            {
                bElem::liveElems[p]->mechanics();
            }
           // else if (bElem::randomNumberGenerator()%55==5)
           //     bElem::liveElems[p]->mechanics(); /// once in a while all objects will be moving
        }
    }

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
        if (this->lockers.at(cnt)->getStats()->getInstanceId() == who->getStats()->getInstanceId())
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
    this->getStats()->setStatsOwner(owner);
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

    soundManager::getInstance()->registerSound(who->getBoard()->getInstanceId(),c3d,vel,who->getStats()->getInstanceId(),this->getType(),this->getAttrs()->getSubtype(),eventType,event);

}









