#include "bunker.h"
#include <elementFactory.h>

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






bool bunker::mechanics()
{
    bool res=bElem::mechanics();
    if(!res || this->getStats()->isMoving() || this->getStats()->isWaiting() || this->myGun->getStats()->isWaiting())
        return false;
    int randomTest=bElem::randomNumberGenerator()%55;
    if(randomTest>25)
    {
        this->help=0;
        this->myGun->use(shared_from_this());
    }
    for(int c=0; c<4; c++)
    {
        direction d=(direction)c;
        std::shared_ptr<bElem> e=this->getElementInDirection(d);
        int dd=0;
        while(e && ++dd<this->brange)
        {
            if(e->getType()==_player)
            {
                this->getStats()->setFacing(d);
                this->getStats()->setMyDirection(d);
                this->myGun->use(shared_from_this());
            }
            if (!e->getAttrs()->isSteppable())
            {
                break;
            }
            e=e->getElementInDirection(d);
        }
    }
    this->getStats()->setWaiting((1+bElem::randomNumberGenerator()%55)*5);
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
        while(element->getAttrs()->isSteppable()==true)
        {
            routes[dir]++;
            element=element->getElementInDirection((direction)(dir));
            if (element.get()==nullptr)
                break;
        }
        if (element)
        {
            if (element->getAttrs()->isKillable()==true)
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
        this->getStats()->setFacing(this->findLongestShot());
    return true;
}


int bunker::getType() const
{
    return _bunker;
}

