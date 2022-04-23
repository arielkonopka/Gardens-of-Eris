#include "../include/bElem.h"

videoElement::videoElementDef* bElem::vd=NULL;
int bElem::instances=0;

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

bElem::bElem(chamber* board, gCollect* garbage)
{
    this->init();
    this->attachedBoard=board;
    this->garbageBin=garbage;

}
bool bElem::isUsable()
{
    return false;
}

bool bElem::isWeapon()
{
    return false;
}


bElem::bElem(chamber* board, gCollect* garbage, int x, int y)
{
    this->init();
    this->attachedBoard=board;
    this->garbageBin=garbage;
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

void bElem::init()
{
    this->attachedBoard=NULL;
    this->garbageBin=NULL;
    this->x=-1;
    this->y=-1;
    this->animPhase=0;
    this->setEnergy(_defaultEnergy); // this is rather a durability of an object, if it is killable;
    this->myStats.strength=0; // generic object has no strength
    this->myStats.dexterity=_initialDexterity; // We do not have the dexterity;
    this->killed=0;
    this->steppingOn=NULL;
    this->interacted=0;
    this->destroyed=0;
    this->instance=bElem::instances;
    bElem::instances++;
    this->collectedBy=NULL;
    this->myInventory=NULL;
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
    this->randomNumberGenerator.seed(seed);
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


void bElem::setBoard(chamber* board)
{
    this->attachedBoard=board;
}

void bElem::setGarbageBin(gCollect* garbage)
{
    this->garbageBin=garbage;
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


// the way it should operate is:
// release the field it is residingg and step on the new one (replace it on a board)
bool bElem::stepOnElement(bElem* step)
{
    if (this->attachedBoard==NULL || this->garbageBin==NULL || step->isSteppable()==false)  return false;
    if (this->steppingOn==NULL)
    {
        this->steppingOn=step;
    }
    else
    {
        this->attachedBoard->chamberArray[this->x][this->y]=this->steppingOn;
        this->steppingOn=step;
    }
    coords crds=this->steppingOn->getCoords();
    this->attachedBoard->chamberArray[crds.x][crds.y]=this;
    this->x=crds.x;
    this->y=crds.y;
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
    oState res;
    if ( this->garbageBin==NULL )
    {
#ifdef _debug
        std::cout<<"Tried to dispose broken element!\n";
#endif
        return ERROR;
    }
    if(this->steppingOn!=NULL && x>=0 && y>=0)
    {
        this->attachedBoard->chamberArray[this->x][this->y]=this->steppingOn; //steppingOn can be NULL, that could mean that someone would have to create an empty field here.
        this->steppingOn=NULL;
        res=DISPOSED;
    }
    else if (x>=0 && y>=0)
    {
        this->attachedBoard->chamberArray[this->x][this->y]=NULL;
        res=NULLREACHED;
    }
    else
    {
        res=DISPOSED;
    }
    this->garbageBin->addToBin(this); //add myself to to bin - this should be the only way of the object disposal!
    this->x=-1; //we set the state of the object to be unprovisioned - out of the game.
    this->y=-1;
    this->attachedBoard=NULL;
    this->garbageBin=NULL;
    return res; // false means that there is no more elements to go.
}

oState bElem::disposeElement()
{
    int x0=this->x;//save necessary data, becasue it will be lost after disposeElementUnsafe
    int y0=this->y;
    gCollect* myGarbage=this->garbageBin;
    chamber* myChamber=this->attachedBoard;
    if(this->disposeElementUnsafe()==NULLREACHED)
    {
        bElem* newElem=new bElem(myChamber,myGarbage);
        newElem->stepOnElement(newElem->attachedBoard->getElement(x0,y0));
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
    if (this->attachedBoard!=NULL && this->x!=-1 && this->y!=-1 && this->garbageBin!=NULL)
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
bool bElem::getActive()
{
    return true;
}




bool bElem::interact(bElem *who)
{
    if (who!=NULL)
    {
        who->interacted=_interactedTime;
    }
    return false;
}

bool bElem::destroy()
{
    if (this->canBeDestroyed()==true && this->isDestroyed()==false)
    {
        this->destroyed=_defaultDestroyTime;
        return true;
    }
    return false;
}


int bElem::getCnt()
{
    return 0;
}

int bElem::getAnimPh()
{
    // this->animPhase++;
    return this->animPhase;
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
    if(this->myInventory!=NULL)
    {
        this->myInventory->mechanics();
    }
    this->taterCounter++; //this is our source of sequential numbers
    if (this->steppingOn!=NULL)
    {
        this->steppingOn->mechanics(true);
    }
    this->animPhase++;

    if (this->destroyed>0) //We support two modes of destruction, this one is the demolishion thing
    {
        this->destroyed--;
        if(this->destroyed==0)
        {
            if (this->getType()!=_belemType) //We do not dispose the empty elements
                this->disposeElement();
            return true;
        }
    }
    if(this->canCollect()==true && this->myInventory!=NULL)
    {
        this->myInventory->mechanics();
    }
    return false;
}

bool bElem::isSteppable()
{
    return true;
}

bool bElem::canBeKilled()
{
    return false;
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
    return false;
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
        return NULL;
    }

    if (this->steppingOn!=NULL)
    {
        this->attachedBoard->chamberArray[this->x][this->y]=this->steppingOn;
        this->steppingOn=NULL;
        this->x=-1;
        this->y=-1;
        return this;
    }
    else
    {
        bElem *newElem=new bElem(this->attachedBoard,this->garbageBin,this->x,this->y);
        //we remove the coordinates as well
        this->x=-1;
        this->y=-1;
        this->attachedBoard->chamberArray[this->x][this->y]=newElem;
        return this;
    }
    return NULL; // Should never happen!
}

// this object cannot actively interact with outhers
bool bElem::canInteract()
{
    return false;
}

// Collect another element. The collectible contains location information. that way,
// we restore the element below the collectible, and the collectible will be stored in a vector structure.
// but for now it does nothing
bool bElem::collect(bElem *collectible)
{
    bElem *collected;
    if (this->canCollect()==false)
    {
        return false;
    }
    if (collectible==NULL)
    {
        return false;
    }
    collected=collectible->removeElement();
    if (collected==NULL) //this should never happen!
    {
#ifdef _debug
        std::cout<<"Collecting failed!\n";
#endif
        return false;
    }
#ifdef _debug
    std::cout<<"Collect "<<collected->getType()<<" st: "<<collected->getSubtype()<<"\n";
#endif
    collected->setCollector(this);
    this->myInventory->addToInventory(collected);
    return true;
}

bElem* bElem::getCollector()
{
    return this->collectedBy;
}
void bElem::setCollector(bElem* collector)
{
    this->collectedBy=collector;
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
    return this->myStats.energy;
}

bool bElem::setEnergy(int points)
{
    this->myStats.energy=(points>=0)?points:0;
    return (points==0);
}

bool bElem::kill()
{
    return false;
}

bool bElem::isDestroyed()
{
    return (this->destroyed>0);
}

stats bElem::getStats()
{
    return myStats;
}
void bElem::setStats(stats newStats)
{
    this->myStats=newStats;

}

bool bElem::isMod()
{
    return false;
}

modType bElem::getModType()
{
    return NONE;
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
    myNeigh.steppable[0]=this->isSteppableDirection(UP);
    myNeigh.steppableClose[0]=myNeigh.steppable[0];
    if (up!=NOCOORDS and left!=NOCOORDS)
    {
        myNeigh.steppable[1]=this->getElementInDirection(UP)->isSteppableDirection(LEFT);
    }
    else
    {
        myNeigh.steppable[1]=false;
    }
    myNeigh.steppableOther[0]=myNeigh.steppable[1];
    myNeigh.steppable[2]=this->isSteppableDirection(LEFT);
    myNeigh.steppableClose[1]=myNeigh.steppable[2];

    if (down!=NOCOORDS and left!=NOCOORDS)
    {
        myNeigh.steppable[3]=this->getElementInDirection(DOWN)->isSteppableDirection(LEFT);
    }
    else
    {
        myNeigh.steppable[3]=false;
    }
    myNeigh.steppableOther[1]=myNeigh.steppable[3];
    myNeigh.steppable[4]=this->isSteppableDirection(DOWN);
    myNeigh.steppableClose[2]=myNeigh.steppable[4];
    if (down!=NOCOORDS and right!=NOCOORDS)
    {
        myNeigh.steppable[5]=this->getElementInDirection(DOWN)->isSteppableDirection(RIGHT);
    }
    else
    {
        myNeigh.steppable[5]=false;
    }
    myNeigh.steppableOther[2]=myNeigh.steppable[5];
    myNeigh.steppable[6]=this->isSteppableDirection(RIGHT);
    myNeigh.steppableClose[3]=myNeigh.steppable[6];
    if (up!=NOCOORDS and right!=NOCOORDS)
    {
        myNeigh.steppable[7]=this->getElementInDirection(UP)->isSteppableDirection(RIGHT);
    }
    else
    {
        myNeigh.steppable[7]=false;
    }
    myNeigh.steppableOther[3]=myNeigh.steppable[5];
    return myNeigh;

}

