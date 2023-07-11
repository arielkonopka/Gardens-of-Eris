#include "inventory.h"
#include "bElem.h"




void inventory::changeOwner(std::shared_ptr<bElem> who)
{
    this->owner=who;
    for(auto w:this->weapons)
    {
        w->status->setCollector(who);
    }
    for(auto t:this->tokens)
    {
        t->status->setCollector(who);
    }
    for(auto k:this->keys)
    {
        k->status->setCollector(who);
    }
    for(auto u:this->usables)
    {
        u->status->setCollector(who);
    }
    for(auto m:this->mods)
    {
        m->status->setCollector(who);
    }





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
    auto removeFromColl=[&](std::vector<std::shared_ptr<bElem>>& _vect,unsigned long int _inst )
    {
        for(unsigned int c=0; c<_vect.size();)
        {
            if(_vect[c]->status->getInstanceId()==_inst)
            {
                _vect[c]->status->setCollected(false);
                this->decrementTokenNumber({_vect[c]->getType(),_vect[c]->attrs->getSubtype()});
                _vect.erase(_vect.begin()+c);
                continue;
            }
            c++;
        }
    };
    removeFromColl(this->weapons,instance);
    removeFromColl(this->usables,instance);
    removeFromColl(this->tokens,instance);
    removeFromColl(this->keys,instance);
    removeFromColl(this->mods,instance);
    return true;

}


std::shared_ptr<bElem> inventory::getKey(int type, int subtype,bool removeIt)
{
    /* finds a key in the inventory and returns it, when nothing found, nullptr is returned*/
    std::shared_ptr<bElem> res;
    for(size_t c=0; c<this->keys.size(); c++)
    {
        if(this->keys[c]->getType()==type && this->keys[c]->attrs->getSubtype()==subtype)
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
    return this->usables[this->uPos];
}

bool inventory::nextUsable()
{
    if (this->usables.size()<=0)
        return false;
    this->uPos=(this->uPos+1) % this->usables.size();
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
    this->decrementTokenNumber({this->weapons.at(this->wPos)->getType(),this->weapons.at(this->wPos)->attrs->getSubtype()});
    this->weapons.at(this->wPos)->disposeElement();
    return true;
}


std::shared_ptr<bElem> inventory::getActiveWeapon()
{
    if (this->weapons.size()<=0)
        return nullptr;

    this->wPos=this->wPos%this->weapons.size();
 //   std::cout<<" * Wpos: "<<this->wPos<<"\n";
    if (this->weapons[this->wPos]->attrs->getAmmo()<=0)
    {
  //      std::cout<<" * Wpos: removing weapon - empty"<<this->wPos<<"\n";

        this->removeActiveWeapon();
        return nullptr; // We will remove empty Weapons recursively, if it is necessary
    }
    return this->weapons[this->wPos];
}

bool inventory::nextGun()
{
    if (this->weapons.size()<=0)
        return false;
    this->wPos=(this->wPos+1) % this->weapons.size();
    return true;
}

bool inventory::addToInventory(std::shared_ptr<bElem> what)
{
    bool res=false;
    if(what==nullptr)
        return false;
    // if(what->isDying() || what->isTeleporting() || what->status->isDestroying())
    //     return false;

    what->status->setCollector(this->owner.lock());

    this->incrementTokenNumber({what->getType(),what->attrs->getSubtype()});
    if(what->getType()==_key)
    {
        this->keys.push_back(what);
        return true;
    }

    if (what->attrs->isWeapon()==true)
    {
    std::cout<<"* collect Weapon\n";
        this->weapons.push_back(what);
        return true;
    }
    if (what->attrs->canInteract())
    {
        this->usables.push_back(what);
        return true;
    }
    if(what->attrs->isMod()==true)
    {
        this->mods.push_back(what);
        return true;
    }
    if(what->attrs->isCollectible()==true && what->getType()!=_rubishType)
    {
        // we do not collect stash items, we already merged its inventory
        if(what->getType()!=_stash)
            this->tokens.push_back(what); // the collect method should remove it from the board properly
        return true;
    }
    if(what->attrs->canCollect()==true)
    {
        /* this is probably a stash, or something like that. that is why, when we create an object, that shoots infinite ammo, it is better to have gun in non standard places, it would not be picked up that way*/
        this->mergeInventory(what->attrs->getInventory());
        what->attrs->setInventory(nullptr);
        what->disposeElement();
        res=true;
    }

    return res;
}

std::shared_ptr<bElem> inventory::requestToken(int type, int subType)
{
    for(size_t c=0; c<this->tokens.size(); c++)
    {
        std::cout<<"  ** "<<this->tokens[c]->getType()<<"\n";
        if(this->tokens[c]->getType()==type && (subType==-1 || this->tokens[c]->attrs->getSubtype()==subType)) // negative value of subtype will be ignored
        {
            std::cout<<" *** accepted\n";
            std::shared_ptr<bElem> token=this->tokens[c];
            this->decrementTokenNumber( {type,token->attrs->getSubtype()});
            this->tokens.erase(this->tokens.begin()+c);
            return token;
        }
    }
    return nullptr;
}


int inventory::requestTokens(int number, int type, int subType)
{
    bool found=false;
    for(int n=number; n>0; n--)
    {
        found=false;
        for(size_t c=0; c<this->tokens.size(); c++)
        {
            if(this->tokens[c]->getType()==type && this->tokens[c]->attrs->getSubtype()==subType)
            {
                found=true;
                this->decrementTokenNumber( {type,subType});
                this->removeToken(c); // this also disposes the token! Use it with care
                break;
            }
        }
        if(found==false)
        {
            return n;
        }
    }
    return 0;
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




bool inventory::mergeInventory(std::shared_ptr<inventory> theOtherInventory)
{
    if(theOtherInventory.get()==nullptr)
        return false;
    auto mergeList=[&](std::vector<std::shared_ptr<bElem>>& _vect1,std::vector<std::shared_ptr<bElem>>& _vect2)
    {
        for(auto item:_vect1)
        {
            this->incrementTokenNumber({item->getType(),item->attrs->getSubtype()});
            item->status->setCollected(false);
            item->status->setCollector(this->owner.lock());
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



bool inventory::removeToken(int position)
{
    tType token;
    /* removes a token from tokens pocket, warning, it performs disposeElement on it, so it should not be referenced anywhere else!*/
    if(position>=(int)this->tokens.size())
        return false;
    token.tokenType=this->tokens[position]->getType();
    token.tokenSubtype=this->tokens[position]->attrs->getSubtype();
    this->decrementTokenNumber(token);
    this->tokens[position]->disposeElement();
    this->tokens.erase(this->tokens.begin()+position);
    return true;


}

bool inventory::findInInventory(unsigned long int instanceId)
{
    for(auto k:this->keys)
        if(k->status->getInstanceId()==instanceId)
            return true;
    for(auto m:this->mods)
        if(m->status->getInstanceId()==instanceId)
            return true;
    for(auto t:this->tokens)
        if(t->status->getInstanceId()==instanceId)
            return true;
    for(auto w:this->weapons)
        if(w->status->getInstanceId()==instanceId)
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
    for(auto w:this->weapons)
    {
        w->setBoard(board);
    }
    for(auto k:this->keys)
    {
        k->setBoard(board);
    }
    for(auto m:this->mods)
    {
        m->setBoard(board);
    }
    for(auto t:this->tokens)
    {
        t->setBoard(board);
    }

}











