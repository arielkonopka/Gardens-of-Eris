#include "player.h"

videoElement::videoElementDef *player::vd = nullptr;
std::vector<std::shared_ptr<bElem>> player::allPlayers;
std::vector<std::shared_ptr<bElem>> player::visitedPlayers;
std::shared_ptr<bElem> player::activePlayer = nullptr;

player::player(std::shared_ptr<chamber> board) : player()
{

    this->setBoard(board);
}

player::player() : killableElements(), movableElements(), mechanical()
{
}

bool player::additionalProvisioning(int subtype,int typeId)
{
    bElem::additionalProvisioning(subtype, typeId);
    this->getAttrs()->setCollect(true);
    this->getAttrs()->setEnergy(125);
    this->provisioned = true;
    this->registerLiveElement(shared_from_this());
    if ( this->getBoard() && player::allPlayers.size() <= 0)
    {
        this->getStats()->setActive(true);
        this->getStats()->setMarked(true);
    }
    else
    {
        this->getStats()->setActive(false);
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
    if (player::activePlayer == nullptr || (player::activePlayer && player::activePlayer->getStats()->isDisposed()))
    {
        /* find active player, because it is nullptr */
        for (auto p : player::allPlayers)
        {
            if (p && p->getStats()->isMarked() && !p->getStats()->isDisposed())
            {
                player::activePlayer = p;
                p->getStats()->setActive(true);
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
    if (this->getStats()->isActive())
    {
        this->getStats()->setActive(false);
        player::activePlayer = nullptr;
        this->getBoard()->player = NOCOORDS;
        if (player::visitedPlayers.size() > 0)
        {
            // Activate next inactive player avatar
            std::shared_ptr<bElem> p = player::visitedPlayers[0];
            p->getStats()->setActive(true);
            p->getBoard()->player = p->getStats()->getMyPosition();
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
    if (who == nullptr || this->getBoard() == nullptr || this->getStats()->isActive() || this->getStats()->isMarked() || killableElements::interact(who) == false)
        return false;

    if (who->getType() == this->getType() && !this->getStats()->isActive() && !this->getStats()->isMarked())
    {
#ifdef _VerbousMode_
        std::cout << "Adding new avatar\n";
#endif
        player::visitedPlayers.push_back(shared_from_this());
        this->playSound("Player", "ActivateAvatar");
        this->getStats()->setMarked(true);
    }
    return true;
}



bool player::stepOnElement(std::shared_ptr<bElem> step)
{
    bool r = movableElements::stepOnElement(step);
    bool st=false;
    if (this->getBoard().get() != nullptr && this->getStats()->isActive())
        st=this->getBoard()->visitPosition(this->getStats()->getMyPosition()); // we visit the position.
    if(st)
    {
        this->getStats()->setStats(STEPS,this->getStats()->getStats(STEPS)+(bElem::randomNumberGenerator()%2));

        this->vRadius=2+(std::log(this->getStats()->getStats(STEPS)));
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
    if (this->getStats()->isMoving())
    {
        if (bElem::getCntr() % 3 == 0)
            this->animPh++;
        return true;
    }
    if(!res || !this->getStats()->isActive()) return res;
    this->getBoard()->player.x = this->getStats()->getMyPosition().x;
    this->getBoard()->player.y = this->getStats()->getMyPosition().y;
    coords3d c3d;
    c3d.x = this->getStats()->getMyPosition().x * 32 + this->getOffset().x;
    c3d.z = this->getStats()->getMyPosition().y * 32 + this->getOffset().y;
    c3d.y = 50;
    coords3d vel;
    switch (this->getStats()->getMyDirection())
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
            this->getStats()->setFacing(this->getStats()->getMyDirection());
            //
        }
        break;

    case 1:
        this->getStats()->setFacing(this->getBoard()->cntrlItm.dir);
        if (this->shootGun())
        {
            this->animPh += (bElem::getCntr() % 2);
        }
        break;
    case 2:
    {
        if (this->getElementInDirection(this->getBoard()->cntrlItm.dir) == nullptr)
            return false;
        this->getStats()->setFacing(this->getBoard()->cntrlItm.dir);
        std::shared_ptr<bElem> be=this->getElementInDirection(this->getBoard()->cntrlItm.dir);
        if (be->getAttrs()->isInteractive())
            be->interact(shared_from_this());
        if(be->getAttrs()->isCollectible())
            this->collect(be);
        this->animPh++;
        break;
    }
    case 3:
        this->getAttrs()->getInventory()->nextUsable();
        this->getStats()->setWaiting(_mov_delay);
        break;
    case 4:
        if (this->dragInDirection(this->getBoard()->cntrlItm.dir))
        {
            this->getStats()->setFacing((direction)(((int)this->getStats()->getMyDirection() + 2) % 4)); /* we face backwards while dragging */
        }
        else if (this->moveInDirection(this->getBoard()->cntrlItm.dir))
        {
            this->getStats()->setFacing(this->getStats()->getMyDirection());
        }
        break;
    case 8:
        if (this->getAttrs()->getInventory()->getUsable() != nullptr)
            this->getAttrs()->getInventory()->getUsable()->use(this->getElementInDirection(this->getBoard()->cntrlItm.dir));
        break;
    case 5:
    {
        this->getAttrs()->getInventory()->nextGun();
        this->getStats()->setWaiting(_mov_delay);
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
    std::shared_ptr<bElem> gun = this->getAttrs()->getInventory()->getActiveWeapon();
    if (gun != nullptr)
    {
        if (gun->use(shared_from_this()))
        {
            this->getStats()->setWaiting(_interactedTime * 2);
        };
        return true;
    }
    this->getStats()->setWaiting(_interactedTime);
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
    if (this->getStats()->isTeleporting() || this->getStats()->isDying() || this->getStats()->isDestroying() || !this->getStats()->isActive())
        return bElem::getAnimPh();
    return this->animPh;
}

