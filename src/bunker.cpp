#include "bunker.h"

videoElement::videoElementDef* bunker::vd=NULL;
bunker::bunker(chamber *board,gCollect *garbageBin):nonSteppable(board,garbageBin)
{
    this->myGun=new plainGun(this->attachedBoard,this->garbageBin);
    this->myGun->setSubtype(1); // we have a gun, and we are not afraid to use it.
    this->setDirection(UP);
    this->rotated=0;
}
bunker::bunker(chamber* board, gCollect* garbageBin, int x, int y):nonSteppable(board,garbageBin,x,y)
{
    this->myGun=new plainGun(this->attachedBoard,this->garbageBin);
    this->myGun->setSubtype(1); // we have a gun, and we are not afraid to use it.
    this->setDirection(UP);
    this->rotated=0;
}

videoElement::videoElementDef* bunker::getVideoElementDef()
{
    return bunker::vd;
}

bunker::~bunker()
{
    delete this->myGun;
}
bool bunker::mechanics(bool collected)
{
    direction myNewDirection=LEFT;
    bool res=nonSteppable::mechanics(collected);
    res=res || this->myGun->mechanics(collected);
    if (this->rotated>0)
    {
        this->rotated--;
        return res;
    }
    if(this->myGun->readyToShoot()==false)
        return res;
 //   std::cout<<"shoot!\n";

    myNewDirection=this->findLongestShot();

    if (this->getDirection()!=myNewDirection)
    {
        int a=(int)(this->getDirection());
        int b=(int)(myNewDirection);
        if (a>b) a--;
        if (a<b) a++;
        this->setDirection((direction)(a%4));
        this->rotated=_mov_delay;
        return true;
    }

    this->myGun->use(this);
    this->rotated=50;
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
        element=this;
        while(element->isSteppableDirection((direction)(dir))==true)
        {
            routes[dir]++;
        //    std::cout<<"dir "<<dir<<".";
            element=element->getElementInDirection((direction)(dir));
            if (element==NULL)
                break;
        }
        if (element!=NULL)
        {
            if (element->canBeKilled()==true)
            {
                routes[dir]=65535; // We shoot here, the place, where something to be killed stands at
            }
        }
        if(routes[dir]>longest)
        {
            longest=routes[dir];
            longestDir=(direction)((dir%4));
        }
    }
    return longestDir;
}



