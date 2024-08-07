
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

#ifndef bELEMATTR_H
#define bELEMATTR_H
#include <memory>
#include "../include/configManager.h"

class bElem;
class inventory;


/**
 * @brief The bElemAttr class represents the attributes of an element.
 *
 * The bElemAttr class stores and manages the attributes of an element, such as its subtype, killability, destroyability,
 * movability, and other properties. It provides methods to get and set these attributes, as well as a constructor to
 * initialize the attributes for a specific owner, type, and subtype.
 */
class bElemAttr
{
public:
    bElemAttr(std::shared_ptr<bElem> owner, int type, int subtype);
    int getSubtype() const;
    std::shared_ptr<inventory> getInventory() const;
    int getEnergy() const;
    int getMaxEnergy() const;
    int getAmmo() const;
    int getMaxAmmo() const;
    void setSubtype(int st);
    void setInventory(std::shared_ptr<inventory> inv);
    void setEnergy(int e);
    void setMaxEnergy(int e);
    void setAmmo(int a);
    void setMaxAmmo(int m);
    void setKillable(bool k);
    void setDestroyable(bool d);
    void setSteppable(bool s);
    void setMovable(bool m);
    void setInteractive(bool i);
    void setCollectible(bool c);
    void setPush(bool p);
    void setPushed(bool p);
    void setCollect(bool c);
    void setWeapon(bool w);
    void setOpen(bool o);
    void setLocked(bool l);
    bool isKillable() const;
    bool isDestroyable() const;
    bool isSteppable() const;
    bool isMovable() const;
    bool isInteractive() const;
    bool isCollectible() const;
    bool isWeapon() const;
    bool isOpen() const;
    bool isLocked() const;
    bool isMod() const;

    bool canPush() const;
    bool canBePushed() const;
    bool canCollect() const;

private:
    void getDefaultValues(int typeId, int subtypeId);
    std::shared_ptr<inventory> inv=nullptr;
    bool provisioned = false;
    std::weak_ptr<bElem> owner;
    int bElemType = -1;
    int subType = -1;
    bool killable = false;
    bool destroyable = false;
    bool steppable = true;
    bool movable = false;
    bool interactive = false;
    bool collectible = false;
    bool push = false; /* can the object push other objects? */
    bool pushed = false; /* can the object be pushed? */
    bool collect = false;
    bool weapon = false;
    bool open = false;
    bool locked = false;
    int energy = 100;
    int maxEnergy = 100;
    int ammo = 0;
    int maxAmmo = 0;
};

#endif
