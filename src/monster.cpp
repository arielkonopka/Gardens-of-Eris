#include "monster.h"

// the static variable must be initialized
videoElement::videoElementDef* monster::vd=NULL;


monster::monster(chamber *board): killableElements(board), nonSteppable(board), mechanical(board), movableElements(board)
{
    this->animph=0;
    this->setInventory(new inventory(this));
    this->internalCnt=0;
    this->setDirection(UP);
    this->setSubtype(0);
    this->inited=false;
    if(bElem::randomNumberGenerator()%10==0)
    {
        plainGun* pg=new plainGun(board);
        pg->setSubtype(1);
        this->collect(pg);
    }
    if(bElem::randomNumberGenerator()%2==0)
    {
        this->rotA=1;
        this->rotB=3;
    }
    else
    {
        this->rotA=3;
        this->rotB=1;
    }

}

monster::monster(chamber* board, int newSubtype): killableElements(board), nonSteppable(board), mechanical(board), movableElements(board)
{
    this->setInventory(new inventory(this));
    this->animph=0;
    this->internalCnt=0;
    this->setDirection(UP);
    this->setSubtype(newSubtype);
    this->inited=false;
    if(bElem::randomNumberGenerator()%2==0)
    {
        this->rotA=1;
        this->rotB=3;
    }
    else
    {
        this->rotA=3;
        this->rotB=1;
    }

}


monster::~monster()
{
    //dtor
}
videoElement::videoElementDef* monster::getVideoElementDef()
{
    return monster::vd;
}
int monster::getType()
{
    return _monster;
}

bool monster::checkNeigh()
{
    bool r=false;
    for(int c=0; c<4; c++)
    {
        direction d=(direction)c;
        bElem* e=this->getElementInDirection(d);;
        if(e==NULL)
            continue;
        if(e->isCollectible())
        {
            this->collect(e);
            this->setWait(_mov_delay);
            r=true;
            continue;
        }
        if(e->getType()==_player)
        {
            e->hurt(5);
            r=true;
            continue;
        }

        if(this->myInventory->getActiveWeapon()!=NULL)
        {
            while(e!=NULL)
            {
                if(e->getType()==_player && this->myInventory->getActiveWeapon()!=NULL && e->isActive())
                {
                    direction td=this->getDirection();
                    this->setDirection(d);
                    this->myInventory->getActiveWeapon()->use(this);
                    this->setDirection(td);
                    this->setWait(_mov_delay);
                    break;

                }
                if(!e->isSteppable() || e->getElementInDirection(d)==NULL)
                    break;
                e=e->getElementInDirection(d);
            }
        }
    }
    return r;
}



bool monster::mechanics()
{

    direction newDir=NODIRECTION;
    direction oldDir=(direction)(((int)this->getDirection())%4);
    if(!movableElements::mechanics() || this->getMoved()>0)
        return false;

    if(this->steppableNeigh())
        this->inited=false;
    if(!this->inited)
    {
        if(this->isSteppableDirection(oldDir))
        {
            return this->moveInDirection(oldDir);
        }
        this->setDirection((direction)((((int)oldDir)+rotB)%4));
        oldDir=this->getDirection();
        this->inited=true;
    }
    this->checkNeigh();
    if(this->isWaiting())
        return true;
    for(int c=0; c<4; c++)
    {
        newDir=(direction)((((int)oldDir)+rotA)%4);
        if(this->isSteppableDirection(newDir))
        {
            this->moveInDirection(newDir);
            return true;
        }
        else
        {
            oldDir=(direction)((((int)oldDir)+rotB)%4);
        }
    }
    return true;
}

bool monster::steppableNeigh()
{
    sNeighboorhood n=this->getSteppableNeighboorhood();
    for(int c=0; c<8; c++)
    {
        if(n.steppable[c]==false)
            return false;

    }
    return true;
}

