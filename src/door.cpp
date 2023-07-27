#include "door.h"

videoElement::videoElementDef *door::vd = nullptr;

videoElement::videoElementDef *door::getVideoElementDef()
{
    return door::vd;
}

int door::getType() const
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
    bool res=bElem::additionalProvisioning(subtype,typeId);
    this->attrs->setSteppable(false);
    this->attrs->setLocked(true);
    this->attrs->setOpen(false);
    return res;
}

bool door::stepOnAction(bool step, std::shared_ptr<bElem>who)
{
    if(who==nullptr)
        return false;
    if(step==true)
    {
        if(this->attrs->getSubtype()%2==1 )
        {
            std::shared_ptr<bElem> k;
            if( who->attrs->canCollect())
            {
                k=who->attrs->getInventory()->getKey(_key,this->attrs->getSubtype(),true);
            }
            if(!k)
            {
                who->kill();
            }
            else
                this->playSound("Door","CollectKey");
        }
    }
    else
    {
        if(this->attrs->getSubtype()%2==1 )
        {
            this->attrs->setOpen(false);
            this->attrs->setSteppable(this->attrs->isOpen());
            this->attrs->setLocked(true);
            this->playSound("Door", (this->attrs->isOpen()) ? "Unlock" : "Lock");
            this->status->setFacing((!this->attrs->isOpen()) ? UP : LEFT);
        }
    }
    return true;
}


void door::_alignWithOpen()
{
        this->status->setFacing((!this->attrs->isOpen()) ? UP : LEFT);
        this->attrs->setSteppable(this->attrs->isOpen());

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
        who->status->setWaiting(1);
        this->attrs->setOpen(!this->attrs->isOpen());
        this->_alignWithOpen();
        this->playSound("Door", (this->attrs->isOpen()) ? "Unlock" : "Lock");
        return true;
    }
    if (!who->attrs->canCollect())
    {
        return false;
    }
    key = who->attrs->getInventory()->getKey(_key, this->attrs->getSubtype(), this->attrs->getSubtype()%2!=1);
    if (key != nullptr)
    {
        this->playSound("Door", "Open");
        this->attrs->setOpen(true);
        this->attrs->setLocked(false);
        this->_alignWithOpen();
    }
    else
    {
        return false;
    }
    if(key &&  this->attrs->getSubtype()%2!=1)
    {
        this->playSound("Door","CollectKey");
        key->disposeElement();
    }
    return true;
}


door::~door()
{
    // dtor
}
