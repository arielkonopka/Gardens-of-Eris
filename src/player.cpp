#include "player.h"

videoElement::videoElementDef* player::vd=NULL;


player::player(chamber *board,gCollect *garbage) : killableElements::killableElements(board,garbage)
{
    this->used=0;
    this->interacted=0;
    this->_me_moved=0;
    this->animPh=0;
    this->myStats.dexterity=_initialDexterity;
    this->myInventory=new inventory();
}

player::~player()
{
    //We remove all the collected weapons and stuff, we need to remember, that this is our responsibility to destroy our objects


}
videoElement::videoElementDef* player::getVideoElementDef()
{
    return player::vd;

}


bool player::mechanics(bool collected)
{
    killableElements::mechanics(collected);
    if (this->x<0) return false; // It was disposed, so we do not want to process this
    if (this->getActive()==true)
    {
        this->attachedBoard->player.x=this->x;
        this->attachedBoard->player.y=this->y;
    }
    if (this->isDying()==true)
    {
       // this->animPh++;
        return true;
    }
    if (this->attachedBoard->cntrlItm.type==0 && this->_me_moved==0)
    {
        bool res=this->moveInDirection(this->attachedBoard->cntrlItm.dir);
        if (res) this->animPh++;
        return true;
    }
     if (this->attachedBoard->cntrlItm.type==1 && this->_me_moved==0)
    {

        this->setDirection(this->attachedBoard->cntrlItm.dir);
        bool res=this->shootGun();
        if (res)
            this->animPhase++;
        this->_me_moved=_mov_delay;
        return true;
    }
  if (this->attachedBoard->cntrlItm.type==2 && this->_me_moved==0)
    {

        bElem* obj=this->getElementInDirection(this->attachedBoard->cntrlItm.dir);
        bool res;
        this->_me_moved=_mov_delay;
        this->setDirection(this->attachedBoard->cntrlItm.dir);
        if (obj==NULL)
            return false;
        std::cout<<"Interact/n";
        res=obj->interact(this);
        if (res)
            this->animPhase++;
        return true;
    }

    return false;

}

//shoots any suitable gun
bool player::shootGun()
{
    bool res=false;
    bElem* gun=this->myInventory->getActiveWeapon();
    if(gun!=NULL)
    {
        std::cout<<"Gun present\n";
        gun->use(this);
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


