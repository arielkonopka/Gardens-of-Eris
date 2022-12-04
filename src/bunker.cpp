#include "bunker.h"

videoElement::videoElementDef* bunker::vd=nullptr;
bunker::bunker(chamber *board):mechanical(board), nonSteppable(board),movableElements(board),myGun(new plainGun(board,1))
{
//    this->setDirection(UP);

}
bunker::bunker(chamber* board, int x, int y):mechanical(board,x,y), nonSteppable(board,x,y),movableElements(board,x,y),myGun(new plainGun(board,1))
{
//    this->setCoords((coords){x,y});
//    this->setDirection(UP);
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
        this->activatedBy->unlockThisObject(this);
        this->activatedBy=nullptr;
    }
    delete this->myGun;
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
                this->activatedBy->unlockThisObject(this);
                this->activatedBy=nullptr;
            }
        }

    }

    int randomTest=bElem::randomNumberGenerator()%1000+this->help;

    if(this->myGun->readyToShoot()==false)
        return res;
    if(randomTest>990)
    {
        this->myGun->use(this);
    }
    return res;
}

bool bunker::interact(bElem* Who)
{
    if(mechanical::interact(Who)==false)
        return false;
    this->help=5555;
    if(Who->getStats()!=nullptr && this->activatedBy==nullptr)
    {
        this->activatedBy=Who;
        this->backUp=this->getStats();
        this->setStats(Who->getStats());
        Who->lockThisObject(this);
    }
    return true;
}

direction bunker::findLongestShot()
{
    int dir=0;
    int longest=0;
    direction longestDir=UP;
    bElem *element;
    int routes[]= {0,0,0,0};
    for(dir=0; dir<4; dir++)
    {
        element=this->getElementInDirection((direction)(dir));
        if(element==nullptr) continue;
        while(element->isSteppable()==true)
        {
            routes[dir]++;
            element=element->getElementInDirection((direction)(dir));
            if (element==nullptr)
                break;
        }
        if (element!=nullptr)
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
    this->setDirection(this->findLongestShot());
    return false;
}


