#include "../include/bElem.h"
#include "floorElement.h"
#include "rubbish.h"
#include "elements.h"

videoElement::videoElementDef* bElem::vd=nullptr;
std::vector<std::shared_ptr<bElem>> bElem::liveElems;
unsigned int bElem::sTaterCounter=0;
int bElem::instances=0;
bool bElem::randomNumberGeneratorInitialized=false;
std::mt19937 bElem::randomNumberGenerator;


int bElem::getInstanceid() const
{
    return this->eConfig.instance;
}

void bElem::resetInstances()
{
    bElem::instances=0;
}


bElem::bElem() : std::enable_shared_from_this<bElem>(),elementMutex(al_create_mutex())
{

    this->eConfig.instance=bElem::instances++;
    this->state.myDirection=UP;
    if(!bElem::randomNumberGeneratorInitialized)
    {
        std::random_device rd;
        std::mt19937::result_type seed = rd() ^ (
                                             (std::mt19937::result_type)
                                             std::chrono::duration_cast<std::chrono::seconds>(
                                                     std::chrono::system_clock::now().time_since_epoch()
                                             ).count() +
                                             (std::mt19937::result_type)
                                             std::chrono::duration_cast<std::chrono::microseconds>(
                                                     std::chrono::high_resolution_clock::now().time_since_epoch()
                                             ).count() );
        bElem::randomNumberGenerator.seed(seed);
        bElem::randomNumberGeneratorInitialized=true;
    }
}

bElem::bElem(std::shared_ptr<chamber> board): bElem()
{
    this->attachedBoard=board;
}


bool bElem::isUsable()
{
    return false;
}

bool bElem::isWeapon()
{
    return false;
}



coords bElem::getOffset()
{
    return {0,0};
}
bool bElem::isWaiting()
{
    if((long int)this->state.waiting>(long int)this->getCntr()+_maxWaitingTtime)
        this->state.waiting=0;
    if ((long int)(this->getCntr())<(long int)(this->state.waiting)) // the counter can get overloaded
        return true;
    return false;
}

void bElem::setWait(int time)
{
    this->state.waiting=this->getCntr()+time;
}

int bElem::getWait() const
{
    int res=(this->state.waiting>0)?this->state.waiting-(int)this->getCntr():0;
    return res;
}

bool bElem::setDirection(direction dir)
{
    this->state.myDirection=dir;

    return true;
}



std::shared_ptr<chamber> bElem::getBoard()
{
    return this->attachedBoard;
}

void bElem::setBoard(std::shared_ptr<chamber> board)
{
    this->attachedBoard=board;
    if (this->eConfig.myInventory.get()!=nullptr)
    {
        this->eConfig.myInventory->updateBoard();
    }
}


direction bElem::getDirection()
{
    return this->state.myDirection;
}

void bElem::setCoords(int x, int y)
{
    this->state.myPosition=(coords)
    {
        x,y
    };
}
void bElem::setCoords(coords point)
{
    this->state.myPosition=point;
}

std::shared_ptr<bElem> bElem::getStomper()
{
    return this->state.stomping;
}

void bElem::stomp(std::shared_ptr<bElem> who)
{
    this->state.stomping=who;
}

void bElem::unstomp()
{
    this->state.stomping=nullptr;
}

void bElem::setCollected(std::shared_ptr<bElem> who)
{
#ifdef _VerbousMode_

    if(who.get()!=nullptr)
        std::cout<<"Set collected me: "<<this->getInstanceid()<<" collector "<<who->getInstanceid()<<"\n";
    else
        std::cout<<"Set null on collector "<<this->getInstanceid()<<"\n";
#endif
    this->state.collector=who;
#ifdef _VerbousMode_

    std::cout<<"is it set? "<<((this->state.collector.get()!=nullptr)?"yes":"no")<<"\n";
    std::cout<<"is it set? "<<((this->state.collector.get()==who.get())?"yes":"no")<<"\n";
#endif
}

