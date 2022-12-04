#include "door.h"

videoElement::videoElementDef* door::vd=nullptr;

videoElement::videoElementDef* door::getVideoElementDef()
{
    return door::vd;
}

int door::getType()
{
    return _door;
}

door::door(chamber* board) : bElem(board)
{

}

door::door(chamber* board, int subtype) :bElem(board)
{
    this->setSubtype(subtype);
}


bool door::isSteppable()
{
    return this->open;
}
bool door::isOpen()
{
    return this->open;
}
void door::stomp(bElem* who)
{
    bElem* key=nullptr;
    if(this->getSubtype()%2==1)
    {
        if(who->getInventory()!=nullptr)
        {
            key=who->getInventory()->getKey(_key,this->getSubtype(),true); // take the key on your way out. you don't have a key? Kill, could be used as tricky traps for monsters
            if(key==nullptr)
            {
                who->kill();
            }
        }
        else
        {
            who->kill();
        }
    }
    return; // True means you can safely continue
}
void door::unstomp()
{
   if(this->getSubtype()%2==1)
    {
        this->locked=true;
        this->open=false;
        this->setDirection((!this->open)?UP:LEFT);
    }
}


/* open the door if you can */
bool door::interact(bElem* who)
{
    bool bres=bElem::interact(who);
    bElem* key=nullptr;
    if (!bres)
        return false;
    //std::cout<<"can interact\n";
    if (this->locked==false)
    {
        this->interacted=this->getCntr()+10;
        this->open=!this->open;
        this->setDirection((!this->open)?UP:LEFT);
        return true;
    }
    //If it cannot collect, it cannot hold a key.
    if (who->canCollect()==false)
    {
        return false;
    }
    //if Door is unlocked, only open/close thing
    if (this->getSubtype()%2==0)
    {
        key=who->getInventory()->getKey(_key,this->getSubtype(),true);
    }
    else
    {
        key=who->getInventory()->getKey(_key,this->getSubtype(),false);
    }
    if(key!=nullptr)
    {
        this->open=true;
        this->locked=false;
        this->setDirection((!this->open)?UP:LEFT);
    }
    else
    {
        return false;
    }
    if (this->getSubtype()%2==0)
    {
        key->disposeElement();
    }
    return true;
}

bool door::canBeKilled()
{
    return false;
}

bool door::canBeDestroyed()
{
    return true;
}



door::~door()
{
    //dtor
}
