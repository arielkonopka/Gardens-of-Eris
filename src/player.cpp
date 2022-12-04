#include "player.h"

videoElement::videoElementDef* player::vd=nullptr;
std::vector<player*> player::allPlayers;
std::vector<player*> player::visitedPlayers;
player* player::activePlayer=nullptr;

player::player(chamber *board) : killableElements(board), movableElements(board),nonSteppable(board),mechanical(board)
{
    this->setInventory(new inventory(this));
    this->registerLiveElement(this);
    this->setStats(new elemStats(100));
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
    if (player::activePlayer==nullptr)
    {
        /* find active player, because it is nullptr */
        for(player* p:player::allPlayers)
        {
            if (p->isActive())
            {
                player::activePlayer=p;
            }
        }
    }
    /* return value can be nullptr, then no active player found*/
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
        player::activePlayer=nullptr;
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
    if (who==nullptr)
        return false;
    if(this->isActive())
        return false;
    if (killableElements::interact(who)==false)
        return false;

    if(who->getType()==this->getType() && !this->visited)
    {
#ifdef _VerbousMode_
        std::cout<<"Adding new avatar\n";
#endif
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


bool player::mechanics()
{

    bool res=killableElements::mechanics();
    if (this->isActive()==true)
    {
        this->getBoard()->player.x=this->getCoords().x;
        this->getBoard()->player.y=this->getCoords().y;
    }
    else
    {
        return true; // Inactive player, not very useful;
    }

    if(!res)
        return false;

    if(this->getMoved()>0)
    {
        if(bElem::getCntr()%4==0) this->animPh++;
        return true;
    }

    switch(this->getBoard()->cntrlItm.type)
    {
    case 0:
        if(this->moveInDirection(this->getBoard()->cntrlItm.dir))
            this->animPh++;
        break;

    case 1:
        this->setDirection(this->getBoard()->cntrlItm.dir);
        if(this->shootGun()) this->animPh+=(this->getCntr()%2);
        break;
    case 2:
        if (this->getElementInDirection(this->getBoard()->cntrlItm.dir)==nullptr)
            return false;
        this->setDirection(this->getBoard()->cntrlItm.dir);
        if (this->getElementInDirection(this->getBoard()->cntrlItm.dir)->interact(this))
            this->animPh++;
        break;
    case 3:
        this->getInventory()->nextUsable();
        this->setWait(_mov_delay);
        break;
    case 4:
        if(this->dragInDirection(this->getBoard()->cntrlItm.dir))
            this->animPh++;
        else if(this->moveInDirection(this->getBoard()->cntrlItm.dir)) this->animPh++;
        break;
    case 8:
        if (this->getInventory()->getUsable()!=nullptr)
            this->getInventory()->getUsable()->use(this->getElementInDirection(this->getBoard()->cntrlItm.dir));
        break;
    case 5:
        this->getInventory()->nextGun();
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
        bElem* gun=this->getInventory()->getActiveWeapon();
        if(gun!=nullptr)
        {
            gun->use(this);
            return true;
        }
        this->setWait(_interactedTime*2);
        return false;

    }




    bool player::canPush()
    {
        return true;
    }


    void player::setActive(bool act)
    {
        this->activated=act;
        this->setTeleporting(_teleportationTime);
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
        if(this->isTeleporting() || this->isDying() || this->isDestroyed() || !this->isActive())
            return bElem::getAnimPh();
        return this->animPh;
    }