void bElem::setDropped()
{
    this->state.collector=nullptr;
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
    if (step.get()==nullptr || step->isSteppable()==false || step->getBoard().get()==nullptr || step->isDisposed())
        return false;
    this->removeElement();
    this->setCoords(step->getCoords());
    this->setBoard(step->getBoard());
    this->state.steppingOn=step;
    if(step->getStomper().get()!=nullptr)
    {
        std::shared_ptr<bElem> stmpr=step->getStomper();
        step->unstomp();
        this->stomp(stmpr);
        stmpr->state.steppingOn=shared_from_this();
    }
    else
    {
        if(step->getCoords()!=NOCOORDS)
            this->getBoard()->setElement(step->getCoords(),shared_from_this());
    }
    step->stomp(shared_from_this());
    return true;
}



oState bElem::disposeElementUnsafe()
{
    oState res=DISPOSED;
    std::shared_ptr<chamber> myBoard=this->getBoard();
    coords mycoords=this->getCoords();
    if(this->isDisposed()==true)
        return ERROR;
    this->state.disposed=true;
    if(this->getInventory().get()!=nullptr)
        this->getInventory()->changeOwner(nullptr);
    if(mycoords.x>=0 && mycoords.y>=0 && this->getBoard()!=nullptr) //object on a board? need extra steps
    {
        if(this->getSteppingOnElement().get()!=nullptr || this->getStomper().get()!=nullptr)
        {
            this->removeElement();
            res=DISPOSED;
        }
        else
        {
            this->getBoard()->setElement(this->state.myPosition,nullptr);
            res=nullptrREACHED;
        }

        if(this->getType()!=_stash && this->getInventory().get()!=nullptr && this->getInventory()->isEmpty()==false && this->getType()!=_rubishType && this->getType()!=_plainMissile && this->getType()!=_plainGun )
        {
            std::shared_ptr<bElem> stash=elementFactory::generateAnElement<rubbish>(myBoard);
            stash->setInventory(this->getInventory());
            stash->getInventory()->changeOwner(stash);
            this->setInventory(nullptr);
            if(myBoard->getElement(mycoords)->isSteppable())
            {
                stash->stepOnElement(myBoard->getElement(mycoords));
            }
            else
            {
                bool stashed=false;
                for(int c=0; c<4; c++)
                {
                    if(myBoard->getElement(mycoords)->isSteppableDirection((direction)c))
                    {
                        stash->stepOnElement(myBoard->getElement(mycoords)->getElementInDirection((direction)c));
                        stashed=true;
                        break;
                    }

                }
                if(!stashed)
                    stash->disposeElement(); // no place for the stash? Burn!

            }
        }

    }
    this->state.disposed=true;
    this->state.myPosition=NOCOORDS;
    this->attachedBoard=nullptr;
    return res; // false means that there is no more elements to go.
}






oState bElem::disposeElement()
{
    std::shared_ptr<bElem> t=shared_from_this();
    std::shared_ptr<bElem> stash=nullptr;
    coords oCoords=this->getCoords();
    //  std::shared_ptr<chamber>  board=this->getBoard();
  //  std::cout<<"Disposal of an element. parameters: disposed: "<<((this->isDisposed())?"true":"false")<<" x,y "<<this->getCoords().x<<" "<<this->getCoords().y<<" \n";
  //  std::cout<<"type: "<<this->getType()<<" collected "<<((this->getCollector().get()!=nullptr)?"true":"false")<<"\n";

    if(this->state.disposed==true)
    {
        std::cout<<"Tried to dispose the same element another time!\n";
        return ERROR;

    }
    this->state.disposed=true;
    if(this->isLiveElement())
    {
        this->deregisterLiveElement(this->getInstanceid());
    }
    if(this->getCollector().get()!=nullptr)
    {
#ifdef _VerbousMode_
        std::cout<<"The element is collected!\n";
#endif
        std::shared_ptr<inventory> cInv=this->getCollector()->getInventory();
        if(cInv.get()!=nullptr)
            cInv->removeCollectibleFromInventory(this->getInstanceid());
        this->setDropped();
        this->state.disposed=true;
        this->attachedBoard=nullptr;
        this->state.myPosition=NOCOORDS;
        return DISPOSED;
    }
    if(this->getInventory().get()!=nullptr && oCoords!=NOCOORDS)
    {
        if(this->getInventory()->isEmpty()==false)
        {
            /*
                Create a rubbish element on the board, so the inventory would not be lost
             */
            stash=elementFactory::generateAnElement<rubbish>(this->getBoard());
            stash->setInventory(this->getInventory());
            stash->getInventory()->changeOwner(stash);
            this->setInventory(nullptr);
            if(this->getStomper().get()!=nullptr)
                stash->stomp(this->getStomper());
            if(this->getSteppingOnElement().get()!=nullptr)
                stash->stepOnElement(this->getSteppingOnElement());
            else
            { /*
                 These situations might happen, when we would like to have the floor destructable, so let's support that situation:
                 In case, that we want to dispose the element, that stands on nullptr, we will create a floor element, place it underneath disposed element,
                 and then finally we will make a stash object, and place it on the floor we just had created.
                */
                std::shared_ptr<floorElement> nF=elementFactory::generateAnElement<floorElement>(this->getBoard());
                nF->setCoords(oCoords);
                this->state.steppingOn=nF;
                nF->stomp(shared_from_this());
                stash->stepOnElement(this->getSteppingOnElement());
            }
        }
    }
    this->removeElement();
    this->attachedBoard=nullptr;
    this->state.myPosition=NOCOORDS;
    return DISPOSED;
}


