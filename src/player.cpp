/*
 * Copyright (c) 2023, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "player.h"

std::vector<std::shared_ptr<bElem>> player::visitedPlayers;
//std::vector<std::shared_ptr<bElem>> player::visitedPlayers;
std::shared_ptr<bElem> player::activePlayer = nullptr;

player::player(std::shared_ptr<chamber> board) : player()
{

    this->setBoard(board);
}

player::player() : bElem()
{
}

bool player::additionalProvisioning(int subtype,int typeId)
{
    bElem::additionalProvisioning(subtype, typeId);
    this->getAttrs()->setCollect(true);
    this->getAttrs()->setEnergy(125);
    this->provisioned = true;
    this->registerLiveElement(shared_from_this());
    if ( this->getBoard() && player::activePlayer==nullptr)
    {
        this->getStats()->setActive(true);
        this->getStats()->setMarked(true);
        //player::visitedPlayers.push_back(shared_from_this());
        player::activePlayer=shared_from_this();
        viewPoint::get_instance()->setOwner(player::activePlayer);
    }
    else
    {
        this->getStats()->setActive(false);
    }

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
    std::mutex my_mutex;
    std::lock_guard<std::mutex> lock(my_mutex);

    if (player::activePlayer == nullptr || (player::activePlayer && player::activePlayer->getStats()->isDisposed()))
    {
        /* find active player, because it is nullptr */
        for (int p=(int)player::visitedPlayers.size()-1; p>=0; p--)
        {
            auto plr=player::visitedPlayers[p];
            if (plr && !plr->getStats()->isDisposed() && plr->getBoard())
            {
                viewPoint::get_instance()->setOwner(plr);
                player::activePlayer = plr;
                plr->getStats()->setActive(true);
                soundManager::getInstance()->setListenerChamber(plr->getBoard()->getInstanceId());
                player::visitedPlayers.erase(player::visitedPlayers.begin()+p);
                break;
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
    if (player::activePlayer && this->getStats()->getInstanceId()==player::activePlayer->getStats()->getInstanceId())
    {
        this->getStats()->setActive(false);
        player::activePlayer = nullptr;
        if(this->getBoard())
            this->getBoard()->player = NOCOORDS;
    }
    for (unsigned int cnt = 0; cnt < player::visitedPlayers.size();)
    {
        if (player::visitedPlayers[cnt]->getStats()->getInstanceId() == this->getStats()->getInstanceId())
        {
            player::visitedPlayers.erase(player::visitedPlayers.begin() + cnt);
        }
        else
            cnt++;
    }

    return bElem::disposeElement();
}

bool player::interact(std::shared_ptr<bElem> who)
{
    if (who == nullptr || this->getBoard() == nullptr || this->getStats()->isActive() || this->getStats()->isMarked() || bElem::interact(who) == false)
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
    bool r = bElem::stepOnElement(step);
    bool st=false;
    if (this->getBoard().get() != nullptr && this->getStats()->isActive())
        st=this->getBoard()->visitPosition(this->getStats()->getMyPosition()); // we visit the position.
    if(st)
    {
        this->getStats()->setStats(STEPS,this->getStats()->getStats(STEPS)+(bElem::randomNumberGenerator()%2));
        this->vRadius=2+(std::log(this->getStats()->getStats(STEPS))/2);
        this->getStats()->setPoints(TOTAL,this->getStats()->getPoints(TOTAL)+1);
    }
    if (r)
        inputManager::getInstance()->hapticKick(1.0);
    return r;
}



bool player::mechanics()
{

    bool res = bElem::mechanics();
    controlItem currentCtrlItem=inputManager::getInstance()->getCtrlItem();
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



    switch (currentCtrlItem.type)
    {
    case -1:
        this->animPh = 0;
        break;
    case 0:
        if (this->moveInDirection(currentCtrlItem.dir))
        {
            this->getStats()->setFacing(this->getStats()->getMyDirection());
            viewPoint::get_instance()->setOwner(shared_from_this());
            //
        }
        break;

    case 1:
        this->getStats()->setFacing(currentCtrlItem.dir);
        if (this->shootGun())
        {
            this->animPh += (bElem::getCntr() % 2);
        }
        break;
    case 2:
    {
        if (this->getElementInDirection(currentCtrlItem.dir) == nullptr)
            return false;
        this->getStats()->setFacing(currentCtrlItem.dir);
        std::shared_ptr<bElem> be=this->getElementInDirection(currentCtrlItem.dir);
        if (be->getAttrs()->isInteractive() && be->interact(shared_from_this()))
            return true;
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
        if (this->dragInDirection(currentCtrlItem.dir))
        {
            this->getStats()->setFacing((direction)(((int)this->getStats()->getMyDirection() + 2) % 4)); /* we face backwards while dragging */
        }
        else if (this->moveInDirection(currentCtrlItem.dir))
        {
            this->getStats()->setFacing(this->getStats()->getMyDirection());
        }
        break;
    case 8:
        if (this->getAttrs()->getInventory()->getUsable() != nullptr)
            this->getAttrs()->getInventory()->getUsable()->interact(shared_from_this());
        this->getStats()->setWaiting(1);
        break;
    case 5:
    {
        this->getAttrs()->getInventory()->nextGun();
        this->getStats()->setWaiting(_mov_delay*2);
        break;
    }
    case 6:
        this->kill();
        break;
    case 9:
        std::shared_ptr<bElem> _be=this->getAttrs()->getInventory()->getUsable();
        this->getStats()->setWaiting(_mov_delay*2);
        if(_be)
            return this->dropItem(_be->getStats()->getInstanceId());
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

