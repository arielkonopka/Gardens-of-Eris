#include "bunker.h"
#include <elementFactory.h>
videoElement::videoElementDef* bunker::vd=nullptr;

bunker::bunker(std::shared_ptr<chamber> board):bunker()
{
    this->setBoard(board);
    this->myGun->setBoard(board);


}

bunker::bunker():mechanical(),movableElements(), myGun(elementFactory::generateAnElement<plainGun>(nullptr,1))
{

}
bool bunker::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool bunker::additionalProvisioning(int subtype, int typeId)
{
    this->registerLiveElement(shared_from_this());
    return bElem::additionalProvisioning(subtype,typeId);;
}

bool bunker::additionalProvisioning(int subtype, std::shared_ptr<bunker>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}



videoElement::videoElementDef* bunker::getVideoElementDef()
{
    return bunker::vd;
}

bunker::~bunker()
{
    if(this->activatedBy!=nullptr)
    {
        //  this->setStats(this->backUp);
        //    this->activatedBy->unlockThisObject(shared_from_this());
        this->activatedBy=nullptr;
    }
}


bool bunker::mechanics()
{
    bool res=bElem::mechanics();
    if(!res || this->status->isMoving() || this->status->isWaiting() || this->myGun->status->isWaiting())
        return false;
    int randomTest=bElem::randomNumberGenerator()%1000;
    if(randomTest>865)
    {
        this->help=0;
        this->myGun->use(shared_from_this());
    }
    return res;
}

bool bunker::interact(std::shared_ptr<bElem> Who)
{
    this->help=5555;
    return true;
}

direction bunker::findLongestShot()
{
    int dir=0;
    int longest=0;
    direction longestDir=UP;
    std::shared_ptr<bElem> element;
    int routes[]= {0,0,0,0};
    for(dir=0; dir<4; dir++)
    {
        element=this->getElementInDirection((direction)(dir));
        if(element.get()==nullptr) continue;
        while(element->attrs->isSteppable()==true)
        {
            routes[dir]++;
            element=element->getElementInDirection((direction)(dir));
            if (element.get()==nullptr)
                break;
        }
        if (element)
        {
            if (element->attrs->isKillable()==true)
            {
                routes[dir]=655350; // We shoot here, the place, where something to be killed stands at
            }
        }
        if(routes[dir]>longest)
        {
            longest=routes[dir];
            longestDir=(direction)((dir));
        }
    }
    return longestDir;
}


bool bunker::selfAlign()
{
    if(this->getBoard())
        this->status->setFacing(this->findLongestShot());
    return true;
}


int bunker::getType() const
{
    return _bunker;
}

