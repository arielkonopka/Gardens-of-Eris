#include "monster.h"

// the static variable must be initialized
videoElement::videoElementDef* monster::vd=NULL;


monster::monster(chamber *board): killableElements(board), nonSteppable(board), mechanical(board), movableElements(board)
{
    this->animph=0;
    this->internalCnt=0;
    this->setDirection(UP);
    this->setSubtype(0);
    this->inited=false;
    if(bElem::randomNumberGenerator()%2==0)
    {
        this->rotA=1;
        this->rotB=3;
    } else
    {
        this->rotA=3;
        this->rotB=1;
    }

}

monster::monster(chamber* board, int newSubtype): killableElements(board), nonSteppable(board), mechanical(board), movableElements(board)
{
    this->animph=0;
    this->internalCnt=0;
    this->setDirection(UP);
    this->setSubtype(newSubtype);
    this->inited=false;
    if(bElem::randomNumberGenerator()%2==0)
    {
        this->rotA=1;
        this->rotB=3;
    } else
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



bool monster::mechanics()
{

    direction newDir=NODIRECTION;
    direction oldDir=(direction)(((int)this->getDirection())%4);
  //  sNeighboorhood n=this->getSteppableNeighboorhood();
    bool res=movableElements::mechanics();
    if(!res)
        return false;
    if(this->getMoved()>0)
        return false;
    if(this->steppableNeigh())
        this->inited=false;
    if(!this->inited)
    {
        if(this->isSteppableDirection(oldDir))
        {
            this->moveInDirection(oldDir);
            return res;
        }
        this->setDirection((direction)((((int)oldDir)+rotB)%4));
        oldDir=this->getDirection();
        this->inited=true;
    }
    for(int c=0;c<4;c++)
    {
        newDir=(direction)((((int)oldDir)+rotA)%4);
        if(this->isSteppableDirection(newDir))
        {
            this->moveInDirection(newDir);
            return true;
        } else
        {
            oldDir=(direction)((((int)oldDir)+rotB)%4);
        }
    }
    return true;
}



/*
bool monster::mechanics()
{
    if (!killableElements::mechanics())
        return false;
    coords tmpcoords;
    bElem* te=this->getElementInDirection(this->getDirection());
    if(te!=NULL)
    {
        if(te->isInteractive())
            te->interact(this);
    }
      for(int c=0; c<4; c++)
      {
          bElem* testElem=this->getElementInDirection((direction)c);
          if (testElem==NULL)
          {
              continue;
          }
          if (testElem->getType()==_player && testElem->isTeleporting()==false && testElem->isActive()==true)
          {
              testElem->hurt(6);
              return true; //no need to leave the place, where we do the damage
              break;
          }

      }

/*    if(!this->isSteppableDirection(this->getDirection()) && this->isSteppableDirection((direction)(((int)this->getDirection()+1)%4)) && this->isSteppableDirection((direction)(((int)this->getDirection()+3)%4)))
        {
            this->moveInDirection((direction)(((int)this->getDirection()+1)%2));
            return true;
        }
 */
/*  if(this->steppableNeigh())
  {
          this->moveInDirection(this->getDirection());
          return true;
  }
  direction fd=NODIRECTION;
  direction ld=NODIRECTION;
  for(int c=3; c<7; c++)
  {
      direction d=(direction)((((int)this->getDirection())+c )%4);
      if(this->isSteppableDirection(d))
      {
          if(fd==NODIRECTION)
              fd=d;
          ld=d;
      }
      if(this->isSteppableDirection(d)==false && ld!=NODIRECTION)
      {
          break;
      }

  }
  this->moveInDirection(fd);

  return true;
}

*/

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

