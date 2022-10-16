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
    if(this->waiting-this->getCntr() >_maxWaitingTtime) // the counter can get overloaded
        this->waiting=0;
    return (unsigned int)this->waiting>this->getCntr();
}

void bElem::setWait(int time)
{
    this->waiting=this->getCntr()+time;
}


void bElem::init()
{
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


// the way it should operate is:
// release the field it is residingg and step on the new one (replace it on a board)
// This can also step on things, that are under something. We just squeeze the object between to bottom and top
bool bElem::stepOnElement(bElem* step)
{
    if (step==NULL || step->isSteppable()==false || step->getBoard()==NULL)  return false;
    coords crds=step->getCoords();
    if (this->steppingOn!=NULL)
    {
        this->steppingOn->unstomp();
        if (this->getBoard()!=NULL)
            this->getBoard()->setElement(this->x,this->y,this->steppingOn);
    }
    this->setBoard(step->getBoard());
    this->steppingOn=step;
    if(step->getStomper()!=NULL)
    {
        bElem* stmpr=step->getStomper();
        step->unstomp();
        this->stomp(stmpr);
    }
    else
    {
        if(crds!=NOCOORDS)
            this->getBoard()->setElement(crds.x,crds.y,this);
    }
    step->stomp(this);
    this->setCoords(crds.x,crds.y);
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
        if(this->steppingOn!=NULL || this->getStomper()!=NULL)
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
    int x0=this->x;//save necessary data, becasue it will be lost after disposeElementUnsafe
    int y0=this->y;
    bool collectedItem=false;
    chamber* brd=this->getBoard();
    if(this->isLiveElement())
    {
        this->deregisterLiveElement(this);
    }

    if(this->getCollector()!=NULL)
    {
        collectedItem=true;
        this->getCollector()->myInventory->removeCollectibleFromInventory(this->getInstanceid());
        this->setDropped();
    }
    if(this->disposeElementUnsafe()==NULLREACHED && !collectedItem)
    {
        bElem* newElem=new bElem(brd);
        newElem->stepOnElement(brd->getElement(x0,y0));
    }
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

    if (this->canBeDestroyed()==true && this->isDestroyed()==false)
    {
        if(!this->isLiveElement())
            this->registerLiveElement(this);
        this->destroyed=_defaultDestroyTime;
        this->killed=_defaultDestroyTime;
        return true;
    }
    return false;
}


int bElem::getAnimPh()
{
    // this->animPhase++;
    return this->getCntr()>>2;
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

bool bElem::mechanics(bool collected)
{
    this->taterCounter++; //this is our source of sequential numbers
    this->animPhase++;
    if(this->getBoard()==NULL || this->getCoords()==NOCOORDS)
        return false;

    if (this->destroyed>0) //We support two modes of destruction, this one is the demolishion thing
    {
        this->destroyed--;
        //    std::cout<<"Destroying\n";
        if(this->destroyed==0)
        {
            this->killed=false;
            if (this->getType()!=_belemType)   //We do not dispose the empty elements
            {
                //this->removeElement();
                this->disposeElement();
            }
        }
        return false;

    }
    if(this->isWaiting())
        return false;

    return true;
}

bool bElem::isSteppable()
{
    return (!this->isDying()) && (!this->isTeleporting());
}

bool bElem::canBeKilled()
{
    return true;
}

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
    return (this->killed>0)?true:false;
}

bool bElem::isTeleporting()
{
    return (this->telInProgress>this->getCntr());
}


bool bElem::isCollectible()
{
    return false;
}

bool bElem::canCollect()
{
    return false;
}
// remove element from the board, and return it for further processing(if not needed, run .dispose() on it)
bElem* bElem::removeElement()
{
    if (this->x<0 || this->y<0)
    {
        return this;
    }
    if(this->steppingOn!=NULL && this->getStomper()!=NULL)
    {
        this->getStomper()->steppingOn=this->steppingOn;
        this->steppingOn->stomp(this->getStomper());
        this->unstomp();
        this->steppingOn=NULL;
        this->x=-1;
        this->y=-1;
        this->attachedBoard=NULL;
        return this;
    }
    if (this->steppingOn!=NULL)
    {
        this->steppingOn->unstomp();
        this->attachedBoard->chamberArray[this->x][this->y]=this->steppingOn;
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
            this->getStomper()->steppingOn=NULL;


        }
        else
        {
            bElem *newElem=new bElem(this->attachedBoard,this->x,this->y);
            newElem->setTeleporting(4);
            //this->attachedBoard->chamberArray[this->x][this->y]=newElem;
            //we remove the coordinates as well
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
    collected->setCollected(this);
    this->myInventory->addToInventory(collected);
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
    return false;
}

bool bElem::isDestroyed()
{
    return (this->destroyed>0);
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
    up=this->getAbsCoords(UP);
    left=this->getAbsCoords(LEFT);
    down=this->getAbsCoords(DOWN);
    right=this->getAbsCoords(RIGHT);

    myNeigh.nTypes[0]=this->getTypeInDirection(UP);
    myNeigh.steppable[0]=this->isSteppableDirection(UP);
    myNeigh.steppableClose[0]=myNeigh.steppable[0];
    if (up!=NOCOORDS and left!=NOCOORDS)
    {
        myNeigh.steppable[1]=this->getElementInDirection(UP)->isSteppableDirection(LEFT);
        myNeigh.nTypes[1]=this->getElementInDirection(UP)->getTypeInDirection(LEFT);

    }
    else
    {
        myNeigh.steppable[1]=false;
        myNeigh.nTypes[1]=-1;
    }
    myNeigh.steppableOther[0]=myNeigh.steppable[1];
    myNeigh.steppable[2]=this->isSteppableDirection(LEFT);
    myNeigh.steppableClose[1]=myNeigh.steppable[2];
    myNeigh.nTypes[2]=this->getTypeInDirection(LEFT);

    if (down!=NOCOORDS and left!=NOCOORDS)
    {
        myNeigh.steppable[3]=this->getElementInDirection(DOWN)->isSteppableDirection(LEFT);
        myNeigh.nTypes[3]=this->getElementInDirection(DOWN)->getTypeInDirection(LEFT);
    }
    else
    {
        myNeigh.steppable[3]=false;
        myNeigh.nTypes[3]=-1;
    }
    myNeigh.steppableOther[1]=myNeigh.steppable[3];
    myNeigh.steppable[4]=this->isSteppableDirection(DOWN);
    myNeigh.steppableClose[2]=myNeigh.steppable[4];
    myNeigh.nTypes[4]=this->getTypeInDirection(DOWN);
    if (down!=NOCOORDS and right!=NOCOORDS)
    {
        myNeigh.steppable[5]=this->getElementInDirection(DOWN)->isSteppableDirection(RIGHT);
        myNeigh.nTypes[5]=this->getElementInDirection(DOWN)->getTypeInDirection(RIGHT);
    }
    else
    {
        myNeigh.steppable[5]=false;
        myNeigh.nTypes[5]=-1;
    }
    myNeigh.steppableOther[2]=myNeigh.steppable[5];
    myNeigh.steppable[6]=this->isSteppableDirection(RIGHT);
    myNeigh.steppableClose[3]=myNeigh.steppable[6];
    myNeigh.nTypes[6]=this->getTypeInDirection(RIGHT);
    if (up!=NOCOORDS and right!=NOCOORDS)
    {
        myNeigh.steppable[7]=this->getElementInDirection(UP)->isSteppableDirection(RIGHT);
        myNeigh.nTypes[7]=this->getElementInDirection(UP)->getTypeInDirection(RIGHT);
    }
    else
    {
        myNeigh.steppable[7]=false;
        myNeigh.nTypes[7]=-1;
    }
    myNeigh.steppableOther[3]=myNeigh.steppable[5];
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
    for(unsigned int p=0; p<bElem::liveElems.size(); p++)
    {


        if(bElem::liveElems[p]->getCoords()!=NOCOORDS)
        {
            bElem::liveElems[p]->mechanics(false);
        }
        else if(bElem::liveElems[p]->getStomper()!=NULL || bElem::liveElems[p]->getCollector()!=NULL)
        {
            bElem::liveElems[p]->mechanics(true);
        }

    }
    bElem::tick();
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



