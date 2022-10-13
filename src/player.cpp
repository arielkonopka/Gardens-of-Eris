#include "player.h"

videoElement::videoElementDef* player::vd=NULL;
std::vector<player*> player::allPlayers;
std::vector<player*> player::visitedPlayers;
player* player::activePlayer=NULL;

player::player(chamber *board) : killableElements::killableElements(board,true)
{
    this->used=0;
    this->movable=true;
    this->interacted=0;
    this->setMoved(0);
    this->animPh=0;

    this->myInventory=new inventory(this);
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


unsigned int player::countVisitedPlayers()
{
    return player::visitedPlayers.size();
}


oState player::disposeElement()
{
    if(this->isActive())
    {
        this->setActive(false);
        player::activePlayer=NULL;
        this->getBoard()->player=NOCOORDS;
        if(player::visitedPlayers.size()>0)
        {
            // Activate next inactive player avatar
            bElem* p=player::visitedPlayers[0];
            p->setActive(true);
            p->getBoard()->player=p->getCoords();
            player::visitedPlayers.erase(player::visitedPlayers.begin());
        }
    }
    for(unsigned int cnt=0; cnt<player::allPlayers.size();)
    {
        if(player::allPlayers[cnt]==this)
        {
            player::allPlayers.erase(player::allPlayers.begin()+cnt);
        }
        else cnt++;
    }
    for(unsigned int cnt=0; cnt<player::visitedPlayers.size();)
    {
        if(player::visitedPlayers[cnt]==this)
        {
            player::visitedPlayers.erase(player::visitedPlayers.begin()+cnt);
        }
        else cnt++;
    }
    return killableElements::disposeElement();
}

bool player::interact(bElem* who)
{
    if (who==NULL)
        return false;
    if(this->isActive())
        return false;
    if (killableElements::interact(who)==false)
        return false;
    // if(this->visited)
    //     return true;
    //std::cout<<"Interacting...\n";
    if(who->getType()==this->getType() && !this->visited)
    {
        std::cout<<"Adding new avatar\n";
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
    bool res=killableElements::mechanics(collected);

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
    if(!res)
        return false;
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
        //  this->setMoved(_mov_delay_push);
        return true;
    }
    case 2:
    {
        bElem* obj=this->getElementInDirection(this->getBoard()->cntrlItm.dir);
        bool res;
        //  this->setMoved(_mov_delay_push);
        this->setDirection(this->getBoard()->cntrlItm.dir);
        if (obj==NULL)
            return false;
        //std::cout<<"Interact\n";
        res=obj->interact(this);
        if (res)
            this->animPh++;
        return true;
    }
    break;
    case 3:
        this->myInventory->nextUsable();
        this->setWait(_mov_delay);
        break;
    case 4:
        if (this->myInventory->getUsable()!=NULL)
        {
            this->myInventory->getUsable()->use(this->getElementInDirection(this->getBoard()->cntrlItm.dir));
        };
        break;
    case 5:
        this->myInventory->nextGun();
        this->setWait(_mov_delay);
        break;
    case 6:
        this->kill();
        break;
    }

    return true;

}

//shoots any suitable gun
bool player::shootGun()
{
    // bool res=false;
    bElem* gun=this->myInventory->getActiveWeapon();
    if(gun!=NULL)
    {
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
    this->teleporting=_teleportationTime;
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
    if(this->isTeleporting() || this->isDying() || this->isDestroyed())
        return killableElements::getAnimPh();
    return this->animPh;
}



bool player::canCollect()
{
    return true;
}