/*
 * This method returns absolute coordinates, when asked for coordinates of the next cell in a direction from the elements's standpoint, should be resistant for non-provisioned units
 * better use those on a board though
 */
coords bElem::getAbsCoords(direction dir)
{
    coords res=this->getCoords();
    if (this->attachedBoard.get()==nullptr)
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
    if (res.y>=this->attachedBoard->height || res.y<0 || res.x>=this->attachedBoard->width || res.x<0)
    {
        return NOCOORDS;
    }
    return res;
}

std::shared_ptr<bElem> bElem::getElementInDirection(direction di)
{
    coords mycoords=this->getAbsCoords(di);
    if (mycoords==NOCOORDS)
        return nullptr;
    return this->attachedBoard->getElement(mycoords.x,mycoords.y);
}



bElem::~bElem()
{
    al_destroy_mutex(this->elementMutex);
    if(this->isLiveElement())
        this->deregisterLiveElement(this->getInstanceid());

}


ALLEGRO_MUTEX* bElem::getMyMutex()
{
    return this->elementMutex;
}



int bElem::getSubtype()
{
    return this->eConfig.subtype;
}

int bElem::getAmmo()
{
    return 0;
}

void bElem::setAmmo()
{
    return;
}

std::shared_ptr<inventory> bElem::getInventory()
{

    return this->eConfig.myInventory;

}




std::shared_ptr<elemStats> bElem::getStats()
{
    return this->eConfig.myStats;
}

void bElem::setStats(std::shared_ptr<elemStats> stat)
{
    this->eConfig.myStats=stat;
}



bool bElem::moveInDirection(direction d)
{
    return false;
}

bool bElem::use(std::shared_ptr<bElem> who)
{
    return false;
}

void bElem::stopWaiting()
{
    this->state.waiting=0;
}



bool bElem::interact(std::shared_ptr<bElem> who)
{
    if(this->canInteract()) /* penalty for getting into counter overflow */
    {
        this->state.interacted=this->getCntr()+_interactedTime;
        return true;
    }
    return false;
}

bool bElem::destroy()
{

    if (this->canBeDestroyed() || this->isSteppable() ||this->isDestroyed() ) // && (!this->isDestroyed()))
    {
        if (this->canBeDestroyed())
            this->registerLiveElement(shared_from_this());
        this->state.destroyed=_defaultDestroyTime+this->getCntr();
        this->state.destTimeBeg=this->getCntr();
        this->state.destTimeReq=_defaultDestroyTime;
        if(this->getSteppingOnElement().get()!=nullptr)
            this->getSteppingOnElement()->destroy();

        return true;
    }
    return false;
}


int bElem::getAnimPh()
{
    int base=(int)this->getCntr();
    if(this->isDying())
    {
        base=(int)(this->getCntr()-this->state.killTimeBeg);
    }
    if(this->isDestroyed())
    {
        base=(int)(this->getCntr()-this->state.destTimeBeg);
    }
    if(this->isTeleporting())
    {
        base=(int)(this->getCntr()-this->state.telReqTime);
    }
    return base>>3;
}

