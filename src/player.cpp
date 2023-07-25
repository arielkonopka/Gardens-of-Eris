#include "player.h"

videoElement::videoElementDef *player::vd = nullptr;
std::vector<std::shared_ptr<bElem>> player::allPlayers;
std::vector<std::shared_ptr<bElem>> player::visitedPlayers;
std::shared_ptr<bElem> player::activePlayer = nullptr;

player::player(std::shared_ptr<chamber> board) : player()
{

    this->setBoard(board);
}

player::player() : killableElements(), movableElements(), nonSteppable(), mechanical()
{
}

bool player::additionalProvisioning(int subtype,int typeId)
{
    //  this->provisioned = true;
    this->attrs=std::make_unique<bElemAttr>(shared_from_this(),typeId,subtype);
    this->attrs->setCollect(true);
    this->attrs->setEnergy(125);
    this->provisioned = true;
    this->registerLiveElement(shared_from_this());
    if ( this->getBoard() && player::allPlayers.size() <= 0)
    {
        this->status->setActive(true);
        this->status->setMarked(true);
    }
    else
    {
        this->status->setActive(false);
    }
    player::allPlayers.push_back(shared_from_this());
    return true;
}


bool player::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}
bool player::additionalProvisioning(int subtype, std::shared_ptr<player>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

std::shared_ptr<bElem> player::getActivePlayer()
{
    if (player::activePlayer == nullptr || (player::activePlayer && player::activePlayer->status->isDisposed()))
    {
        /* find active player, because it is nullptr */
        for (auto p : player::allPlayers)
        {
            if (p && p->status->isMarked() && !p->status->isDisposed())
            {
                player::activePlayer = p;
                p->status->setActive(true);
                if(p->getBoard())
                    soundManager::getInstance()->setListenerChamber(p->getBoard()->getInstanceId());
            }
        }
    }
    /* return value can be nullptr, then no active player found*/


    return player::activePlayer;
}

unsigned int player::countVisitedPlayers()
{
    return player::visitedPlayers.size();
}

oState player::disposeElement()
{
    if (this->status->isActive())
    {
        this->status->setActive(false);
        player::activePlayer = nullptr;
        this->getBoard()->player = NOCOORDS;
        if (player::visitedPlayers.size() > 0)
        {
            // Activate next inactive player avatar
            std::shared_ptr<bElem> p = player::visitedPlayers[0];
            p->status->setActive(true);
            p->getBoard()->player = p->status->getMyPosition();
            player::visitedPlayers.erase(player::visitedPlayers.begin());
        }
    }
    for (unsigned int cnt = 0; cnt < player::allPlayers.size();)
    {
        if (player::allPlayers[cnt] == shared_from_this())
        {
            player::allPlayers.erase(player::allPlayers.begin() + cnt);
        }
        else
            cnt++;
    }
    for (unsigned int cnt = 0; cnt < player::visitedPlayers.size();)
    {
        if (player::visitedPlayers[cnt] == shared_from_this())
        {
            player::visitedPlayers.erase(player::visitedPlayers.begin() + cnt);
        }
        else
            cnt++;
    }
    return killableElements::disposeElement();
}

bool player::interact(std::shared_ptr<bElem> who)
{
    if (who == nullptr || this->getBoard() == nullptr || this->status->isActive() || this->status->isMarked() || killableElements::interact(who) == false)
        return false;

    if (who->getType() == this->getType() && !this->status->isActive() && !this->status->isMarked())
    {
#ifdef _VerbousMode_
        std::cout << "Adding new avatar\n";
#endif
        player::visitedPlayers.push_back(shared_from_this());
        this->playSound("Player", "ActivateAvatar");
        this->status->setMarked(true);
    }
    return true;
}



bool player::stepOnElement(std::shared_ptr<bElem> step)
{
    bool r = movableElements::stepOnElement(step);
    bool st=false;
    if (this->getBoard().get() != nullptr && this->status->isActive())
        st=this->getBoard()->visitPosition(this->status->getMyPosition()); // we visit the position.
    if(st)
    {
        this->status->setStats(STEPS,this->status->getStats(STEPS)+(bElem::randomNumberGenerator()%2));

        this->vRadius=2+(std::log(this->status->getStats(STEPS)));
    }

    return r;
}

videoElement::videoElementDef *player::getVideoElementDef()
{
    return player::vd;
}

bool player::mechanics()
{

    bool res = bElem::mechanics();
    if (this->status->isMoving())
    {
        if (bElem::getCntr() % 3 == 0)
            this->animPh++;
        return true;
    }
    if(!res || !this->status->isActive()) return res;
    this->getBoard()->player.x = this->status->getMyPosition().x;
    this->getBoard()->player.y = this->status->getMyPosition().y;
    coords3d c3d;
    c3d.x = this->status->getMyPosition().x * 32 + this->getOffset().x;
    c3d.z = this->status->getMyPosition().y * 32 + this->getOffset().y;
    c3d.y = 50;
    coords3d vel;
    switch (this->status->getMyDirection())
    {
    case UP:
        vel = {0, 0, -1};
        break;
    case LEFT:
        vel = {-1, 0, 0};
        break;
    case RIGHT:
        vel = {1, 0, 0};
        break;
    case DOWN:
        vel = {0, 0, 1};
        break;
    case NODIRECTION:
        vel = {0, 0, 0};
    }
    soundManager::getInstance()->setListenerChamber(this->getBoard()->getInstanceId());
    soundManager::getInstance()->setListenerOrientation({0, -10, 0});
    soundManager::getInstance()->setListenerPosition(c3d);
    if (!res)
        return false;



    switch (this->getBoard()->cntrlItm.type)
    {
    case -1:
        this->animPh = 0;
        break;
    case 0:
        if (this->moveInDirection(this->getBoard()->cntrlItm.dir))
        {
            this->status->setFacing(this->status->getMyDirection());
            //
        }
        break;

    case 1:
        this->status->setFacing(this->getBoard()->cntrlItm.dir);
        if (this->shootGun())
        {
            this->animPh += (bElem::getCntr() % 2);
        }
        break;
    case 2:
        if (this->getElementInDirection(this->getBoard()->cntrlItm.dir) == nullptr)
            return false;
        this->status->setFacing(this->getBoard()->cntrlItm.dir);
        if (this->getElementInDirection(this->getBoard()->cntrlItm.dir)->interact(shared_from_this()))
            this->animPh++;
        break;
    case 3:
        this->attrs->getInventory()->nextUsable();
        this->status->setWaiting(_mov_delay);
        break;
    case 4:
        if (this->dragInDirection(this->getBoard()->cntrlItm.dir))
        {
            this->status->setFacing((direction)(((int)this->status->getMyDirection() + 2) % 4)); /* we face backwards while dragging */
        }
        else if (this->moveInDirection(this->getBoard()->cntrlItm.dir))
        {
            this->status->setFacing(this->status->getMyDirection());
        }
        break;
    case 8:
        if (this->attrs->getInventory()->getUsable() != nullptr)
            this->attrs->getInventory()->getUsable()->use(this->getElementInDirection(this->getBoard()->cntrlItm.dir));
        break;
    case 5:
    {
        this->attrs->getInventory()->nextGun();
        this->status->setWaiting(_mov_delay);
        break;
    }
    case 6:
        this->kill();
        break;
    }
    return true;
}
// shoots any suitable gun
bool player::shootGun()
{
    std::shared_ptr<bElem> gun = this->attrs->getInventory()->getActiveWeapon();
    if (gun != nullptr)
    {
        if (gun->use(shared_from_this()))
        {
            this->status->setWaiting(_interactedTime * 2);
        };
        return true;
    }
    this->status->setWaiting(_interactedTime);
    return false;
}



float player::getViewRadius() const
{
    return this->vRadius;
}





int player::getType() const
{
    return _player;
}

int player::getAnimPh() const
{
    if (this->status->isTeleporting() || this->status->isDying() || this->status->isDestroying() || !this->status->isActive())
        return bElem::getAnimPh();
    return this->animPh;
}

