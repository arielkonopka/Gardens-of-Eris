#include "monster.h"

// the static variable must be initialized
videoElement::videoElementDef* monster::vd=NULL;

monster::monster(chamber *board): killableElements(board,true)
{
    this->animph=0;
    this->internalCnt=0;
    this->setDirection(UP);
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

int monster::getSubtype()
{
    return 0;
}
bool monster::mechanics(bool collected)
{
    if (!killableElements::mechanics(collected))
        return false;
    coords tmpcoords;
    sNeighboorhood myNeigh=this->getSteppableNeighboorhood();
    bool _empty=true;
    for(int c=0; c<4; c++)
    {
        if(myNeigh.steppableClose[c]==false)
            _empty=false;
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
    if(_empty)
    {
        this->moveInDirection(this->getDirection());
        return true;
    }
    if(this->moveInDirection(this->getDirection()))
    {

        bElem* testElem=this->getElementInDirection((direction)(((int)this->getDirection()+3)%4));
        if (testElem!=NULL)
        {
            if (testElem->isSteppable())
                this->setDirection((direction)(((int)this->getDirection()+3)%4));
        }
    }
    else
    {
        this->setDirection((direction)(((int)this->getDirection()+1)%4));
        this->setMoved(_mov_delay);

    }

    return true;
}



bool monster::steppableNeigh()
{
    for (int c=0; c<4; c++)
    {
        if (!this->isSteppableDirection((direction)c))
        {
            return false;
        }

    }
    return true;
}

