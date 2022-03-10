#include "player.h"

videoElement::videoElementDef* player::vd=NULL;


player::player(chamber *board,gCollect *garbage) : movableElements::movableElements(board,garbage)
{
    this->used=0;
    this->interacted=0;
    this->_me_moved=0;
    this->animPh=0;
}

player::~player()
{
    //dtor
}
videoElement::videoElementDef* player::getVideoElementDef()
{
    return player::vd;

}


bool player::mechanics(bool collected)
{
    movableElements::mechanics(collected);
    if (this->x<0) return false; // It was disposed, so we do not want to process this
    if (this->getActive()==true)
    {
        this->attachedBoard->player.x=this->x;
        this->attachedBoard->player.y=this->y;
    }
    if (this->isDying()==true)
    {
        this->animPh++;
        return true;
    }
    if (this->attachedBoard->cntrlItm.type==0 && this->_me_moved==0)
    {
        bool res=this->moveInDirection(this->attachedBoard->cntrlItm.dir);
        if (res) this->animPh++;
        return true;
    }



    return false;

}

bool player::canPush()
{
    return true;
}
bool player::getActive()
{
    return true; // temporary, in the future only one player should be active in the whole chamber array
}


int player::getType()
{
    return _player;
}


bool player::canInteract()
{
    return true;
}

int player::getAnimPh()
{
    return this->animPh;
}




bool player::canCollect()
{
    return true;
}
bool player::canBeKilled()
{
    return true;
}

