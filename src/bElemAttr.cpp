#include "../include/bElemAttr.h"
#include "../include/bElem.h"
#include "../include/inventory.h"



bElemAttr::bElemAttr(std::shared_ptr<bElem> owner, int type, int subtype)
{
    this->owner = owner;
    this->setSubtype(subtype);

    if (owner == nullptr)
        return; // no point getting values, set it yourself then
    this->getDefaultValues(type, subtype);

}

/**
 * @brief Sets the default values for the element attributes based on the specified type and subtype.
 *
 * This method sets the default values for the element attributes based on the specified type and subtype.
 * It retrieves the relevant attribute values from the configuration manager and assigns them to the element.
 *
 * @param typeId The type ID of the element.
 * @param subtypeId The subtype ID of the element.
 *
 * @note This method provisionally sets the attributes for the element based on the type and subtype.
 * If the subtype ID is less than 0, it is treated as 0.
 */
void bElemAttr::getDefaultValues(int typeId, int subtypeId)
{
    auto sprites = configManager::getInstance()->getConfig()->sprites;
    for (unsigned long int c = 0; c < sprites.size(); c++)
    {
        auto sprite = sprites[c];
        if (sprite.eType != typeId)
            continue;
        for (unsigned long int c1 = 0; c1 < sprite.attributes.size(); c1++)
        {
            auto attr = sprite.attributes[c1];
            if ((!this->provisioned && attr.subType < 0) || (attr.subType == subtypeId))
            {
                this->setKillable(attr.killable);
                this->setDestroyable(attr.destroyable);
                this->setSteppable(attr.steppable);
                this->setMovable(attr.isMovable);
                this->setInteractive(attr.isInteractive);
                this->setCollectible(attr.isCollectible);
                this->setPush(attr.canPush);
                this->setPushed(attr.canBePushed);
                this->setCollect(attr.canCollect);
                this->setWeapon(attr.isWeapon);
                this->setOpen(attr.isOpen);
                this->setLocked(attr.isLocked);
                this->setEnergy(attr.energy);
                this->setMaxEnergy(attr.maxEnergy);
                this->setAmmo(attr.ammo);
                this->setMaxAmmo(attr.maxAmmo);
                this->provisioned = true; // this way we do not have to setup attributes for the walls.
                if(subtypeId>=0) break; // we can define the first one as -1, and then the 0 and positive subtypes would override it.
            }
        }
    }

}



bool bElemAttr::isMod() const
{
    return false;
}

int bElemAttr::getSubtype() const
{
    return this->subType;
}

void bElemAttr::setSubtype(int st)
{
    this->subType = st;
}

int bElemAttr::getEnergy() const
{
    return this->energy;
}

void bElemAttr::setEnergy(int e)
{
    this->energy = (unsigned int)(e>0)?e:0;
}

bool bElemAttr::isKillable() const
{
    return this->killable;
}
void bElemAttr::setKillable(bool k)
{
    this->killable = k;
}


bool bElemAttr::isDestroyable() const
{
    return this->destroyable;
}

void bElemAttr::setDestroyable(bool d)
{
    this->destroyable=d;
}

bool bElemAttr::isSteppable() const
{
    std::shared_ptr<bElem> owner=this->owner.lock();
    if(!owner)
        return this->steppable;
    return this->steppable && !owner->getStats()->isDying() && !owner->getStats()->isDestroying() && !owner->getStats()->isTeleporting();
}

void bElemAttr::setSteppable(bool s)
{
    this->steppable=s;
}

bool bElemAttr::isMovable() const
{
    std::shared_ptr<bElem> own=this->owner.lock();
    return this->movable && !own->getStats()->isDestroying() && !own->getStats()->isDying() && !own->getStats()->isTeleporting();;
}

void bElemAttr::setMovable(bool m)
{
    this->movable=m;
}

bool bElemAttr::isInteractive() const
{
    return this->interactive;
}

void bElemAttr::setInteractive(bool i)
{
    this->interactive=i;
}

bool bElemAttr::isCollectible() const
{
    return this->collectible;
}




void bElemAttr::setCollectible(bool c)
{
   this->collectible=c;
}

bool bElemAttr::canPush() const
{
    return this->push;
}

void bElemAttr::setPush(bool p)
{
    this->push=p;
}

bool bElemAttr::canBePushed() const
{
    return this->pushed;
}

void bElemAttr::setPushed(bool p)
{
    this->pushed=p;
}


/**
 * @brief Checks if the element has an associated inventory for collecting.
 *
 * This method checks if the element has an associated inventory for collecting. It returns true if the element has a valid inventory,
 * and false otherwise.
 *
 * @return True if the element has an inventory for collecting, false otherwise.
 */
bool bElemAttr::canCollect() const
{

    return (bool)this->inv;

}

/**
 * @brief Sets the collect flag for the element attribute.
 *
 * This method sets the collect flag for the element attribute. If the flag is set to true and the element does not have an inventory
 * and has an owner, a new inventory will be created and associated with the element.
 *
 * @param c The collect flag indicating whether the element can be collected.
 *
 * @note This method will create inventories for elements, but it will not destroy them. So, even if the element's type changes to
 * something that does not have an inventory, it will still have it.
 *
 * @note The state owner of the element should be set before calling this method.
 */
void bElemAttr::setCollect(bool c)
{
    if(c==true && !this->canCollect() && !this->owner.expired())
    {
        std::shared_ptr<inventory> newInv=std::make_shared<inventory>();
        newInv->changeOwner(this->owner.lock());
        this->inv=newInv;
    }

}

bool bElemAttr::isWeapon() const
{
    return this->weapon;
}

void bElemAttr::setWeapon(bool w)
{
    this->weapon=w;
}

bool bElemAttr::isOpen() const
{
    return this->open;
}

void bElemAttr::setOpen(bool o)
{
    this->open=o;
}

bool bElemAttr::isLocked() const
{
    return this->locked;
}

void bElemAttr::setLocked(bool l)
{
    this->locked=l;
}

int bElemAttr::getMaxEnergy() const
{
    return this->maxEnergy;
}

void bElemAttr::setMaxEnergy(int e)
{
    this->maxEnergy=e;
}

int bElemAttr::getAmmo() const
{
    return this->ammo;
}

void bElemAttr::setAmmo(int a)
{
    this->ammo=a;
}

int bElemAttr::getMaxAmmo() const
{
    return this->maxAmmo;
}

void bElemAttr::setMaxAmmo(int m)
{
    this->maxAmmo=m;
}


std::shared_ptr<inventory> bElemAttr::getInventory() const
{
    return this->inv;
}

void bElemAttr::setInventory(std::shared_ptr<inventory>inv)
{
    this->inv=inv;
}





