#include "monster.h"

// the static variable must be initialized
videoElement::videoElementDef* monster::vd=NULL;

monster::monster(chamber *board,gCollect *garbage): movableElements(board,garbage)
{
    this->animph=0;
    this->internalCnt=0;
    this->facing=UP;
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
    movableElements::mechanics(collected);
    coords tmpcoords;

    //this->tick(collected);

    this->internalCnt++;
    if (this->internalCnt%10==0)
        this->animph++;
    // if (this->internalCnt%20==0)
    //     this->facing=(direction)(((int)this->facing+1)%4);
    if (this->_me_moved==0)
    {
        for(int c=0; c<4; c++)
        {
            bElem* testElem=this->getElementInDirection((direction)c);
            if (testElem==NULL)
            {
                continue;
            }
            if (testElem->getType()==_player)
            {
                testElem->hurt(6);
                return true; //no need to leave the place, where we do the damage
                break;
            }
        }
        if(this->moveInDirection(this->facing))
        {
            bElem* testElem=this->getElementInDirection((direction)(((int)this->facing+3)%4));
            if (testElem!=NULL)
            {
                if (testElem->isSteppable())
                    this->facing=(direction)(((int)this->facing+3)%4);
            }
        }
        else
        {
            this->facing=(direction)(((int)this->facing+1)%4);
            this->_me_moved=_mov_delay;
            if (this->facing==RIGHT || this->facing==DOWN) this->_me_moved++;
        }

    }
    return true;
}
int monster::getAnimPh()
{
    return this->animph;

}


bool monster::steppableNeigh()
{
    for (int c=0; c<4; c++)
    {
        if (this->isSteppableDirection((direction)c)==false)
        {
            return false;
        }

    }
    return true;
}



bool monster::canBeKilled()
{
    return true;
}
