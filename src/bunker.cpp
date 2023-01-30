#include "bunker.h"

videoElement::videoElementDef* bunker::vd=nullptr;

bunker::bunker(std::shared_ptr<chamber> board):mechanical(board), nonSteppable(board),movableElements(board),
    myGun(elementFactory::generateAnElement<plainGun>(board,1))
{

}

bunker::bunker():mechanical(),nonSteppable(),movableElements(), myGun(nullptr)
{

}



videoElement::videoElementDef* bunker::getVideoElementDef()
{
    return bunker::vd;
}

bunker::~bunker()
{
    if(this->activatedBy!=nullptr)
    {
        this->setStats(this->backUp);
        //    this->activatedBy->unlockThisObject(shared_from_this());
        this->activatedBy=nullptr;
    }
}

bool bunker::isMovable()
{
    return true;
}


bool bunker::mechanics()
{
    bool res=mechanical::mechanics();
    if(!res)
        return false;
    if(this->help>0)
    {
        this->help--;
        if (this->help==0)
        {
            if(this->activatedBy!=nullptr)
            {
                this->setStats(this->backUp);
                this->activatedBy->unlockThisObject(shared_from_this());
                this->activatedBy=nullptr;
            }
        }

    }

    int randomTest=bElem::randomNumberGenerator()%1000+this->help;

    if(this->myGun->readyToShoot()==false)
        return res;
    if(randomTest>990)
    {
        this->myGun->use(shared_from_this());
    }
    return res;
}

bool bunker::interact(std::shared_ptr<bElem> Who)
{
    if(mechanical::interact(Who)==false)
        return false;
    this->help=5555;
    if(Who->getStats().get()!=nullptr && this->activatedBy.get()==nullptr)
    {
        this->activatedBy=Who;
        this->backUp=this->getStats();
        this->setStats(Who->getStats());
        Who->lockThisObject(shared_from_this());
    }
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
        while(element->isSteppable()==true)
        {
            routes[dir]++;
            element=element->getElementInDirection((direction)(dir));
            if (element.get()==nullptr)
                break;
        }
        if (element.get()!=nullptr)
        {
            if (element->canBeKilled()==true)
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
        this->setDirection(this->findLongestShot());
    return true;
}


int bunker::getType()
{
    return _bunker;
}

