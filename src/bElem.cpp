#include "../include/bElem.h"
#include "elements.h"
videoElement::videoElementDef* bElem::vd=NULL;
std::vector<bElem*> bElem::liveElems;
unsigned int bElem::sTaterCounter=0;
int bElem::instances=0;
bool bElem::randomNumberGeneratorInitialized=false;
std::mt19937 bElem::randomNumberGenerator;


int bElem::getInstanceid()
{
    return this->instance;
}

void bElem::resetInstances()
{
    bElem::instances=0;
}



bElem::bElem()
{

    this->init();
}

bElem::bElem(chamber* board)
{
    this->init();
    this->x=-1;
    this->y=-1;
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


bElem::bElem(chamber* board, int x, int y)
{
    this->init();
    this->attachedBoard=board;
    this->x=x;
    this->y=y;
    // If we fail to place the object on a board, we will let it hanging, check, if x or y are ==-1
    if (this->attachedBoard->chamberArray[x][y]!=NULL)
    {
        if (this->stepOnElement(this->attachedBoard->chamberArray[x][y])==false)
        {
            this->x=-1;
            this->y=-1;
        }
    }
    else
    {
        this->attachedBoard->chamberArray[x][y]=this;
    }

}
coords bElem::getOffset()
{
    return {0,0};
}
bool bElem::isWaiting()
{
    if((long int)this->waiting>(long int)this->getCntr()+_maxWaitingTtime)
        this->waiting=0;
    if ((long int)(this->getCntr())<(long int)(this->waiting)) // the counter can get overloaded
        return true;
    return false;
}

void bElem::setWait(int time)
{
    this->waiting=this->getCntr()+time;
}

int bElem::getWait()
{
    return (this->waiting>0)?(int)this->waiting-(int)this->getCntr():0;
}
void bElem::init()
{
    this->telReqTime=0;
    this->killTimeReq=0;
    this->destTimeReq=0;
    this->killTimeBeg=0;
    this->destTimeBeg=0;
    this->telTimeReq=0;
    this->setSubtype(0);
    this->setDirection(NODIRECTION);
    this->waiting=0;
    this->disposed=false;
    this->hasActivatedMechanics=false;
    this->stomping=NULL;
    this->collector=NULL;
    this->attachedBoard=NULL;

    this->animPhase=0;
    this->myStats=new elemStats(_defaultEnergy);
    this->killed=0;
    this->steppingOn=NULL;
    this->interacted=0;
    this->destroyed=0;
    this->instance=bElem::instances++;

    this->myInventory=NULL;
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
    this->telInProgress=0;
}

bool bElem::setDirection(direction dir)
{
    this->myDirection=dir;

    return true;
}

bool bElem::isInteractive()
{
    return false;
}

chamber* bElem::getBoard()
{
    return this->attachedBoard;
}

void bElem::setBoard(chamber* board)
{
    this->attachedBoard=board;
    if (this->myInventory!=NULL)
    {
        this->myInventory->updateBoard();
    }



}


direction bElem::getDirection()
{
    return this->myDirection;
}

void bElem::setCoords(int x, int y)
{
    this->x=x;
    this->y=y;
}
void bElem::setCoords(coords point)
{
    this->setCoords(point.x,point.y);
}

bElem* bElem::getStomper()
{
    return this->stomping;
}

void bElem::stomp(bElem* who)
{
    this->stomping=who;
}

void bElem::unstomp()
{
    this->stomping=NULL;
}

void bElem::setCollected(bElem* who)
{
    this->collector=who;
}

void bElem::setDropped()
{
    this->collector=NULL;
}

/*
   This is the most basic method for dealing with all objects. Because all of them must be put to a place on a boart at some point,
   and this is the method that is to be used for that purpose.
   It should work on all types of configurations, except standing on an empty point
   If places on an object that already is covered by another one, the newly placed object is placed in between
   This method takes "steppable" flag into consideration
*/
bool bElem::stepOnElement(bElem* step)
{
    if (step==NULL || step->isSteppable()==false || step->getBoard()==NULL)  return false;
    if(this->getStomper()!=NULL || this->getSteppingOnElement()!=NULL)
        this->removeElement();
    this->setCoords(step->getCoords());
    this->setBoard(step->getBoard());
    this->steppingOn=step;
    if(step->getStomper()!=NULL)
    {
        bElem* stmpr=step->getStomper();
        step->unstomp();
        this->stomp(stmpr);
        stmpr->steppingOn=this;
    }
    else
    {
        if(step->getCoords()!=NOCOORDS)
            this->getBoard()->setElement(step->getCoords(),this);
    }
    step->stomp(this);
    return true;
}




coords bElem::getCoords()
{
    coords mycoords;
    mycoords.x=this->x;
    mycoords.y=this->y;
    return mycoords;
}
oState bElem::disposeElementUnsafe()
{
    oState res=DISPOSED;
    chamber *myBoard=this->getBoard();
    coords mycoords=this->getCoords();
    if(this->disposed==true)
        return ERROR;
    this->disposed=true;
    if(mycoords.x>=0 && mycoords.y>=0 && this->getBoard()!=NULL) //object on a board? need extra steps
    {
        if(this->getSteppingOnElement()!=NULL || this->getStomper()!=NULL)
        {
            this->removeElement();
            res=DISPOSED;
        }
        else
        {
            this->getBoard()->setElement(this->x,this->y,NULL);
            res=NULLREACHED;
        }
        if(this->getType()!=_stash && this->myInventory!=NULL && this->myInventory->isEmpty()==false && this->getType()!=_rubishType && this->getType()!=_plainMissile && this->getType()!=_plainGun )
        {
            bElem* stash=new rubbish(myBoard);
            stash->myInventory=this->myInventory;
            stash->myInventory->changeOwner(stash);
            this->myInventory=NULL;
            if(myBoard->getElement(mycoords.x,mycoords.y)->isSteppable())
            {
                stash->stepOnElement(myBoard->getElement(mycoords.x,mycoords.y));
            }
            else
            {
                bool stashed=false;
                for(int c=0; c<4; c++)
                {
                    if(myBoard->getElement(mycoords.x,mycoords.y)->isSteppableDirection((direction)c))
                    {
                        stash->stepOnElement(myBoard->getElement(mycoords.x,mycoords.y)->getElementInDirection((direction)c));
                        stashed=true;
                        break;
                    }

                }
                if(!stashed)
                    stash->disposeElement(); // no place for the stash? Burn!

            }
        }

    }

    gCollect::getInstance()->addToBin(this); //add myself to to bin - this should be the only way of the object disposal!
    this->disposed=true;
    this->x=-1; //we set the state of the object to be unprovisioned - out of the game.
    this->y=-1;
    this->attachedBoard=NULL;
    return res; // false means that there is no more elements to go.
}

oState bElem::disposeElement()
{
    rubbish* stash=NULL;
    coords oCoords=this->getCoords();
    chamber* board=this->getBoard();
    if(this->disposed==true)
    {
        std::cout<<"Tried to dispose the same element another time!\n";
        return ERROR;

    }
    if(this->isLiveElement())
    {
        this->deregisterLiveElement(this);
    }
    if(this->getCollector()!=NULL)
    {
        //  std::cout<<"Removing collected item from"<<this->getCollector()->myInventory<<"\n";

        inventory* cInv=this->getCollector()->myInventory;
        if(cInv!=NULL)
            cInv->removeCollectibleFromInventory(this->getInstanceid());
        this->setDropped();
        this->disposed=true;
        this->attachedBoard=NULL;
        this->x=-1;
        this->y=-1;
        gCollect::getInstance()->addToBin(this);
        return DISPOSED;
    }
    if(this->getInventory()!=NULL && oCoords!=NOCOORDS)
    {
        if(this->getInventory()->isEmpty()==false)
        {
            stash=new rubbish(this->getBoard());
            stash->setInventory(this->getInventory());
            this->setInventory(NULL);

        }
    }
    this->removeElement();
    if(stash!=NULL)
    {
        stash->stepOnElement(board->getElement(oCoords));
    }
    this->disposed=true;
    this->attachedBoard=NULL;
    this->x=-1;
    this->y=-1;
    gCollect::getInstance()->addToBin(this);
    return DISPOSED;
}


/*
 * This method returns absolute coordinates, when asked for coordinates of the next cell in a direction from the elements's standpoint, should be resistant for non-provisioned units
 * better use those on a board though
 */
coords bElem::getAbsCoords(direction dir)
{
    coords res;
    res.x=this->x;
    res.y=this->y;

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
    if (this->attachedBoard==NULL)
        return NOCOORDS;
//    std::cout<<"resxy "<<res.y<<" "<<(this->attachedBoard->width)<<"\n";
    if (res.y>=this->attachedBoard->height || res.y<0 || res.x>=this->attachedBoard->width || res.x<0)
    {
        return NOCOORDS;
    }
    return res;
}

bElem* bElem::getElementInDirection(direction di)
{
    coords mycoords=this->getAbsCoords(di);
    if (mycoords==NOCOORDS)
        return NULL;
    return this->attachedBoard->getElement(mycoords.x,mycoords.y);
}



bElem::~bElem()
{
    delete this->myStats;
    if(this->isLiveElement())
        this->deregisterLiveElement(this);
    if(this->myInventory!=NULL)
    {
        delete this->myInventory;

    }
}

int bElem::getType()
{
    return _belemType;
}

int bElem::getSubtype()
{
    return this->subtype;
}

int bElem::getAmmo()
{
    return 0;
}

void bElem::setAmmo()
{
    return;
}


bool bElem::isProvisioned()
{
    if (this->attachedBoard!=NULL && this->x!=-1 && this->y!=-1 )
        return true;
    return false;
}

bool bElem::moveInDirection(direction d)
{
    return false;
}

bool bElem::use(bElem *who)
{
    return false;
}

void bElem::stopWaiting()
{
    this->waiting=0;
}



bool bElem::interact(bElem *who)
{
    if(this->canInteract()) /* penalty for getting into counter overflow */
    {
        this->interacted=this->getCntr()+_interactedTime;
        return true;
    }
    return false;
}

bool bElem::destroy()
{

    if (this->canBeDestroyed()==true) // && (!this->isDestroyed()))
    {

        this->registerLiveElement(this);
        this->destroyed=_defaultDestroyTime+this->getCntr();
        this->destTimeBeg=this->getCntr();
        this->destTimeReq=_defaultDestroyTime;
        if(this->getSteppingOnElement()!=NULL)
            this->getSteppingOnElement()->destroy();

        //  this->killed=_defaultDestroyTime+this->getCntr();
        return true;
    }
    return false;
}


int bElem::getAnimPh()
{
    int base=(int)this->getCntr();
    if(this->isDying())
    {
        base=(int)(this->getCntr()-this->killTimeBeg);
    }
    if(this->isDestroyed())
    {
        base=(int)(this->getCntr()-this->destTimeBeg);
    }
    if(this->isTeleporting())
    {
        base=(int)(this->getCntr()-this->telReqTime);
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
    this->taterCounter++; //this is our own source of sequential numbers
    if((this->getBoard()==NULL || this->getCoords()==NOCOORDS) && (this->getCollector()==NULL))
        return false;

    if (this->canBeDestroyed() && (long int)this->destroyed>0 && this->getCntr()>this->destTimeBeg+this->destTimeReq-1 && !this->isWaiting() )
    {
        this->disposeElement();
        return false;
    }
    if((long int)this->killed>0 && this->canBeKilled() && this->getCntr()>this->killTimeBeg+this->killTimeReq-1 && !this->isWaiting())
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
    //return (!this->isDying() && !this->isTeleporting() && this->isDestroyed());
};

bool bElem::canBeDestroyed()
{
    return true;
    //return (!this->isDying() && this->isDestroyed());
}

bool bElem::isSteppableDirection(direction di)
{
    coords tmpcoords;
    tmpcoords=this->getAbsCoords(di);
    if(!(tmpcoords==NOCOORDS))
    {
        if(this->attachedBoard->getElement(tmpcoords.x,tmpcoords.y)!=NULL)
        {
            return this->attachedBoard->getElement(tmpcoords.x,tmpcoords.y)->isSteppable();
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
    if((long int)this->killed>(long int)this->getCntr()+_maxWaitingTtime)
        this->killed=0;
    return (this->killed>0 && (long int)this->killed>=(long int)this->getCntr());
}

bool bElem::isTeleporting()
{
    if((long int)this->telInProgress>(long int)this->getCntr()+_maxWaitingTtime)
        this->telInProgress=0;
    if ((long int)(this->getCntr())<(long int)(this->telInProgress)) // the counter can get overloaded
        return true;
    return false;

}


bool bElem::isCollectible()
{
    return false;
}

bool bElem::canCollect()
{
    return myInventory!=NULL;
}
// remove element from the board, and return it for further processing(if not needed, run .dispose() on it)
bElem* bElem::removeElement()
{
    if (this->x<0 || this->y<0 || this->getBoard()==NULL)
    {
        return (this->disposed)?NULL:this;
    }
    if(this->getSteppingOnElement()!=NULL && this->getStomper()!=NULL)
    {
        this->getStomper()->steppingOn=this->steppingOn;
        this->getSteppingOnElement()->stomp(this->getStomper());
        this->unstomp();
        this->steppingOn=NULL;
        this->x=-1;
        this->y=-1;
        this->attachedBoard=NULL;
        return this;
    }
    if (this->getSteppingOnElement()!=NULL)
    {
        this->getSteppingOnElement()->unstomp();
        this->attachedBoard->setElement(this->x,this->y,this->getSteppingOnElement());
        this->steppingOn=NULL;
        this->x=-1;
        this->y=-1;
        this->attachedBoard=NULL;
        return this;
    }
    else
    {
        if(this->getStomper()!=NULL)
        {
            this->getStomper()->steppingOn=this->getSteppingOnElement();
        }
        else
        {
            bElem *newElem=new bElem(this->attachedBoard);
            newElem->setCoords(this->getCoords());
            newElem->attachedBoard->setElement(newElem->getCoords(),newElem);
        }
        this->x=-1;
        this->y=-1;
        this->attachedBoard=NULL;
        return this;
    }
    return NULL; // Should never happen!
}

// this object cannot actively interact with outhers
bool bElem::canInteract()
{
#ifdef _VerbousMode_
    if (this->getType()==_player)
        std::cout<<"interacted "<<this->interacted<<"  taterCounter "<<this->sTaterCounter<<"\n";
#endif
    return (this->interacted<this->getCntr()|| this->interacted>this->getCntr()+_interactedTime+10);
}

// Collect another element. The collectible contains location information. that way,
// we restore the element below the collectible, and the collectible will be stored in a vector structure.
// but for now it does nothing
bool bElem::collect(bElem *collectible)
{
    bElem *collected;
    if (collectible==NULL || collectible->isCollectible()==false ||this->canCollect()==false || collectible->isDying() || collectible->isTeleporting() || collectible->isDestroyed())
    {
        return false;
    }

    collected=collectible->removeElement();
    if (collected==NULL) //this should never happen!
    {
#ifdef _VerbousMode_
        std::cout<<"Collecting failed!\n";
#endif
        return false;
    }
#ifdef _VerbousMode_
    std::cout<<"Collect "<<collected->getType()<<" st: "<<collected->getSubtype()<<"\n";
#endif
    collectible->setCollected(this);
    this->myInventory->addToInventory(collectible);
    return true;
}

bElem* bElem::getCollector()
{
    return this->collector;
}





bool bElem::selfAlign()
{
    return false;
}

bool bElem::setSubtype(int st)
{
    this->subtype=st;
    return true;
}



bool bElem::hurt(int points)
{
    return false;
}

int bElem::getEnergy()
{
    return this->myStats->getEnergy();
}

bool bElem::setEnergy(int points)
{
    this->myStats->setEnergy(points);
    return true;
}

bool bElem::kill()
{
    if (!this->canBeKilled() || this->isDying() ||this->isDestroyed() || this->isTeleporting())
    {
        return false;
    }
    if(!isLiveElement())
        this->registerLiveElement(this);
    this->killTimeBeg=this->getCntr();
    this->killTimeReq=_defaultKillTime;
    this->killed=_defaultKillTime+this->getCntr();
    return true;
}

bool bElem::isDestroyed()
{
    if((long int)this->destroyed+_maxWaitingTtime<(long int)this->getCntr())
        this->destroyed=0;

    return this->destroyed>0 && (long int)this->destroyed>(long int)this->getCntr();
}

elemStats* bElem::getStats()
{
    return this->myStats;
}
void bElem::setStats(elemStats* stat)
{
    this->myStats=stat; // we do not do anything with the stats, please use it wisely
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
    bElem *e=this->getElementInDirection(di);
    if(e!=NULL)
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
        bElem* e=this->getElementInDirection(d);
        bElem* e1=NULL;
        if(e!=NULL)
        {
            e1=e->getElementInDirection(d1);
            myNeigh.nTypes[c]=e->getType();
            myNeigh.steppable[c]=e->isSteppable();
            myNeigh.steppableClose[c/2]=myNeigh.steppable[c];
            if(e1!=NULL)
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
    this->telReqTime=this->getCntr();
    this->telTimeReq=time;
    this->telInProgress=this->getCntr()+time;

}



void bElem::registerLiveElement(bElem* who)
{
    if (this->isLiveElement())
        return;
    this->hasActivatedMechanics=true;
    /*
    for(std::vector<bElem*>::iterator p=bElem::liveElems.begin(); p!=bElem::liveElems.end(); p++) //we don't register objects twice
    {
        if ((*p)->getInstanceid()==who->getInstanceid())
        {
            return;
        }
    }
    */
    bElem::liveElems.push_back(who);
}

void bElem::deregisterLiveElement(bElem* who)
{
    if (!this->isLiveElement())
        return;
    std::vector<bElem*>::iterator p;
    for(p=bElem::liveElems.begin(); p!=bElem::liveElems.end();)
    {
        if(who->getInstanceid()==(*p)->getInstanceid())
        {
            bElem::liveElems.erase(p);
            //std::cout<<"Deregistered element "<<who->getType()<<" "<<who<<"\n";
        }
        else
        {
            p++;
        }
    }
    this->hasActivatedMechanics=false;
}

void bElem::runLiveElements()
{
//    std::cout<<"Active Elements: "<<bElem::liveElems.size()<<"\n";
    bElem::tick();
    for(unsigned int p=0; p<bElem::liveElems.size(); p++)
    {


        if(bElem::liveElems[p]->getCoords()!=NOCOORDS)
        {
            bElem::liveElems[p]->mechanics();
        }
        else if(bElem::liveElems[p]->getStomper()!=NULL || bElem::liveElems[p]->getCollector()!=NULL)
        {
            bElem::liveElems[p]->mechanics();
        }

    }

    if (gCollect::getInstance()->garbageQsize()>0) //clean up, when there is garbage to be cleared
        gCollect::getInstance()->purgeGarbage();
}

void bElem::setActive(bool active)
{

}


void bElem::tick()
{
    bElem::sTaterCounter++;

}

unsigned int bElem::getCntr()
{
    return bElem::sTaterCounter;
}

bool bElem::isLiveElement()
{
    return this->hasActivatedMechanics;
}

bool bElem::isLocked()
{
    return this->lockers.size()!=0;
}

bool bElem::lockThisObject(bElem* who)
{
    this->lockers.push_back(who);
    return true;
}

bool bElem::unlockThisObject(bElem* who)
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

void bElem::setInventory(inventory* inv)
{
    this->myInventory=inv;
}


