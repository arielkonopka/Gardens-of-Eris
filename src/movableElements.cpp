#include "movableElements.h"


videoElement::videoElementDef* movableElements::vd=NULL;



movableElements::movableElements(chamber *board) : mechanical(board)
{
    this->_me_moved=0;
    this->_me_canPush=false;
    this->movable=true;
}
movableElements::movableElements(chamber *board,int x, int y) : mechanical(board,x,y)
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
    bElem* stepOn=this->getElementInDirection(dir);
    if (this->_me_moved>0 || this->isDying()==true) return false;
    this->setDirection(dir);
    if (stepOn==NULL) return false;
    if (stepOn->isSteppable()==true)
    {
        this->stepOnElement(stepOn);
        this->_me_moved=speed;
        return true;
    }
    else if (this->canPush()==true && stepOn->isMovable()==true)
    {
        bElem* stepOn2=stepOn->getElementInDirection(dir);
        if(stepOn2==NULL)
            return false;
        if (stepOn2->isSteppable())
        {
            stepOn->stepOnElement(stepOn2); //move next object in direction
            this->stepOnElement(this->getElementInDirection(dir));  // move the initiating object
            this->_me_moved=speed+1;
            return true;
        }
    }
    if (this->canCollect()==true && this->myInventory!=NULL && stepOn->isCollectible()==true)
    {
        if (this->myInventory->addToInventory(stepOn)==true)
        //if (this->collect(this->attachedBoard->chamberArray[ncoord.x][ncoord.y])==true)
        {
            this->_me_moved=speed;
            return true;
        }
    }
    if (this->canInteract()==true && stepOn->isInteractive()==true)
    {
        if(stepOn->interact(this)==true)
        {
            this->_me_moved=speed;
            return true;

        }
    }
    return false;
}


int movableElements::getType()
{
    return _movableType;
}
/*
int movableElements::getSubtype()
{
    return 0;
}
*/
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

    return true;
}


void movableElements::setMoved(int time)
{
    this->_me_moved=time;
}

int movableElements::getMoved()
{
    return this->_me_moved;
}


bool movableElements::dragInDirection(direction dragIntoDirection)
{
    direction objFromDir=(direction)(((int)dragIntoDirection)+2);
    bElem *draggedObj=this->getElementInDirection(objFromDir);
    if(!draggedObj->isSteppable())
        return false;
    return draggedObj->moveInDirection(dragIntoDirection);

}
















