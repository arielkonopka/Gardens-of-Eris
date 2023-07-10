#include "door.h"

videoElement::videoElementDef *door::vd = nullptr;

videoElement::videoElementDef *door::getVideoElementDef()
{
    return door::vd;
}

int door::getType()
{
    return _door;
}

door::door() : audibleElement()
{
}
door::door(std::shared_ptr<chamber> board) : door()
{

    this->setBoard(board);

}

bool door::additionalProvisioning(int subtype, std::shared_ptr<door>sbe)
{
    return this->additionalProvisioning(subtype,sbe->getType());
}

bool door::additionalProvisioning()
{
    return this->additionalProvisioning(0,this->getType());
}

bool door::additionalProvisioning(int subtype, int typeId)
{
    return bElem::additionalProvisioning(subtype,typeId);
}



/* open the door if you can */
bool door::interact(std::shared_ptr<bElem> who)
{
    bool bres = bElem::interact(who);
    std::shared_ptr<bElem> key = nullptr;
    if (!bres)
        return false;
    if (!this->attrs->isLocked())
    {
        this->status->setInteracted(10);
        this->attrs->setOpen(!this->attrs->isOpen());
        this->attrs->setSteppable(this->attrs->isOpen());
        this->status->setFacing((!this->open) ? UP : LEFT);
        this->playSound("Door", (this->open) ? "Unlock" : "Lock");

        return true;
    }
    // If it cannot collect, it cannot hold a key.
    if (!who->attrs->canCollect())
    {
        return false;
    }
    // if Door is unlocked, only open/close thing
    if (this->attrs->getSubtype() % 2 == 0)
    {
        key = who->attrs->getInventory()->getKey(_key, this->attrs->getSubtype(), true);
    }
    else
    {
        key = who->attrs->getInventory()->getKey(_key, this->attrs->getSubtype(), false);
    }
    if (key != nullptr)
    {
        this->playSound("Door", "Open");
        this->attrs->setOpen(true);
        this->attrs->setLocked(false);
        this->status->setFacing((!this->open) ? UP : LEFT);
        this->attrs->setSteppable(true);
    }
    else
    {
        return false;
    }
    if (this->attrs->getSubtype() % 2 == 0)
    {
        key->disposeElement();
    }
    return true;
}


door::~door()
{
    // dtor
}