int bElem::getSwitchId()
{
    return -1;
}

bool bElem::isMovable()
{
    return false;
}

bool bElem::isActive()
{
    return true;
}

bool bElem::isOpen()
{
    return true;
}

bool bElem::isSwitchOn()
{
    return false;
}

bool bElem::mechanics()
{
    this->state.taterCounter++; //this is our own source of sequential numbers
    if((this->getBoard().get()==nullptr || this->getCoords()==NOCOORDS) && (this->getCollector().get()==nullptr))
        return false;

    if (this->canBeDestroyed() && (long int)this->state.destroyed>0 && this->getCntr()>=this->state.destTimeBeg+this->state.destTimeReq-1 )
    {
        this->disposeElement();
        return false;
    }
    if((long int)this->state.killed>0 && this->canBeKilled() && this->getCntr()>=this->state.killTimeBeg+this->state.killTimeReq-1 )
    {
        this->disposeElement(); //it seems we really died. what a waste
        return false;
    }
    if(this->isWaiting() || this->isTeleporting() || this->isDying() || this->isDestroyed())
        return false;

    return true;
}

bool bElem::isSteppable()
{
    return  (!this->isDying()) && (!this->isTeleporting()) && (!this->isDestroyed()) ;
}

bool bElem::canBeKilled()
{
    return true;
};

bool bElem::canBeDestroyed()
{
    return true;
}

bool bElem::isSteppableDirection(direction di)
{
    coords tmpcoords;
    tmpcoords=this->getAbsCoords(di);
    if(!(tmpcoords==NOCOORDS))
    {
        if(this->attachedBoard->getElement(tmpcoords).get()!=nullptr)
        {
            return this->attachedBoard->getElement(tmpcoords)->isSteppable();
        }
    }
    return false;
}


videoElement::videoElementDef* bElem::getVideoElementDef()
{
    return bElem::vd;
}

bool bElem::isDying()
{
    if((long int)this->state.killed>(long int)this->getCntr()+_maxWaitingTtime)
        this->state.killed=0;
    return (this->state.killed>0 && (long int)this->state.killed>=(long int)this->getCntr());
}

bool bElem::isTeleporting()
{
    if((long int)this->state.telInProgress>(long int)this->getCntr()+_maxWaitingTtime)
        this->state.telInProgress=0;
    if ((long int)(this->getCntr())<(long int)(this->state.telInProgress)) // the counter can get overloaded
        return true;
    return false;

}


bool bElem::canCollect()
{
    return this->eConfig.myInventory.get()!=nullptr;
}
// remove element from the board, and return it for further processing(if not needed, run .dispose() on it)
std::shared_ptr<bElem> bElem::removeElement()
{

    if (this->state.myPosition==NOCOORDS || this->getBoard().get()==nullptr)
    {
        return (this->isDisposed())?nullptr:shared_from_this();
    }
    if(this->getSteppingOnElement()!=nullptr && this->getStomper().get()!=nullptr)
    {
        this->getStomper()->state.steppingOn=this->state.steppingOn;
        this->getSteppingOnElement()->stomp(this->getStomper());
    }
    else if (this->getSteppingOnElement().get()!=nullptr && this->getStomper().get()==nullptr)
    {
        this->attachedBoard->setElement(this->getCoords(),this->getSteppingOnElement());
        this->getSteppingOnElement()->unstomp();
    }
    else if (this->getSteppingOnElement().get()==nullptr && this->getStomper().get()!=nullptr)
    {
        this->getStomper()->state.steppingOn=nullptr;
    }
    else if(this->getStomper().get()==nullptr && this->getSteppingOnElement().get()==nullptr)
    {
        std::shared_ptr<bElem> newElem=elementFactory::generateAnElement<floorElement>(this->attachedBoard);
        newElem->setCoords(this->getCoords());
        newElem->attachedBoard->setElement(newElem->getCoords(),newElem);
    }
    this->unstomp();
    this->state.steppingOn=nullptr;
    this->state.myPosition=NOCOORDS;
    this->attachedBoard=nullptr;
    return shared_from_this();
}


