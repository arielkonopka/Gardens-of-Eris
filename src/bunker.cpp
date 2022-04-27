#include "bunker.h"

videoElement::videoElementDef* bunker::vd=NULL;
bunker::bunker(chamber *board,gCollect *garbageBin):nonSteppable(board,garbageBin)
{
    this->myGun=new plainGun(this->attachedBoard,this->garbageBin);
    this->myGun->setSubtype(1); // we have a gun, and we are not afraid to use it.
    this->setDirection(UP);
    this->rotated=0;
    this->interacted=-1;
    this->help=0;
}
bunker::bunker(chamber* board, gCollect* garbageBin, int x, int y):nonSteppable(board,garbageBin,x,y)
{
    this->myGun=new plainGun(this->attachedBoard,this->garbageBin);
    this->myGun->setSubtype(1); // we have a gun, and we are not afraid to use it.
    this->setDirection(UP);
    this->rotated=0;
    this->interacted=-1;
    this->help=0;
}

videoElement::videoElementDef* bunker::getVideoElementDef()
{
    return bunker::vd;
}

bunker::~bunker()
{
    delete this->myGun;
}

bool bunker::isMovable()
{
    return true;
}


bool bunker::mechanics(bool collected)
{
    bool res=nonSteppable::mechanics(collected);
    if(this->help>0)
        this->help--;
    int randomTest=this->randomNumberGenerator()%1000+this->help;
    res=res || this->myGun->mechanics(collected);
    if(this->myGun->readyToShoot()==false)
        return res;
    if(randomTest>990)
        this->myGun->use(this);
    return true;
}

bool bunker::interact(bElem* Who)
{
    if(this->interacted>0)
        return false;
    this->help=500;
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
        if(element==NULL) continue;
        while(element->isSteppable()==true)
        {
            routes[dir]++;
            element=element->getElementInDirection((direction)(dir));
            if (element==NULL)
                break;
        }
        if (element!=NULL)
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


