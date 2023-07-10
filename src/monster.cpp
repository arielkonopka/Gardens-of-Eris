#include "monster.h"

// the static variable must be initialized
videoElement::videoElementDef *monster::vd = nullptr;

monster::monster(std::shared_ptr<chamber> board) : monster()
{
    this->setBoard(board);
}

monster::monster(std::shared_ptr<chamber> board, int newSubtype) : monster(board)
{
}
monster::monster() : killableElements(), nonSteppable(), mechanical(), movableElements()
{

}

bool monster::additionalProvisioning(int subtype, std::shared_ptr<monster>sbe)
{
    return bElem::additionalProvisioning(subtype,sbe->getType());
}









bool monster::additionalProvisioning(int subtype, int typeId)
{
    bool res1= bElem::additionalProvisioning(subtype,typeId);
//  this->attrs->setEnergy(_defaultEnergy);
//   this->setInventory(std::make_shared<inventory>()); // setCollect should be true
    if (bElem::randomNumberGenerator() % 2 == 0)
    {
        this->rotA = 1;
        this->rotB = 3;
    }
    if (bElem::randomNumberGenerator() % 5 == 0)
    {
        this->weapon = elementFactory::generateAnElement<plainGun>(this->getBoard(),0);
        this->weapon->attrs->setEnergy(((bElem::randomNumberGenerator()*555)%5)*5);
        this->weapon->attrs->setAmmo(5 * (5 + bElem::randomNumberGenerator() % 55));
        this->weapon->attrs->setMaxEnergy(5*5*5);
    }
    return res1;
}

bool monster::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
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
#ifdef _VerbousMode_
        std::cout<<"  ** CHK isCollectible\n";
#endif
        if (e->attrs->isCollectible())
        {
            this->collect(e);
            this->status->setWaiting(_mov_delay);
            r = true;
            continue;
        }
#ifdef _VerbousMode_
        std::cout<<"  ** CHK isCollectible done\n";
        std::cout<<"  ** CHK getType\n";
#endif
        if (e->getType() == _player)
        {
#ifdef _VerbousMode_
            std::cout<<"  *** Hurt \n";
#endif
            e->hurt(5);
#ifdef _VerbousMode_
            std::cout<<"  *** Done\n";
#endif
            r = true;
            continue;
        }
#ifdef _VerbousMode_
        std::cout<<"  ** CHK getType Done\n";
#endif
        if (this->weapon.get()!=nullptr  || this->attrs->canCollect()) //
        {
            while (e != nullptr) // this is the "mostervision"
            {

                if (e->getType() == _stash || e->getType() == _rubishType || (e->getType()==_goldenAppleType && e->attrs->getSubtype()!=0) || e->attrs->isWeapon()) // take the direction towards remainings from other objects, broken apples or guns
                {
                    this->status->setMyDirection(d);
                    this->status->setFacing(d);
                    this->inited = false;
                    this->status->setWaiting(_mov_delay); // will wait...
                    return true;
                }

                if (
                    ((e->getType() == _player && e->status->isActive()) || (e->getType() == _patrollingDrone && e->status->hasActivatedMechanics()))
                    &&
                    ((this->attrs->canCollect() && this->attrs->getInventory()->getActiveWeapon() != nullptr ) || this->weapon != nullptr)
                )
                {
                    this->status->setFacing(d);
                    if (this->weapon != nullptr)
                    {
                        this->weapon->use(shared_from_this()); // shoot an object with native gun
                    }
                    else
                    {
                        this->attrs->getInventory()->getActiveWeapon()->use(shared_from_this()); // shoot with the one from the inventory - surprise thing:)
                    }
                    this->status->setWaiting(_mov_delay); // will wait next couple times
                    break;
                }
                // if it is something interesting, go and fetch it
                if (e->getType() == _stash || e->getType() == _rubishType || (e->getType()==_goldenAppleType && e->attrs->getSubtype()!=0) || e->attrs->isWeapon()) // take the direction towards remainings from other objects, broken apples or guns
                {
                    this->status->setMyDirection(d);
                    this->status->setFacing(d);
                    this->inited = false;
                    this->status->setWaiting(_mov_delay); // will wait...
                    return true;
                }

                // closed door? and we got a key?
                if ((e->getType() == _door && !e->attrs->isSteppable()) && (this->attrs->getInventory()->countTokens(_door, e->attrs->getSubtype()) > 0))
                {

                    this->status->setMyDirection(d);
                    this->inited = false;
                    this->status->setWaiting(_mov_delay);
                    return true;

                }
                // we do not see behind non steppable objects
                if (!e->attrs->isSteppable() || e->getElementInDirection(d) == nullptr)
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
    direction oldDir = (direction)(((int)this->status->getMyDirection()) % 4);
    if (!movableElements::mechanics() || this->status->getMoved() > 0)
        return false;
//    std::cout<<"   * CHK seppableNeigh\n";
    if (this->steppableNeigh())
        this->inited = false;
//    std::cout<<"   * CHK seppableNeigh done\n";

    if (!this->inited)
    {
        if (this->isSteppableDirection(oldDir))
        {
            this->status->setFacing(oldDir);
            return this->moveInDirection(oldDir);
        }
        this->status->setMyDirection((direction)((((int)oldDir) + rotB) % 4));
        this->status->setFacing(this->status->getMyDirection());
        oldDir = this->status->getMyDirection();
        this->inited = true;
    }
    this->checkNeigh();
    if (this->status->isWaiting())
        return true;
    for (int c = 0; c < 4; c++)
    {
        newDir = (direction)((((int)oldDir) + rotA) % 4);
        if (this->isSteppableDirection(newDir))
        {
            this->status->setFacing(newDir);
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
