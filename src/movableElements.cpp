#include "movableElements.h"




movableElements::movableElements() :bElem()
{

}

movableElements::movableElements(std::shared_ptr<chamber> board) : movableElements()
{
    this->setBoard(board);
}


movableElements::~movableElements()
{
    //dtor
}


bool movableElements::moveInDirection(direction dir)
{
    return this->moveInDirectionSpeed(dir,_mov_delay);
}


bool movableElements::moveInDirectionSpeed(direction dir, int speed)
{
    std::shared_ptr<bElem> stepOn=this->getElementInDirection(dir);
    if (stepOn.get()==nullptr || this->getStats()->isMoving() || this->getStats()->isDying() || this->getStats()->isTeleporting() || this->getStats()->isDestroying() || dir==NODIRECTION)
        return false;
    this->getStats()->setMyDirection(dir);
    if (stepOn->getAttrs()->isSteppable()==true)
    {
        this->stepOnElement(stepOn);
        this->getStats()->setMoved(speed);
        this->playSound("Move","StepOn");
        return true;
    }
    if (this->getAttrs()->canPush()==true && stepOn->getAttrs()->canBePushed()==true && stepOn->getAttrs()->isMovable()==true)
    {
        std::shared_ptr<bElem> stepOn2=stepOn->getElementInDirection(dir);
        if(stepOn2.get()==nullptr || !stepOn2->getAttrs()->isSteppable())
        {
           this->playSound("Move","BlockedMove");
            return false;
        }
        if(stepOn->moveInDirectionSpeed(dir,speed+1)) //move next object in direction
        {
            this->stepOnElement(this->getElementInDirection(dir));  // move the initiating object
            this->getStats()->setMoved(speed+1);
            this->playSound("Move","StepOn");
        }
        return true;
    }
    if (this->getAttrs()->canCollect() && stepOn->getAttrs()->isCollectible()==true)
    {
        if (this->collect(stepOn)==true )
        {
            return true;
        }
    }

    if (this->getAttrs()->isInteractive()==true)
    {
        if(stepOn->interact(shared_from_this())==true)
        {
            return true;

        }
    }
    return false;
}


int movableElements::getType() const
{
    return _movableType;
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
    if(!draggedObj->getAttrs()->isMovable())
    {
        d2=(direction)((((int)this->getStats()->getMyDirection())+2)%4);
        draggedObj=this->getElementInDirection(d2);
        d2=this->getStats()->getMyDirection();
        if(draggedObj.get()==nullptr || !draggedObj->getAttrs()->isMovable())
            return false;
    }

    this->moveInDirectionSpeed(dragIntoDirection,speed);
    return draggedObj->moveInDirectionSpeed(d2,speed);

}

coords movableElements::getOffset() const
{
    coords res= {0,0};
    int rx=configManager::getInstance()->getConfig()->tileWidth;
    int ry=configManager::getInstance()->getConfig()->tileHeight;

    if(this->getStats()->isMoving() && this->getStats()->getMovingTotalTime()>0)
    {
        switch(this->getStats()->getMyDirection())
        {
        case(UP):
            res.y=((this->getStats()->getMoved())*ry)/this->getStats()->getMovingTotalTime();
            break;
        case (DOWN):
            res.y=-((this->getStats()->getMoved())*ry)/this->getStats()->getMovingTotalTime();
            break;
        case(LEFT):
            res.x=((this->getStats()->getMoved())*rx)/this->getStats()->getMovingTotalTime();
            break;
        case(RIGHT):
            res.x=-((this->getStats()->getMoved())*rx)/this->getStats()->getMovingTotalTime();
            break;
        case (NODIRECTION):
            res.x=0;
            res.y=0;
            break;

        }
    }

    return res;
}




bool movableElements::additionalProvisioning(int subtype, std::shared_ptr<movableElements>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool movableElements::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool movableElements::additionalProvisioning(int subtype,int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}










