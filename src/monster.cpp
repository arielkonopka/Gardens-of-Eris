#include "monster.h"

// the static variable must be initialized
videoElement::videoElementDef *monster::vd = nullptr;

monster::monster(std::shared_ptr<chamber> board) : monster()
{
    this->setBoard(board);
}

monster::monster(std::shared_ptr<chamber> board, int newSubtype) : monster(board)
{
    this->setSubtype(newSubtype);
}
monster::monster() : killableElements(), nonSteppable(), mechanical(), movableElements()
{
    this->setEnergy(_defaultEnergy);
    this->setInventory(std::make_shared<inventory>());
    if (bElem::randomNumberGenerator() % 2 == 0)
    {
        this->rotA = 1;
        this->rotB = 3;
    }
    if (bElem::randomNumberGenerator() % 5 == 0)
    {
        this->weapon = std::make_unique<plainGun>();
        this->weapon->setEnergy(((bElem::randomNumberGenerator()*555)%5)*5);
        this->weapon->setAmmo(5 * (5 + bElem::randomNumberGenerator() % 55));
        this->weapon->setMaxEnergy(5 * this->weapon->getEnergy());
    }
}

monster::~monster()
{
 
}
videoElement::videoElementDef *monster::getVideoElementDef()
{
    return monster::vd;
}
int monster::getType()
{
    return _monster;
}

bool monster::checkNeigh()
{
    bool r = false;
    for (int c = 0; c < 4; c++)
    {
        direction d = (direction)c;
        std::shared_ptr<bElem> e = this->getElementInDirection(d);
        ;
        if (e == nullptr)
            continue;
        if (e->isCollectible())
        {
            this->collect(e);
            this->setWait(_mov_delay);
            r = true;
            continue;
        }
        if (e->getType() == _player)
        {
            e->hurt(5);
            r = true;
            continue;
        }

        if (this->weapon != nullptr || this->getInventory()->getActiveWeapon() != nullptr)
        {
            while (e != nullptr)
            {
                if ((this->getInventory()->getActiveWeapon() != nullptr || this->weapon != nullptr) &&
                    ((e->getType() == _player && e->isActive()) ||
                     (e->getType() == _patrollingDrone && e->isLiveElement())))
                {
                    this->setFacing(d);
                    if (this->weapon != nullptr)
                    {
                        this->weapon->use(shared_from_this());
                    }
                    else
                    {
                        this->getInventory()->getActiveWeapon()->use(shared_from_this());
                    }
                    this->setWait(_mov_delay);
                    break;
                }

                if (e->getType() == _stash || e->getType() == _rubishType || e->isWeapon()) // Take remainings, and guns
                {
                    this->setDirection(d);
                    this->inited = false;
                    this->setWait(_mov_delay);
                    return true;
                }
                // closed door?
                if (e->getType() == _door && !e->isSteppable())
                {
                    if (this->getInventory()->countTokens(_door, e->getSubtype()) > 0) // we got the key? Go for it
                    {
                        this->setDirection(d);
                        this->inited = false;
                        this->setWait(_mov_delay);
                        return true;
                    }
                }
                if (!e->isSteppable() || e->getElementInDirection(d) == nullptr)
                    break;
                e = e->getElementInDirection(d);
            }
        }
    }
    return r;
}
bool monster::mechanics()
{

    direction newDir = NODIRECTION;
    direction oldDir = (direction)(((int)this->getDirection()) % 4);
    if (!movableElements::mechanics() || this->getMoved() > 0)
        return false;

    if (this->steppableNeigh())
        this->inited = false;
    if (!this->inited)
    {
        if (this->isSteppableDirection(oldDir))
        {
            this->setFacing(oldDir);
            return this->moveInDirection(oldDir);
        }
        this->setDirection((direction)((((int)oldDir) + rotB) % 4));
        this->setFacing(this->getDirection());
        oldDir = this->getDirection();
        this->inited = true;
    }
    this->checkNeigh();
    if (this->isWaiting())
        return true;
    for (int c = 0; c < 4; c++)
    {
        newDir = (direction)((((int)oldDir) + rotA) % 4);
        if (this->isSteppableDirection(newDir))
        {
            this->setFacing(newDir);
            this->moveInDirection(newDir);
            return true;
        }
        else
        {
            oldDir = (direction)((((int)oldDir) + rotB) % 4);
        }
    }
    return true;
}

bool monster::steppableNeigh()
{
    sNeighboorhood n = this->getSteppableNeighboorhood();
    for (int c = 0; c < 8; c++)
    {
        if (n.steppable[c] == false)
            return false;
    }
    return true;
}
