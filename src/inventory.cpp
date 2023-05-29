#include "inventory.h"
#include "bElem.h"




void inventory::changeOwner(std::shared_ptr<bElem> who)
{
    this->owner=who;
    for(auto w:this->weapons)
    {
        w->setDropped();
        w->setCollected(who);
    }
    for(auto t:this->tokens)
    {
        t->setDropped();
        t->setCollected(who);
    }
    for(auto k:this->keys)
    {
        k->setDropped();
        k->setCollected(who);
    }
    for(auto u:this->usables)
    {
        u->setDropped();
        u->setCollected(who);
    }
    for(auto m:this->mods)
    {
        m->setDropped();
        m->setCollected(who);
    }





}





bool inventory::removeCollectibleFromInventory(int instance)
{
#ifdef _VerbousMode_
    std::cout<<"Remove from Inventory: "<<instance<<"\n";
//   std::cout<<"[w";
#endif
    for(unsigned int c=0; c<this->weapons.size();)
    {
        if(this->weapons.at(c)->getInstanceid()==instance)
        {
#ifdef _VerbousMode_
            std::cout<<".";
#endif
            this->decrementTokenNumber({this->weapons.at(c)->getType(),this->weapons.at(c)->getSubtype()});
            this->weapons.erase(this->weapons.begin()+c);

        }
        else
        {
            c++;
        }

    }
    //  std::cout<<"u";
    for(unsigned int c=0; c<this->usables.size();)
    {
        if(this->usables.at(c)->getInstanceid()==instance)
        {
            this->decrementTokenNumber({this->usables.at(c)->getType(),this->usables.at(c)->getSubtype()});

            this->usables.erase(this->usables.begin()+c);
#ifdef _VerbousMode_
            std::cout<<".";
#endif
        }
        else
        {
            c++;
        }

    }
    for(unsigned int c=0; c<this->tokens.size();)
    {
        if(this->tokens.at(c)->getInstanceid()==instance)
        {
            this->decrementTokenNumber({this->tokens.at(c)->getType(),this->tokens.at(c)->getSubtype()});
            this->tokens.erase(this->tokens.begin()+c);
#ifdef _VerbousMode_
            std::cout<<".";
#endif

        }
        else
        {
            c++;
        }

    }
    for(unsigned int c=0; c<this->keys.size();)
    {
        if(this->keys.at(c)->getInstanceid()==instance)
        {
            this->decrementTokenNumber({this->keys.at(c)->getType(),this->keys.at(c)->getSubtype()});
            this->keys.erase(this->keys.begin()+c);
#ifdef _VerbousMode_
            std::cout<<".";
#endif

        }
        else
        {
            c++;
        }

    }
    for(unsigned int c=0; c<this->mods.size();)
    {
        if(this->mods.at(c)->getInstanceid()==instance)
        {
            this->decrementTokenNumber({this->mods.at(c)->getType(),this->mods.at(c)->getSubtype()});
#ifdef _VerbousMode_
            std::cout<<".";
#endif
            this->mods.erase(this->mods.begin()+c);

        }
        else
        {
            c++;
        }

    }
    return true;

}


