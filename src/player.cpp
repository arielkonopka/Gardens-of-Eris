#include "player.h"

videoElement::videoElementDef* player::vd=NULL;
std::vector<player*> player::allPlayers;
std::vector<player*> player::visitedPlayers;


player::player(chamber *board,gCollect *garbage) : killableElements::killableElements(board,garbage)
{
    this->used=0;
    this->interacted=0;
    this->setMoved(0);
    this->animPh=0;
    this->myStats.dexterity=_initialDexterity;
    this->myInventory=new inventory();
    this->visited=false;
    player::allPlayers.push_back(this);
}

player::~player()
{

    //We remove all the collected weapons and stuff, we need to remember, that this is our responsibility to destroy our objects
    for(std::vector<player*>::iterator p=player::allPlayers.begin(); p!=player::allPlayers.end();)
    {
        player* px=*p;
        if(this->getInstanceid()==px->getInstanceid())
            player::allPlayers.erase(p);
        else
           p++;
    }
    for(std::vector<player*>::iterator p=player::visitedPlayers.begin(); p!=player::visitedPlayers.end();)
    {
        player* px=*p;
        if(this->getInstanceid()==px->getInstanceid())
            player::visitedPlayers.erase(p);
        else
           p++;
    }



}
bool player::interact(bElem* who)
{
    if (who==NULL)
        return false;
    if(who->getType()==this->getType())
    {
        player::visitedPlayers.push_back(this);
        this->visited=true;
    }
    return true;
}

bool player::getVisited()
{
    return this->visited;
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
    else
    {
        this->animPh=this->taterCounter/40;
        return true; // Inactive player, not very useful;
    }
    if(this->isTeleporting())
    {
        this->animPh=this->teleporting;
        return true;
    }
    if ( (this->isDying()==true))
        return true;
    switch(this->attachedBoard->cntrlItm.type)
    {
    case 0:
    {
        bool res=this->moveInDirection(this->attachedBoard->cntrlItm.dir);
        if (res) this->animPh++;
        return true;
    }
    case 1:
    {
        this->setDirection(this->attachedBoard->cntrlItm.dir);
        bool res=this->shootGun();
        if (res)
            this->animPh+=(this->taterCounter%2);
        this->setMoved(_mov_delay);
        return true;
    }
    case 2:
    {
        bElem* obj=this->getElementInDirection(this->attachedBoard->cntrlItm.dir);
        bool res;
        this->setMoved(_mov_delay);
        this->setDirection(this->attachedBoard->cntrlItm.dir);
        if (obj==NULL)
            return false;
        std::cout<<"Interact/n";
        res=obj->interact(this);
        if (res)
            this->animPh++;
        return true;
    }
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

bool player::findAndActivatePlayer()
{
    if(player::visitedPlayers.size()<=0)
        return false;
    player::visitedPlayers[0]->setActive(true);
    player::visitedPlayers[0]->attachedBoard->player=player::visitedPlayers[0]->getCoords();
    player::visitedPlayers.erase(player::visitedPlayers.begin());
    return true;
}



bool player::canCollect()
{
    return true;
}


