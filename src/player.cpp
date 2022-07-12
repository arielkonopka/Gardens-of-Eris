#include "player.h"

videoElement::videoElementDef* player::vd=NULL;
std::vector<player*> player::allPlayers;
std::vector<player*> player::visitedPlayers;
player* player::activePlayer=NULL;

player::player(chamber *board) : killableElements::killableElements(board)
{
    this->used=0;
    this->movable=true;
    this->interacted=0;
    this->setMoved(0);
    this->animPh=0;
    this->myStats.dexterity=_initialDexterity;
    this->myInventory=new inventory();
    this->visited=false;
    if(player::allPlayers.size()>0)
    {
        this->activated=false;
    }
    else
    {
        this->activated=true;
    }
    player::allPlayers.push_back(this);

}
player* player::getActivePlayer()
{
    if (player::activePlayer==NULL)
    {
        /* find active player, because it is null */
        for(player* p:player::allPlayers)
        {
            if (p->isActive())
            {
                player::activePlayer=p;
            }
        }
    }
    /* return value can be null, then no active player found*/
    return player::activePlayer;
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

oState player::disposeElement()
{
    if(this->isActive())
    {
        this->setActive(false);
        this->getBoard()->player=NOCOORDS;
        if(player::visitedPlayers.size()>0)
        { // Activate next inactive player avatar
            bElem* p=player::visitedPlayers[0];
            p->setActive(true);
            p->getBoard()->player=p->getCoords();
            player::visitedPlayers.erase(player::visitedPlayers.begin());
        }
    }
    return killableElements::disposeElement();
}

bool player::interact(bElem* who)
{
    if (who==NULL)
        return false;
    if(this->isActive())
        return false;
    if (bElem::interact(who)==false)
        return false;
   // if(this->visited)
   //     return true;
    std::cout<<"Interacting...\n";
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
    if (this->getCoords()==NOCOORDS)
        return false; // It was disposed, so we do not want to process this
    if (this->isActive()==true)
    {
        this->getBoard()->player.x=this->x;
        this->getBoard()->player.y=this->y;
    }
    else
    {
        this->animPh=this->getCntr()>>2;
        return true; // Inactive player, not very useful;
    }
    if(this->isTeleporting())
    {
        this->animPh=this->teleporting;
        return true;
    }
    if ( (this->isDying()==true))
        return true;
    switch(this->getBoard()->cntrlItm.type)
    {
    case 0:
    {
        bool res=this->moveInDirection(this->getBoard()->cntrlItm.dir);
        if (res) this->animPh++;
        return true;
    }
    case 1:
    {
        this->setDirection(this->getBoard()->cntrlItm.dir);
        bool res=this->shootGun();
        if (res)
            this->animPh+=(this->taterCounter%2);
        this->setMoved(_mov_delay_push);
        return true;
    }
    case 2:
    {
        bElem* obj=this->getElementInDirection(this->getBoard()->cntrlItm.dir);
        bool res;
        this->setMoved(_mov_delay_push);
        this->setDirection(this->getBoard()->cntrlItm.dir);
        if (obj==NULL)
            return false;
        //std::cout<<"Interact\n";
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
   // bool res=false;
    bElem* gun=this->myInventory->getActiveWeapon();
    if(gun!=NULL)
    {
     //   std::cout<<"Gun present\n";
        gun->use(this);
        return true;
    }
    return false;

}




bool player::canPush()
{
    return true;
}


void player::setActive(bool act)
{
    this->activated=act;
}



bool player::isActive()
{
    return this->activated; // temporary, in the future only one player should be active in the whole chamber array
}


int player::getType()
{
    return _player;
}


int player::getAnimPh()
{
    return this->animPh;
}



bool player::canCollect()
{
    return true;
}


