#include "movableElements.h"


videoElement::videoElementDef* movableElements::vd=nullptr;


movableElements::movableElements():audibleElement()
{

}

movableElements::movableElements(std::shared_ptr<chamber> board) : audibleElement(board)
{

}


movableElements::~movableElements()
{
    //dtor
}
bool movableElements::isMovable()
{
    return this->movable && !this->isDestroyed() && !this->isDying() && !this->isTeleporting();
}

bool movableElements::moveInDirection(direction dir)
{
    return this->moveInDirectionSpeed(dir,_mov_delay);
}


bool movableElements::moveInDirectionSpeed(direction dir, int speed)
{
    std::shared_ptr<bElem> stepOn=this->getElementInDirection(dir);
    if (stepOn.get()==nullptr || this->getMoved()>0 || this->isDying() || this->isTeleporting() || this->isDestroyed() || dir==NODIRECTION)
        return false;
    this->setDirection(dir);
    if (stepOn->isSteppable()==true)
    {
        this->stepOnElement(stepOn);
        this->setMoved(speed);
        this->playSound("Move","StepOn");
        return true;
    }
    else if (this->canPush()==true && stepOn->isMovable()==true)
    {
        std::shared_ptr<bElem> stepOn2=stepOn->getElementInDirection(dir);
        if(stepOn2.get()==nullptr || !stepOn2->isSteppable())
        {
           this->playSound("Move","BlockedMove");
            return false;
        }
        stepOn->moveInDirectionSpeed(dir,speed+1); //move next object in direction
        this->stepOnElement(this->getElementInDirection(dir));  // move the initiating object
        this->setMoved(speed+1);
        this->playSound("Move","StepOn");
        return true;
    }
    if (this->canCollect()==true && this->getInventory().get()!=nullptr && stepOn->isCollectible()==true)
    {
        if (this->collect(stepOn)==true && this->getStats().get()!=nullptr)
        {
            this->getStats()->countCollect(stepOn);
            return true;
        }
    }
    if (this->canInteract()==true)
    {
        if(stepOn->interact(shared_from_this())==true)
        {
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
   return this->dragInDirectionSpeed(dragIntoDirection,_mov_delay*2);
}


bool movableElements::dragInDirectionSpeed(direction dragIntoDirection, int speed)
{
    direction objFromDir=(direction)((((int)dragIntoDirection)+2)%4);
    direction d2=dragIntoDirection;
    std::shared_ptr<bElem> draggedObj=this->getElementInDirection(objFromDir);
    if(draggedObj.get()==nullptr)
        return false;
    if(!draggedObj->isMovable())
    {
        d2=(direction)((((int)this->getDirection())+2)%4);
        draggedObj=this->getElementInDirection(d2);
        d2=this->getDirection();
        if(draggedObj.get()==nullptr || !draggedObj->isMovable())
            return false;
    }

    this->moveInDirectionSpeed(dragIntoDirection,speed);
    return draggedObj->moveInDirectionSpeed(d2,speed);

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


void movableElements::setMovable(bool m)
{
    this->movable=m;
}



void movableElements::setCanPush(bool sp)
{
    this->_me_canPush=sp;
}












