#include "movableElements.h"


videoElement::videoElementDef* movableElements::vd=nullptr;



movableElements::movableElements(chamber *board) : bElem(board)
{

}
movableElements::movableElements(chamber *board,int x, int y) : bElem(board,x,y)
{

}

movableElements::~movableElements()
{
    //dtor
}
bool movableElements::isMovable()
{
    return this->movable && !this->isDestroyed() && !this->isDying();
}

bool movableElements::moveInDirection(direction dir)
{
    return this->moveInDirectionSpeed(dir,_mov_delay);
}

bool movableElements::moveInDirectionSpeed(direction dir, int speed)
{
    bElem* stepOn=this->getElementInDirection(dir);
    if (stepOn==nullptr || this->getMoved()>0 || this->isDying() || this->isTeleporting() || this->isDestroyed() )
        return false;
    this->setDirection(dir);
    if (stepOn->isSteppable()==true)
    {
        this->stepOnElement(stepOn);
        this->setMoved(speed);
        return true;
    }
    else if (this->canPush()==true && stepOn->isMovable()==true)
    {
        bElem* stepOn2=stepOn->getElementInDirection(dir);
        if(stepOn2==nullptr)
            return false;
        if (stepOn2->isSteppable())
        {
            stepOn->stepOnElement(stepOn2); //move next object in direction
            this->stepOnElement(this->getElementInDirection(dir));  // move the initiating object
            this->setMoved(speed+1);
            stepOn->setMoved(speed+1);
            stepOn->setDirection(dir);
            return true;
        }
    }
    if (this->canCollect()==true && this->getInventory()!=nullptr && stepOn->isCollectible()==true)
    {
        if (this->collect(stepOn)==true && this->getStats())
            //if (this->collect(this->attachedBoard->chamberArray[ncoord.x][ncoord.y])==true)
        {
            this->getStats()->countCollect(stepOn);
            // this->setMoved(speed);
            return true;
        }
    }
    if (this->canInteract()==true && stepOn->isInteractive()==true)
    {
        if(stepOn->interact(this)==true)
        {
            // this->setMoved(speed);
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



videoElement::videoElementDef* movableElements::getVideoElementDef()
{
    return movableElements::vd;
}




void movableElements::setMoved(int time)
{
    this->_me_moved=this->getCntr()+time;
    this->movingTotalTime=time;
}

int movableElements::getMoved()
{
    if(this->isTeleporting()) return 0;
    if(this->_me_moved-this->getCntr()>50)
        this->_me_moved=0;
    return (this->_me_moved<this->getCntr())?0:this->_me_moved-this->getCntr();
}


bool movableElements::dragInDirection(direction dragIntoDirection)
{
    direction objFromDir=(direction)(((int)dragIntoDirection)+2);
    bElem *draggedObj=this->getElementInDirection(objFromDir);
    if(!draggedObj->isSteppable())
        return false;
    return draggedObj->moveInDirection(dragIntoDirection);

}

coords movableElements::getOffset()
{
    coords res= {0,0};
    if(this->getMoved()>0 && this->movingTotalTime>0)
    {
        switch(this->getDirection())
        {
        case(UP):
            res.y=((this->getMoved())*64)/this->movingTotalTime;
            break;
        case (DOWN):
            res.y=-((this->getMoved())*64)/this->movingTotalTime;
            break;
        case(LEFT):
            res.x=((this->getMoved())*64)/this->movingTotalTime;
            break;
        case(RIGHT):
            res.x=-((this->getMoved())*64)/this->movingTotalTime;
            break;
        case (NODIRECTION):
            break;

        }
    }

    return res;
}















