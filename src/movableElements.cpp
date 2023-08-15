#include "movableElements.h"


videoElement::videoElementDef* movableElements::vd=nullptr;


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
    if (stepOn.get()==nullptr || this->status->isMoving() || this->status->isDying() || this->status->isTeleporting() || this->status->isDestroying() || dir==NODIRECTION)
        return false;
    this->status->setMyDirection(dir);
    if (stepOn->attrs->isSteppable()==true)
    {
        this->stepOnElement(stepOn);
        this->status->setMoved(speed);
        this->playSound("Move","StepOn");
        return true;
    }
    else if (this->attrs->canPush()==true && stepOn->attrs->canBePushed()==true && stepOn->attrs->isMovable()==true)
    {
        std::shared_ptr<bElem> stepOn2=stepOn->getElementInDirection(dir);
        if(stepOn2.get()==nullptr || !stepOn2->attrs->isSteppable())
        {
           this->playSound("Move","BlockedMove");
            return false;
        }
        if(stepOn->moveInDirectionSpeed(dir,speed+1)) //move next object in direction
        {
            this->stepOnElement(this->getElementInDirection(dir));  // move the initiating object
            this->status->setMoved(speed+1);
            this->playSound("Move","StepOn");
        }
        return true;
    }
  /*  if (this->attrs->canCollect() && stepOn->attrs->isCollectible()==true)
    {
        if (this->collect(stepOn)==true )
        {
            this->getStats()->countCollect(stepOn);
            return true;
        }
    }
    */
    if (this->attrs->isInteractive()==true)
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
/*
int movableElements::getSubtype()
{
    return 0;
}
*/




videoElement::videoElementDef* movableElements::getVideoElementDef()
{
    return movableElements::vd;
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
    if(!draggedObj->attrs->isMovable())
    {
        d2=(direction)((((int)this->status->getMyDirection())+2)%4);
        draggedObj=this->getElementInDirection(d2);
        d2=this->status->getMyDirection();
        if(draggedObj.get()==nullptr || !draggedObj->attrs->isMovable())
            return false;
    }

    this->moveInDirectionSpeed(dragIntoDirection,speed);
    return draggedObj->moveInDirectionSpeed(d2,speed);

}

coords movableElements::getOffset() const
{
    coords res= {0,0};
    if(this->status->isMoving() && this->status->getMovingTotalTime()>0)
    {
        switch(this->status->getMyDirection())
        {
        case(UP):
            res.y=((this->status->getMoved())*64)/this->status->getMovingTotalTime();
            break;
        case (DOWN):
            res.y=-((this->status->getMoved())*64)/this->status->getMovingTotalTime();
            break;
        case(LEFT):
            res.x=((this->status->getMoved())*64)/this->status->getMovingTotalTime();
            break;
        case(RIGHT):
            res.x=-((this->status->getMoved())*64)/this->status->getMovingTotalTime();
            break;
        case (NODIRECTION):
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










