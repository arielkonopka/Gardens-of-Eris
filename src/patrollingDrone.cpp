#include "patrollingDrone.h"

videoElement::videoElementDef* patrollingDrone::vd=NULL;

patrollingDrone::patrollingDrone(chamber* board): killableElements(board), nonSteppable(board), mechanical(board), movableElements(board)
{
    this->setSubtype(0);
    this->myInventory=new inventory(this);
    this->setActive(false);
    this->steppables=new bool*[(_defaultRecurrenceDepth*2)+1];
    for(int c=0; c<(_defaultRecurrenceDepth*2)+1; c++)
    {
        this->steppables[c]=new bool[(_defaultRecurrenceDepth*2)+1];
    }
    this->boardSize.x=board->width;
    this->boardSize.y=board->height;

}

patrollingDrone::~patrollingDrone()
{
    for(int c=0; c<(_defaultRecurrenceDepth*2)+1; c++)
    {
        delete this->steppables[c];
    }
    delete this->steppables;
}

patrollingDrone::patrollingDrone(chamber* board, int x, int y):killableElements(board,x,y), nonSteppable(board,x,y), mechanical(board,x,y,false), movableElements(board,x,y)
{
    this->setSubtype(0);
    this->myInventory=new inventory(this);
    this->setActive(false);
    this->steppables=new bool*[(_defaultRecurrenceDepth*2)+1];
    for(int c=0; c<(_defaultRecurrenceDepth*2)+1; c++)
    {
        this->steppables[c]=new bool[(_defaultRecurrenceDepth*2)+1];
    }
    this->boardSize.x=board->width;
    this->boardSize.y=board->height;

}
void patrollingDrone::setVisited(int x, int y)
{
        coords mycrds=this->getCoords();
        int nx=mycrds.x-x+(_defaultRecurrenceDepth*2)+1;
        int ny=mycrds.y-y+(_defaultRecurrenceDepth*2)+1;
        if (nx<0 || nx>=(_defaultRecurrenceDepth*2)+1 || ny<0 || ny>=(_defaultRecurrenceDepth*2)+1)
            return;
        this->steppables[nx][ny]=true;
}

bool patrollingDrone::wasVisited(int x, int y)
{
        coords mycrds=this->getCoords();
        int nx=mycrds.x-x+(_defaultRecurrenceDepth*2)+1;
        int ny=mycrds.y-y+(_defaultRecurrenceDepth*2)+1;
        if (nx<0 || nx>=(_defaultRecurrenceDepth*2)+1 || ny<0 || ny>=(_defaultRecurrenceDepth*2)+1)
            return true;
        return this->steppables[nx][ny];
}

void patrollingDrone::clearVisited()
{
    for(int cx=0; cx<(_defaultRecurrenceDepth*2)+1; cx++)
            for(int cy=0; cy<(_defaultRecurrenceDepth*2)+1; cy++)
            {
                this->steppables[cx][cy]=false;
            }

}


int patrollingDrone::findSomething(bElem* elem,int n,int denyDir)
{
    int length=555; // this is too long route
    coords eCrds=elem->getCoords();
    if(n<0)
        return 555;
    this->setVisited(eCrds.x,eCrds.y);
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
            if(this->wasVisited(crds.x,crds.y))
                continue;
            int _len=this->findSomething(el,n-1,denyDir);
            if (_len<length)
                length=_len;
        }

    }

    return length+1;
}



bool patrollingDrone::mechanics()
{
    if(!killableElements::mechanics())
        return false;
    if(this->isActive()==false)
        return false;

    int length=555;
    direction dir=NODIRECTION;
    coords c=this->getCoords();
    for(int cnt=0; cnt<4; cnt++)
    {

        int l;
        direction d=(direction)cnt;
        bElem* el=this->getElementInDirection(d);
        if(el==NULL) continue;
        if(el->getType()==_key || el->getType()==_goldenAppleType || el->getType()==_plainGun)
        {
            this->collect(el);
            return true;
        }


        if(el!=NULL && el->isSteppable())
        {
            this->clearVisited();
            this->setVisited(c.x,c.y);
            l=this->findSomething(el,_defaultRecurrenceDepth,(direction)cnt);
            //    std::cout<<"Length "<<l<<" "<<d<<"\n";
            if(l<length)
            {
                length=l;
                dir=d;
            }

        }
    }

  //  std::cout<<"Length "<<length<<" "<<dir<<"\n";
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
    if(who->getType()!=_player)
        return false;
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
