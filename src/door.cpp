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
#include "door.h"

bool door::destroy()
{
    if(this->getAttrs()->getSubtype()%2==1)
    {
        std::shared_ptr<bElem> newWall=elementFactory::generateAnElement<wall>(this->getBoard(),1);
        std::shared_ptr<bElem> se=this->getStats()->getSteppingOn();
        this->removeElement();
        newWall->stepOnElement(se);
        this->disposeElement();
        return true;
    }
    return bElem::destroy();
}


int door::getType() const
{
    return _door;
}

door::door() : bElem()
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
    this->getAttrs()->setSteppable(false);
    this->getAttrs()->setLocked(true);
    this->getAttrs()->setOpen(false);
    return res;
}

bool door::stepOnAction(bool step, std::shared_ptr<bElem>who)
{
    if(who==nullptr)
        return false;
    if(step==true)
    {
        if(this->getAttrs()->getSubtype()%2==1 )
        {
            std::shared_ptr<bElem> k;
            if( who->getAttrs()->canCollect())
            {
                k=who->getAttrs()->getInventory()->getKey(_key,this->getAttrs()->getSubtype(),true);
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
        if(this->getAttrs()->getSubtype()%2==1 )
        {
            this->getAttrs()->setOpen(false);
            this->getAttrs()->setSteppable(this->getAttrs()->isOpen());
            this->getAttrs()->setLocked(true);
            this->playSound("Door", (this->getAttrs()->isOpen()) ? "Unlock" : "Lock");
            this->getStats()->setFacing((!this->getAttrs()->isOpen()) ? UP : LEFT);
        }
    }
    return true;
}


void door::_alignWithOpen()
{
    this->getStats()->setFacing((!this->getAttrs()->isOpen()) ? UP : LEFT);
    this->getAttrs()->setSteppable(this->getAttrs()->isOpen());

}

/* open the door if you can */
bool door::interact(std::shared_ptr<bElem> who)
{

    bool bres = bElem::interact(who);
    std::shared_ptr<bElem> key = nullptr;
    if (!bres)
        return false;
    if (!this->getAttrs()->isLocked())
    {
        this->getStats()->setInteracted(10);
        who->getStats()->setWaiting(1);
        this->getAttrs()->setOpen(!this->getAttrs()->isOpen());
        this->_alignWithOpen();
        this->playSound("Door", (this->getAttrs()->isOpen()) ? "Unlock" : "Lock");
        return true;
    }
    if (!who->getAttrs()->canCollect())
    {
        return false;
    }
    key = who->getAttrs()->getInventory()->getKey(_key, this->getAttrs()->getSubtype(), this->getAttrs()->getSubtype()%2!=1);
    if (key != nullptr)
    {
        this->playSound("Door", "Open");
        this->getAttrs()->setOpen(true);
        this->getAttrs()->setLocked(false);
        this->_alignWithOpen();
    }
    else
    {
        return false;
    }
    if(key &&  this->getAttrs()->getSubtype()%2!=1)
    {
        this->playSound("Door","CollectKey");
        key->disposeElement();
    }
    return true;
}

