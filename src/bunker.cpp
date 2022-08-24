#include "bunker.h"

videoElement::videoElementDef* bunker::vd=NULL;
bunker::bunker(chamber *board):mechanical(board)
{
    this->myGun=new plainGun(board,1);
    this->setDirection(UP);
    this->rotated=0;
    this->interacted=-1;
    this->help=0;
    this->activatedBy=NULL;
}
bunker::bunker(chamber* board, int x, int y):mechanical(board,x,y)
{
    this->myGun=new plainGun(board,1);
    this->setDirection(UP);
    this->rotated=0;
    this->interacted=-1;
    this->help=0;
    this->activatedBy=NULL;
}


videoElement::videoElementDef* bunker::getVideoElementDef()
{
    return bunker::vd;
}

bunker::~bunker()
{
    if(this->activatedBy!=NULL)
    {
        this->setStats(this->backUp);
        this->activatedBy->unlockThisObject(this);
        this->activatedBy=NULL;
    }
    delete this->myGun;
}

bool bunker::isMovable()
{
    return true;
}


bool bunker::mechanics(bool collected)
{
    bool res=nonSteppable::mechanics(collected);
    if(res)
    {
        if(this->help>0)
        {
            this->help--;
            if (this->help==0)
            {
                if(this->activatedBy!=NULL)
                {
                    this->setStats(this->backUp);
                    this->activatedBy->unlockThisObject(this);
                    this->activatedBy=NULL;
                }
            }

        }

        int randomTest=bElem::randomNumberGenerator()%1000+this->help;

        if(this->myGun->readyToShoot()==false)
            return res;
        if(randomTest>990)
        {

            this->myGun->use(this);
            this->setMoved(_plainGunCharge);
        }
    }
    return res;
}

bool bunker::interact(bElem* Who)
{
    if(this->interacted>0)
        return false;
    this->help=5555;
    if(Who->getStats()!=NULL && this->activatedBy==NULL)
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


