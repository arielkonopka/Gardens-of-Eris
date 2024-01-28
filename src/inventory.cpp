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
#include "inventory.h"
#include "bElem.h"




void inventory::changeOwner(std::shared_ptr<bElem> who)
{
    auto chg=[](std::shared_ptr<bElem>& w,std::vector<std::shared_ptr<bElem>>& vec)
    {
        for(auto w:vec)
        {
            w->getStats()->setCollector(w);
        }

    };
    this->owner=who;
    chg(who,this->weapons);
    chg(who,this->mods);
    chg(who,this->tokens);
    chg(who,this->usables);
    chg(who,this->keys);
}



/**
* @brief Removes a collectible item from the inventory based on its instance ID.
* This method removes a collectible item from the inventory by searching for its instance ID.
* It iterates through different categories of collectible items, such as weapons, usables, tokens, keys, and mods,
* and removes the item from the corresponding category if its instance ID matches the specified ID.
* The item is also decremented from the token count.
* @param instance The instance ID of the collectible item to be removed.
* @return true if the item was successfully removed, false otherwise.
*/

bool inventory::removeCollectibleFromInventory(unsigned long  int instance)
{
    return (this->retrieveCollectibleFromInventory(instance,true)==nullptr);
}

std::shared_ptr<bElem> inventory::retrieveCollectibleFromInventory(unsigned long int instanceId,bool removeIt)
{
    auto removeFromColl=[&](std::vector<std::shared_ptr<bElem>>& _vect,unsigned long int _inst )
    {
        std::shared_ptr<bElem> clc;
        for(unsigned int c=0; c<_vect.size();)
        {
            if(_vect[c]->getStats()->getInstanceId()==_inst)
            {
                _vect[c]->getStats()->setCollected(false);
                clc=_vect[c];
                if(removeIt)
                {
                    this->decrementTokenNumber({_vect[c]->getType(),_vect[c]->getAttrs()->getSubtype()});
                    _vect.erase(_vect.begin()+c);
                }
                return clc;
            }
            c++;
        }
        clc.reset();
        return clc;
    };
    std::shared_ptr<bElem> e=removeFromColl(this->weapons,instanceId);
    if(!e) e= removeFromColl(this->usables,instanceId);
    if(!e) e= removeFromColl(this->tokens,instanceId);
    if(!e) e= removeFromColl(this->keys,instanceId);
    if(!e) e= removeFromColl(this->mods,instanceId);
    return e;
}


std::shared_ptr<bElem> inventory::getKey(int type, int subtype,bool removeIt)
{
    /* finds a key in the inventory and returns it, when nothing found, nullptr is returned*/
    std::shared_ptr<bElem> res;
    for(size_t c=0; c<this->keys.size(); c++)
    {
        if(this->keys[c]->getType()==type && this->keys[c]->getAttrs()->getSubtype()==subtype)
        {
            res=this->keys[c];
            if (removeIt==true)
            {
                this->decrementTokenNumber({type,subtype});
                this->keys.erase(this->keys.begin()+c);
            }
            return res;
        }
    }
    return nullptr;
}

int inventory::countTokens(int type, int subtype)
{
    if(this->tokenNumbers.find({type,subtype})!= this->tokenNumbers.end())
    {
        tType token;
        token.tokenType=type;
        token.tokenSubtype=subtype;
        int result=this->tokenNumbers[token];
        return result;

    }
    return 0;
}



std::shared_ptr<bElem> inventory::getUsable()
{
    if(this->usables.size()<=0)
        return nullptr;
    if(!this->usables[this->uPos]->getStats()->isCollected())
        this->nextUsable();
    return this->usables[this->uPos];
}

bool inventory::nextUsable()
{
    if (this->usables.size()<=0)
        return false;
    this->uPos=this->cycleElement(this->usables,this->uPos);
    return true;
}

/*
here we care about the sequence, so we move the other elements. that would be a problem with a lot of weapons being removed at once.
*/
bool inventory::removeActiveWeapon()
{
    if((unsigned int)this->wPos>=this->weapons.size())
    {
        return false;
    }
    this->decrementTokenNumber({this->weapons.at(this->wPos)->getType(),this->weapons.at(this->wPos)->getAttrs()->getSubtype()});

   std::shared_ptr<bElem> be_=this->weapons.at(this->wPos);
    this->weapons.erase(this->weapons.begin()+this->wPos);
    this->wPos=this->wPos%this->weapons.size();

    be_->disposeElement();
    return true;
}


