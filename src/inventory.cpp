#include "inventory.h"
#include "bElem.h"
inventory::inventory()
{

    this->wPos=0;
    this->uPos=0;

}
inventory::~inventory()
{
    //We place the objects in the garbage collector.
    for(auto  w:this->weapons)
        w->disposeElement();
    for(auto u: this->usables)
        u->disposeElement();
    for(auto t:this->tokens)
        t->disposeElement();
    for(auto k:this->keys)
        k->disposeElement();
    for(auto m:this->mods)
        m->disposeElement();

}

bElem* inventory::getKey(int type, int subtype,bool removeIt)
{
    /* finds a key in the inventory and returns it, when nothing found, NULL is returned*/
    bElem* res;
    for(size_t c=0; c<this->keys.size(); c++)
    {
        if(this->keys[c]->getType()==type && this->keys[c]->getSubtype()==subtype)
        {
            res=this->keys[c];
            if (removeIt==true)
            {
                this->keys.erase(this->keys.begin()+c);
            }
            return res;
        }
    }
    return NULL;
}




bElem* inventory::getUsable()
{
    if(this->usables.size()<=0)
        return NULL;
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
    /* if(this->wPos>this->weapons.size)
     {
         this->wPos=0;
     } */
    this->weapons.at(this->wPos)->disposeElement();
    this->weapons.erase(this->weapons.begin()+this->wPos);
    if (this->wPos>=(int)this->weapons.size())
        this->wPos=0;
    return true;
}


bElem* inventory::getActiveWeapon()
{
    if (this->weapons.size()<=0)
        return NULL;

    if (this->weapons[this->wPos]->getAmmo()<=0)
    {
        std::cout<<"remove weapon\n";
        this->removeActiveWeapon();
        return this->getActiveWeapon(); // We will remove empty Weapons recursively, if it is necessary
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

bool inventory::addToInventory(bElem* what)
{
    if(what==NULL)
        return false;
    what->deregisterLiveElement(what);
    if(what->myInventory!=NULL)
    {
        /* this is probably a stash, or something like that. that is why, when we create an object, that shoots infinite ammo, it is better to have gun in non standard places, it would not be picked up that way*/
        this->mergeInventory(what->myInventory);
        delete what->myInventory;
        what->myInventory=NULL;

    }

    if (what->isWeapon()==true)
    {
        this->weapons.push_back(what->removeElement());
        return true;
    }
    if (what->isUsable()==true)
    {
        this->usables.push_back(what->removeElement());
        return true;
    }
    if(what->isMod()==true)
    {
        this->mods.push_back(what->removeElement());
        return true;
    }
    if(what->getType()==_key)
    {
        this->keys.push_back(what->removeElement());
        return true;
    }
    if(what->isCollectible()==true && what->getType()!=_rubishType)
    {
        // we do not collect stash items, we already merged its inventory
        if(what->getType()!=_stash)
            this->tokens.push_back(what->removeElement());
        return true;
    }
    if (what->getType()==_rubishType)
    {
        what->disposeElement();
    }
    return false;
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

bool inventory::mergeInventory(inventory* theOtherInventory)
{
    if(theOtherInventory==NULL)
        return false;
    for(auto w:theOtherInventory->weapons)
    {
        this->weapons.push_back(w);
    }
    theOtherInventory->weapons.clear();
    for(auto u:theOtherInventory->usables)
    {
        this->usables.push_back(u);
    }
    theOtherInventory->usables.clear();

    for(auto m:theOtherInventory->mods)
    {
        this->mods.push_back(m);
    }
    theOtherInventory->mods.clear();
    for(auto t:theOtherInventory->tokens)
    {
        this->tokens.push_back(t);
    }
    theOtherInventory->tokens.clear();
    for(auto k:theOtherInventory->keys)
    {
        this->keys.push_back(k);
    }
    theOtherInventory->keys.clear();

    return true;
}



bool inventory::removeToken(int position)
{
    /* removes a token from tokens pocket, warning, it performs disposeElement on it, so it should not be referenced anywhere else!*/
    if(position>=(int)this->tokens.size())
        return false;
    this->tokens[position]->disposeElement();
    this->tokens.erase(this->tokens.begin()+position);
    return true;


}

void inventory::mechanics()
{
    for(auto n:this->weapons)
        n->mechanics(true);

}