// this object cannot actively interact with outhers
bool bElem::canInteract()
{
#ifdef _VerbousMode_
    if (this->getType()==_player)
        std::cout<<"interacted "<<this->interacted<<"  taterCounter "<<this->sTaterCounter<<"\n";
#endif
    return (this->state.interacted<this->getCntr()|| this->state.interacted>this->getCntr()+_interactedTime+10);
}

// Collect another element. The collectible contains location information. that way,
// we restore the element below the collectible, and the collectible will be stored in a vector structure.
// but for now it does nothing
bool bElem::collect(std::shared_ptr<bElem> collectible)
{
    std::shared_ptr<bElem> collected;
    if (collectible.get()==nullptr || !collectible->isCollectible() ||!this->canCollect()|| collectible->isDying() || collectible->isTeleporting() || collectible->isDestroyed())
    {
        return false;
    }

    collected=collectible->removeElement();
    if (collected.get()==nullptr) //this should never happen!
    {
        std::cout<<"Collecting failed!\n";
        return false;
    }
#ifdef _VerbousMode_
    std::cout<<"Collect "<<collected->getType()<<" st: "<<collected->getSubtype()<<"\n";
#endif
    collectible->setCollected(shared_from_this());
#ifdef _VerbousMode_
    std::cout<<"Collected set? "<<(collectible->getCollector()==shared_from_this())<<"\n";
#endif
    this->eConfig.myInventory->addToInventory(collectible);
    return true;
}


bool bElem::setSubtype(int st)
{
    al_lock_mutex(this->elementMutex);
    this->eConfig.subtype=st;
    al_unlock_mutex(this->elementMutex);
    return true;
}




bool bElem::setEnergy(int points)
{
    al_lock_mutex(this->elementMutex);
    if(this->getStats().get()!=nullptr)
        this->getStats()->setEnergy(points);
    al_unlock_mutex(this->elementMutex);
    return true;
}

bool bElem::kill()
{
    if (!this->canBeKilled() || this->isDying() ||this->isDestroyed() || this->isTeleporting())
    {
        return false;
    }
    if(!isLiveElement())
        this->registerLiveElement(shared_from_this());
    this->state.killTimeBeg=this->getCntr();
    this->state.killTimeReq=_defaultKillTime;
    this->state.killed=_defaultKillTime+this->getCntr();
    return true;
}

bool bElem::isDestroyed()
{
    if((long int)this->state.destroyed+_maxWaitingTtime<(long int)this->getCntr())
        this->state.destroyed=0;

    return this->state.destroyed>0 && (long int)this->state.destroyed>=(long int)this->getCntr();
}

bool bElem::additionalProvisioning()
{
    bool r=this->eConfig.provisioned;
    this->eConfig.provisioned=true;
    if(r==true)
        return true;
    if(this->getInventory().get()!=nullptr)
    {
#ifdef _VerbousMode_
        std::cout<<"Adding owner to the inventory\n";
#endif
        this->eConfig.myInventory->changeOwner(shared_from_this());
#ifdef _VerbousMode_
        std::cout<<"set inv owner to: "<<this->getInstanceid()<<"\n";
#endif
    }
    return r;
}


bool bElem::isMod()
{
    return false;
}

modType bElem::getModType()
{
    return NONE;
}



int bElem::getTypeInDirection(direction di)
{
    std::shared_ptr<bElem> e=this->getElementInDirection(di);
    if(e.get()!=nullptr)
        return e->getType();
    return -1;
}