bool inventory::runLives()
{
    auto rl=[](std::vector<std::shared_ptr<bElem>> in)
    {
        for(auto e:in)
        {
            if(e->getStats()->hasActivatedMechanics())
                e->mechanics();
        }
    };
    rl(this->usables);
    rl(this->tokens);
    rl(this->keys);
    rl(this->weapons);
    rl(this->mods);
    return true;
}


std::shared_ptr<bElem> inventory::getActiveWeapon()
{
    if (this->weapons.size()<=0)
        return nullptr;
    this->wPos=this->wPos%this->weapons.size();
    if (this->weapons[this->wPos]->getAttrs()->getAmmo()<=0 || !this->weapons[this->wPos]->getStats()->isCollected())
    {
        this->removeActiveWeapon();
        return nullptr; // We will remove empty Weapons recursively, if it is necessary
    }
    return this->weapons[this->wPos];
}
int inventory::cycleElement(std::vector<std::shared_ptr<bElem>>& vec, int& pos)
{
    int p=pos;
    vec[p]->getStats()->setActive(false);
    p=(p+1) % vec.size();
    vec[p]->getStats()->setActive(true);
    return p;
}



bool inventory::nextGun()
{
    if (this->weapons.size()<=0)
        return false;
    this->wPos=this->cycleElement(this->weapons,this->wPos);
    return true;
}

bool inventory::addToInventory(std::shared_ptr<bElem> what)
{
    bool res=false;
    if(what==nullptr)
        return false;
    if(what->getStats()->isDying() || what->getStats()->isTeleporting() || what->getStats()->isDestroying())
        return false;

    what->getStats()->setCollector(this->owner.lock());
    std::shared_ptr<bElem> o=this->owner.lock();
    if(o)
        o->getStats()->setPoints(TOTAL,o->getStats()->getPoints(TOTAL)+1);
    this->incrementTokenNumber({what->getType(),what->getAttrs()->getSubtype()});
    if(what->getType()==_key)
    {
        this->keys.push_back(what);
        return true;
    }

    if (what->getAttrs()->isWeapon()==true)
    {
        this->weapons.push_back(what);
        return true;
    }
    if (what->getAttrs()->isInteractive())
    {
        this->usables.push_back(what);
        return true;
    }
    if(what->getAttrs()->isMod()==true)
    {
        this->mods.push_back(what);
        return true;
    }
    if(what->getAttrs()->isCollectible()==true && what->getType()!=_rubishType)
    {
        // we do not collect stash items, we already merged its inventory
        if(what->getType()!=_stash)
            this->tokens.push_back(what); // the collect method should remove it from the board properly
        return true;
    }
    if(what->getAttrs()->canCollect()==true)
    {
        /* this is probably a stash, or something like that. that is why, when we create an object, that shoots infinite ammo, it is better to have gun in non standard places, it would not be picked up that way*/
        this->mergeInventory(what->getAttrs()->getInventory());
        what->getAttrs()->setInventory(nullptr);
        what->disposeElement();
        res=true;
    }

    return res;
}

std::shared_ptr<bElem> inventory::requestToken(int type, int subType,bool removeIt)
{
    for(size_t c=0; c<this->tokens.size(); c++)
    {
        if(this->tokens[c]->getType()==type && (subType==-1 || this->tokens[c]->getAttrs()->getSubtype()==subType)) // negative value of subtype will be ignored
        {
            std::shared_ptr<bElem> token=this->tokens[c];
            if(removeIt)
            {
                this->decrementTokenNumber( {type,token->getAttrs()->getSubtype()});
                this->tokens.erase(this->tokens.begin()+c);
            }
            return token;
        }
    }
    return nullptr;
}