std::shared_ptr<bElem> inventory::getKey(int type, int subtype,bool removeIt)
{
    /* finds a key in the inventory and returns it, when nothing found, nullptr is returned*/
    std::shared_ptr<bElem> res;
    for(size_t c=0; c<this->keys.size(); c++)
    {
        if(this->keys[c]->getType()==type && this->keys[c]->getSubtype()==subtype)
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
    this->decrementTokenNumber({this->weapons.at(this->wPos)->getType(),this->weapons.at(this->wPos)->getSubtype()});
    this->weapons.at(this->wPos)->disposeElement();
    return true;
}


std::shared_ptr<bElem> inventory::getActiveWeapon()
{
    if (this->weapons.size()<=0)
        return nullptr;

    this->wPos=this->wPos%this->weapons.size();
    if (this->weapons[this->wPos]->getAmmo()<=0)
    {
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
    // if(what->isDying() || what->isTeleporting() || what->isDestroyed())
    //     return false;

    what->setCollected(this->owner);

    this->incrementTokenNumber({what->getType(),what->getSubtype()});

    if (what->isWeapon()==true)
    {
        this->weapons.push_back(what);
        return true;
    }
    if (what->isUsable()==true)
    {
        this->usables.push_back(what);
        return true;
    }
    if(what->isMod()==true)
    {
        this->mods.push_back(what);
        return true;
    }
    if(what->getType()==_key)
    {
        this->keys.push_back(what);



        return true;
    }
    if(what->isCollectible()==true && what->getType()!=_rubishType)
    {
        // we do not collect stash items, we already merged its inventory
        if(what->getType()!=_stash)
            this->tokens.push_back(what); // the collect method should remove it from the board properly
        return true;
    }
    if(what->canCollect()==true)
    {
        /* this is probably a stash, or something like that. that is why, when we create an object, that shoots infinite ammo, it is better to have gun in non standard places, it would not be picked up that way*/
        this->mergeInventory(what->getInventory());
        what->setInventory(nullptr);
        what->disposeElement();
        res=true;
    }

    return res;
}

std::shared_ptr<bElem> inventory::requestToken(int type, int subType)
{
    for(size_t c=0; c<this->tokens.size(); c++)
    {
        if(this->tokens[c]->getType()==type && (subType<0 || this->tokens[c]->getSubtype()==subType)) // negative value of subtype will be ignored
        {
            std::shared_ptr<bElem> token;
            this->decrementTokenNumber( {type,subType});
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
            if(this->tokens[c]->getType()==type && this->tokens[c]->getSubtype()==subType)
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
    for(auto w:theOtherInventory->weapons)
    {
        this->incrementTokenNumber({w->getType(),w->getSubtype()});
        w->setDropped();
        w->setCollected(this->owner);
        this->weapons.push_back(w);
    }
    theOtherInventory->weapons.clear();
    for(auto u:theOtherInventory->usables)
    {
        this->incrementTokenNumber({u->getType(),u->getSubtype()});
        u->setDropped();
        u->setCollected(this->owner);
        this->usables.push_back(u);
    }
    theOtherInventory->usables.clear();

    for(auto m:theOtherInventory->mods)
    {
        this->incrementTokenNumber({m->getType(),m->getSubtype()});
        m->setDropped();
        m->setCollected(this->owner);
        this->mods.push_back(m);
    }
    theOtherInventory->mods.clear();
    for(auto t:theOtherInventory->tokens)
    {
        this->incrementTokenNumber({t->getType(),t->getSubtype()});
        t->setDropped();
        t->setCollected(this->owner);
        this->tokens.push_back(t);
    }
    theOtherInventory->tokens.clear();
    for(auto k:theOtherInventory->keys)
    {
        this->incrementTokenNumber({k->getType(),k->getSubtype()});
        k->setDropped();
        k->setCollected(this->owner);
        this->keys.push_back(k);
    }
    theOtherInventory->keys.clear();

    return true;
}



bool inventory::removeToken(int position)
{
    tType token;
    /* removes a token from tokens pocket, warning, it performs disposeElement on it, so it should not be referenced anywhere else!*/
    if(position>=(int)this->tokens.size())
        return false;
    token.tokenType=this->tokens[position]->getType();
    token.tokenSubtype=this->tokens[position]->getSubtype();
    this->decrementTokenNumber(token);
    this->tokens[position]->disposeElement();
    this->tokens.erase(this->tokens.begin()+position);
    return true;


}

bool inventory::findInInventory(int instanceId)
{
    for(auto k:this->keys)
        if(k->getInstanceid()==instanceId)
            return true;
    for(auto m:this->mods)
        if(m->getInstanceid()==instanceId)
            return true;
    for(auto t:this->tokens)
        if(t->getInstanceid()==instanceId)
            return true;
    for(auto w:this->weapons)
        if(w->getInstanceid()==instanceId)
            return true;
    return false;
}



bool inventory::isEmpty()
{

    return ((this->keys.size()==0) && (this->mods.size()==0) && (this->tokens.size()==0) && (this->weapons.size()==0));
}




void inventory::updateBoard()
{
    if(this->owner.get()==nullptr)
        return;
    std::shared_ptr<chamber> board=this->owner->getBoard();
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