/*
Here we want to avoid the duplication of boundary checking, that is why we use getABSCoords, isSteppableInDirection and getElementInDirection;
*/
sNeighboorhood bElem::getSteppableNeighboorhood()
{
    sNeighboorhood myNeigh;
    coords up,left,down,right;

    for(int c=0; c<8; c+=2)
    {
        int c1=c/2;
        direction d=(direction)(c1%4);
        direction d1=(direction)((c1+1)%4);
        std::shared_ptr<bElem> e=this->getElementInDirection(d);
        std::shared_ptr<bElem> e1=nullptr;
        if(e.get()!=nullptr)
        {
            e1=e->getElementInDirection(d1);
            myNeigh.nTypes[c]=e->getType();
            myNeigh.steppable[c]=e->isSteppable();
            myNeigh.steppableClose[c/2]=myNeigh.steppable[c];
            if(e1.get()!=nullptr)
            {
                myNeigh.nTypes[c+1]=e1->getType();
                myNeigh.steppable[c+1]=e1->isSteppable();
            }
            else
            {
                myNeigh.nTypes[c+1]=-1;
                myNeigh.steppable[c+1]=false;
            }


        }
        else
        {
            myNeigh.nTypes[c+1]=-1;
            myNeigh.steppable[c+1]=false;
            myNeigh.nTypes[c]=-1;
            myNeigh.steppable[c]=false;
            myNeigh.steppableClose[c/2]=myNeigh.steppable[c];
        }

    }
    return myNeigh;
}

bool bElem::moveInDirectionSpeed(direction d, int speed)
{
    return false;

}

void bElem::setMoved(int time)
{

}
int bElem::getMoved()
{
    return 0;
}
void bElem::setTeleporting(int time)
{
    this->state.telReqTime=this->getCntr();
    this->state.telTimeReq=time;
    this->state.telInProgress=this->getCntr()+time;

}



void bElem::registerLiveElement(std::shared_ptr<bElem> who)
{
#ifdef _VerbousMode_
    std::cout<<"Register mechanics by: "<<who->getType()<<" "<<who->getInstanceid()<<"RL\n";
#endif
    if (this->isLiveElement())
        return;
    this->state.hasActivatedMechanics=true;
    bElem::liveElems.push_back(who);
}

void bElem::deregisterLiveElement(int instanceId)
{
    if (!this->isLiveElement())
        return;
    std::vector<std::shared_ptr<bElem>>::iterator p;
    for(p=bElem::liveElems.begin(); p!=bElem::liveElems.end();)
    {
        if(instanceId==(*p)->getInstanceid())
        {
            bElem::liveElems.erase(p);
        }
        else
        {
            p++;
        }
    }
    this->state.hasActivatedMechanics=false;
}

void bElem::runLiveElements()
{
    bElem::tick();
    for(unsigned int p=0; p<bElem::liveElems.size(); p++)
    {
        if(bElem::liveElems[p]->getCoords()!=NOCOORDS)
        {
            bElem::liveElems[p]->mechanics();
        }
        else if(bElem::liveElems[p]->getStomper().get()!=nullptr || bElem::liveElems[p]->getCollector().get()!=nullptr)
        {
            bElem::liveElems[p]->mechanics();
        }

    }

    //  if (gCollect::getInstance()->garbageQsize()>0) //clean up, when there is garbage to be cleared
    //      gCollect::getInstance()->purgeGarbage();
}

void bElem::setActive(bool active)
{

}


void bElem::tick()
{
    bElem::sTaterCounter++;

}

unsigned int bElem::getCntr() const
{
    return bElem::sTaterCounter;
}

bool bElem::isLiveElement()
{
    return this->state.hasActivatedMechanics;
}

bool bElem::isLocked()
{
    return this->lockers.size()!=0;
}

bool bElem::lockThisObject(std::shared_ptr<bElem> who)
{
    this->lockers.push_back(who);
    return true;
}

bool bElem::unlockThisObject(std::shared_ptr<bElem> who)
{
    for(unsigned int cnt=0; cnt<this->lockers.size();)
    {
        if(this->lockers.at(cnt)->getInstanceid()==who->getInstanceid())
        {
            this->lockers.erase(this->lockers.begin()+cnt);
        }
        else
        {
            cnt++;
        }
    }
    return true;
}

void bElem::setInventory(std::shared_ptr<inventory> inv)
{
    this->eConfig.myInventory=inv;
}

int bElem::getEnergy()
{
    return (this->getStats()!=nullptr)?this->getStats()->getEnergy():555;
};


std::shared_ptr<bElem> bElem::getCollector()
{
    return this->state.collector;
}



std::shared_ptr<bElem> bElem::getSteppingOnElement()
{
    return this->state.steppingOn;
}


void bElem::setStatsOwner(std::shared_ptr<bElem>owner)
{

}