int inventory::requestTokens(int number, int type, int subType)
{
    int num=0;
    for(size_t c=0; c<this->tokens.size() && num<number;)
    {
        if(this->tokens[c]->getType()==type && (this->tokens[c]->getAttrs()->getSubtype()==subType || subType==-1) && num<number)
        {
            this->removeToken(c); // this also disposes the token! Use it with care
            num++;
        }
        else  c++;
    }
    return num;
}


void inventory::incrementTokenNumber(tType token)
{
    if(this->tokenNumbers.find(token)!= this->tokenNumbers.end())
    {
        this->tokenNumbers[token]+=1;
    }
    else
    {
        this->tokenNumbers[token]=1;
    }
}

void inventory::decrementTokenNumber(tType token)
{
    if(this->tokenNumbers.find(token)!= this->tokenNumbers.end())
    {
        this->tokenNumbers[token]-=1;

    }
    else
    {
        this->tokenNumbers[token]=1;

    }

}

/**
* @brief: merge external inventory with local, destroying the other one
* @params: theOtherInventory - pretty self explanatory
* @return: true on success
*/
bool inventory::mergeInventory(std::shared_ptr<inventory> theOtherInventory)
{
    if(theOtherInventory.get()==nullptr)
        return false;
    auto mergeList=[&](std::vector<std::shared_ptr<bElem>>& _vect1,std::vector<std::shared_ptr<bElem>>& _vect2)
    {
        for(auto item:_vect1)
        {
            this->incrementTokenNumber({item->getType(),item->getAttrs()->getSubtype()});
            item->getStats()->setCollected(false);
            item->collectOnAction(false,nullptr); // we let the element to react on dropping.
            item->getStats()->setCollector(this->owner.lock());
            item->getStats()->setCollected(true);
            item->collectOnAction(true,item->getStats()->getCollector().lock());
            _vect2.push_back(item);
        };
    };
    mergeList(theOtherInventory->weapons,this->weapons);
    mergeList(theOtherInventory->usables,this->usables);
    mergeList(theOtherInventory->mods,this->mods);
    mergeList(theOtherInventory->keys,this->keys);
    mergeList(theOtherInventory->tokens,this->tokens);
    theOtherInventory->weapons.clear();
    theOtherInventory->usables.clear();
    theOtherInventory->mods.clear();
    theOtherInventory->keys.clear();
    theOtherInventory->tokens.clear();
    return true;
}

/**
* @brief: removes a token from tokens, and disposes it
* @params: token position
* @returns: true on success
*/
bool inventory::removeToken(int position)
{
    tType token;
    /* removes a token from tokens pocket, warning, it performs disposeElement on it, so it should not be referenced anywhere else!*/
    if(position>=(int)this->tokens.size())
        return false;
    token.tokenType=this->tokens[position]->getType();
    token.tokenSubtype=this->tokens[position]->getAttrs()->getSubtype();
    this->decrementTokenNumber(token);
    this->tokens[position]->disposeElement();
    this->tokens.erase(this->tokens.begin()+position);
    return true;
}

bool inventory::findInInventory(unsigned long int instanceId)
{
    auto checkPresence=[](std::vector<std::shared_ptr<bElem>>& v,unsigned int inst)
    {
        for(auto k:v)
            if(k->getStats()->getInstanceId()==inst)
                return true;
        return false;
    };
    if(checkPresence(this->keys,instanceId))
        return true;
    if(checkPresence(this->mods,instanceId))
        return true;
    if(checkPresence(this->tokens,instanceId))
        return true;
    if(checkPresence(this->weapons,instanceId))
        return true;
    if(checkPresence(this->mods,instanceId))
        return true;
    if(checkPresence(this->usables,instanceId))
        return true;
    return false;
}



bool inventory::isEmpty()
{
    return ((this->keys.size()==0) && (this->mods.size()==0) && (this->tokens.size()==0) && (this->weapons.size()==0));
}




void inventory::updateBoard()
{
    if(this->owner.expired())
        return;
    std::shared_ptr<chamber> board=this->owner.lock()->getBoard();
    auto updateG=[&](std::vector<std::shared_ptr<bElem>>& _collection)
    {
        for(auto w:_collection)
        {
            w->setBoard(board);
        }
    };
    updateG(this->weapons);
    updateG(this->keys);
    updateG(this->mods);
    updateG(this->tokens);
    updateG(this->usables);

}











