#include "movableElements.h"


videoElement::videoElementDef* movableElements::vd=NULL;



movableElements::movableElements(chamber *board,gCollect *garbage) : nonSteppable(board,garbage)
{
    this->_me_moved=0;
    this->_me_canPush=false;
    this->movable=true;
}
movableElements::movableElements(chamber *board,gCollect *garbage,int x, int y) : nonSteppable(board,garbage,x,y)
{
    this->_me_moved=0;
    this->_me_canPush=false;
    this->movable=true;
}

movableElements::~movableElements()
{
    //dtor
}
bool movableElements::isMovable()
{
    return this->movable;
}

bool movableElements::moveInDirection(direction dir)
{
    return this->moveInDirectionSpeed(dir,_mov_delay);
}

bool movableElements::moveInDirectionSpeed(direction dir, int speed)
{
    if (this->_me_moved>0 || this->isDying()==true) return false;
    this->setDirection(dir);
    coords ncoord=this->getAbsCoords(dir);
    if (ncoord==NOCOORDS) return false;
    if (this->attachedBoard->chamberArray[ncoord.x][ncoord.y]->isSteppable()==true)
    {
        this->stepOnElement(this->attachedBoard->chamberArray[ncoord.x][ncoord.y]);
        this->_me_moved=speed;
        if (this->getDirection()==RIGHT || this->getDirection()==DOWN) this->_me_moved++; // we will process that twice in one iteration, we do not want that
        return true;
    }
    else if (this->canPush()==true && this->attachedBoard->chamberArray[ncoord.x][ncoord.y]->isMovable()==true)
    {
        coords ncoord2=this->attachedBoard->chamberArray[ncoord.x][ncoord.y]->getAbsCoords(dir);
        if (ncoord2==NOCOORDS) return false;
        if (this->attachedBoard->chamberArray[ncoord2.x][ncoord2.y]->isSteppable())
        {
            this->attachedBoard->chamberArray[ncoord.x][ncoord.y]->stepOnElement(this->attachedBoard->chamberArray[ncoord2.x][ncoord2.y]); //move next object in direction
            this->stepOnElement(this->attachedBoard->chamberArray[ncoord.x][ncoord.y]);  // move the initiating object
            this->_me_moved=speed+1;
            if (this->getDirection()==RIGHT || this->getDirection()==DOWN) this->_me_moved++; //we will process this twice during movement right or down, it is because how we iterate throught the world.
            return true;
        }
    }
    if (this->canCollect()==true && this->myInventory!=NULL && this->attachedBoard->chamberArray[ncoord.x][ncoord.y]->isCollectible()==true)
    {
        if (this->myInventory->addToInventory(this->attachedBoard->chamberArray[ncoord.x][ncoord.y])==true)
        //if (this->collect(this->attachedBoard->chamberArray[ncoord.x][ncoord.y])==true)
        {
            this->_me_moved=speed;
            if (this->getDirection()==RIGHT || this->getDirection()==DOWN) this->_me_moved++;
            return true;
        }
    }
    if (this->canInteract()==true && this->attachedBoard->chamberArray[ncoord.x][ncoord.y]->isInteractive()==true)
    {
        if(this->attachedBoard->chamberArray[ncoord.x][ncoord.y]->interact(this)==true)
        {
            this->_me_moved=speed;
            if (this->getDirection()==RIGHT || this->getDirection()==DOWN) this->_me_moved++;
            return true;

        }
    }
    return false;
}


int movableElements::getType()
{
    return _movableType;
}

int movableElements::getSubtype()
{
    return 0;
}

bool movableElements::canPush()
{
    return _me_canPush;
}

bool movableElements::mechanics(bool collected)
{
    nonSteppable::mechanics(collected);
    if (this->_me_moved>0)
    {
        this->_me_moved--;
        return false;
    }

    return true;
}



videoElement::videoElementDef* movableElements::getVideoElementDef()
{
    return movableElements::vd;
}


direction movableElements::getDirection()
{

    return nonSteppable::getDirection();
}

bool movableElements::setDirection(direction newDirection)
{
    nonSteppable::setDirection(newDirection);
    if(newDirection==DOWN||newDirection==RIGHT)
        this->_me_moved++;
    return true;
}


void movableElements::setMoved(int time)
{
    this->_me_moved=time;
}
