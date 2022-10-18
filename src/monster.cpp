#include "monster.h"

// the static variable must be initialized
videoElement::videoElementDef* monster::vd=NULL;

monster::monster(chamber *board): killableElements(board,true)
{
    this->animph=0;
    this->internalCnt=0;
    this->setDirection(UP);
    this->setSubtype(0);
}

monster::monster(chamber* board, int newSubtype): killableElements(board,true)
{
    this->animph=0;
    this->internalCnt=0;
    this->setDirection(UP);
    this->setSubtype(newSubtype);
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


bool monster::mechanics(bool collected)
{
    if (!killableElements::mechanics(collected))
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

    if(!this->isSteppableDirection(this->getDirection()) && this->isSteppableDirection((direction)(((int)this->getDirection()+1)%4)) && this->isSteppableDirection((direction)(((int)this->getDirection()+3)%4)))
        {
            this->moveInDirection((direction)(((int)this->getDirection()+1)%2));
            return true;
        }
    if(this->steppableNeigh())
    {
            this->moveInDirection(this->getDirection());
            return true;
    }

    for(int c=3; c<7; c++)
    {
        direction d=(direction)((((int)this->getDirection())+c )%4);
        if(this->isSteppableDirection(d))
        {
            this->moveInDirection(d);
            break;

        }
    }

    return true;
}



bool monster::steppableNeigh()
{
    sNeighboorhood n=this->getSteppableNeighboorhood();
    for(int c=0;c<8;c++)
    {
        if(n.steppable[c]==false)
            return false;

    }
    return true;
}

