#include "patrollingDrone.h"

videoElement::videoElementDef* patrollingDrone::vd=NULL;

patrollingDrone::patrollingDrone(chamber* board): killableElements(board,false)
{
    this->setSubtype(0);
    this->movable=true;
    this->myInventory=new inventory(this);
    this->setActive(false);
    this->steppables=new bool*[board->height];
    for(int c=0; c<board->height; c++)
    {
        this->steppables[c]=new bool[board->width];
    }
    this->boardSize.x=board->width;
    this->boardSize.y=board->height;

}

patrollingDrone::~patrollingDrone()
{
    for(int c=0; c<this->boardSize.y; c++)
    {
        delete this->steppables[c];
    }
    delete this->steppables;
}

patrollingDrone::patrollingDrone(chamber* board, int x, int y):killableElements(board,x,y,false)
{
    this->setSubtype(0);
    this->movable=true;
    this->myInventory=new inventory(this);
    this->setActive(false);
    this->steppables=new bool*[board->height];
    for(int c=0; c<board->height; c++)
    {
        this->steppables[c]=new bool[board->width];
    }
    this->boardSize.x=board->width;
    this->boardSize.y=board->height;

}


int patrollingDrone::findSomething(bElem* elem,int n,int denyDir)
{
    int length=555; // this is too long route
    coords eCrds=elem->getCoords();
    if(n<0)
        return 555;
    this->steppables[eCrds.y][eCrds.x]=true;
    for(int d=0; d<4; d++)
    {
        bElem* el=elem->getElementInDirection((direction)d);
        if(el!=NULL)
        {
            if(el->getType()==_goldenAppleType || el->getType()==_key || el->getType()==_plainGun)
                return 0;
        }
    }
    //ok, we are here, because we should look harder
    for(int d=0; d<4; d++)
    {
        bElem* el=elem->getElementInDirection((direction)((d+denyDir)%4));

        if(el!=NULL && el->isSteppable())
        {
            coords crds=el->getCoords();
            if(this->steppables[crds.y][crds.x])
                continue;
            int _len=this->findSomething(el,n-1,denyDir);
            if (_len<length)
                length=_len;
        }

    }

    return length+1;
}



bool patrollingDrone::mechanics(bool collected)
{
    if(!killableElements::mechanics(collected))
        return false;
    if(this->isActive()==false)
        return false;

    int length=555;
    direction dir=NODIRECTION;
    coords c=this->getCoords();
    this->steppables[c.y][c.x]=true;
    for(int cnt=0; cnt<4; cnt++)
    {

        int l;
        direction d=(direction)cnt;
        bElem* el=this->getElementInDirection(d);
        if(el->getType()==_key || el->getType()==_goldenAppleType || el->getType()==_plainGun)
        {
            myInventory->addToInventory(el);
            return true;
        }
        for(int cx=0; cx<this->getBoard()->width; cx++)
            for(int cy=0; cy<this->getBoard()->height; cy++)
            {
                this->steppables[cy][cx]=false;
            }

        if(el!=NULL && el->isSteppable())
        {

            l=this->findSomething(el,25,(direction)cnt);
            //    std::cout<<"Length "<<l<<" "<<d<<"\n";
            if(l<length)
            {
                length=l;
                dir=d;
            }

        }
    }

    std::cout<<"Length "<<length<<" "<<dir<<"\n";
    if(dir!=NODIRECTION && length<50 && dir!=(direction)(((int)this->getDirection()+2)%4))
    {
        this->moveInDirection(dir);
    }
    else
    {
        int newDirection=this->randomNumberGenerator()%3; // 0 - forward, 1 - forward, 2 - right
        newDirection=(newDirection==2)?3:newDirection;
        direction newD=(direction)(((int)this->getDirection()+newDirection)%4);
        this->moveInDirection(newD);
    }
    this->setWait(_mov_delay*2);
    return true;


}




bool patrollingDrone::interact(bElem* who)
{
    bool res=killableElements::interact(who);
    if (!res) return false;
    if(!this->isLiveElement())
        this->registerLiveElement(this);
    this->setActive(true);
    return true;

}



int patrollingDrone::getType()
{
    return _patrollingDrone;
}

videoElement::videoElementDef* patrollingDrone::getVideoElementDef()
{
    return patrollingDrone::vd;
}
