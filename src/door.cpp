#include "door.h"

videoElement::videoElementDef* door::vd=NULL;

videoElement::videoElementDef* door::getVideoElementDef()
{
    return door::vd;
}

int door::getType()
{
    return _door;
}

door::door(chamber* board, gCollect *garbage) : bElem(board,garbage)
{
    this->initMe();
}
door::door(chamber* board, gCollect* garbage, int x, int y) :bElem(board,garbage,x,y)
{
    this->initMe();
}
door::door(chamber* board, gCollect* garbage, int subtype) :bElem(board,garbage)
{
    this->setSubtype(subtype);
    this->initMe();
}

door::door(chamber* board, gCollect* garbage, int subtype, int x, int y):bElem(board,garbage,x,y)
{
    this->setSubtype(subtype);
    this->initMe();
}

void door::initMe()
{
    this->open=false;
    this->locked=true;
}
bool door::isSteppable()
{
    return this->open;
}
bool door::isOpen()
{
    return this->open;
}

bool door::interact(bElem* who)
{

    //That should not happen, as we do not expect object randomly interacting with others without being programmed to do so.
    if(who->canInteract()==false)
    {
        return false;
    }
    //If it cannot collect, it cannot hold a key.
    if (who->canCollect()==false || who->collectedItems.size()==0)
    {
        return false;
    }
    //if Door is unlocked, only open/close thing
    if (this->locked==false)
    {
        this->open=!this->open;
        return true;
    }
    for(int c=0; c<who->collectedItems.size(); c++)
    {
#ifdef debug
        if(who->collectedItems[c]->getType()==_key)
        {
            std::cout<<" %%% "<<who->collectedItems[c]->getSubtype()<<" "<<this->getSubtype()<<"\n";
        }
#endif
        if (who->collectedItems[c]->getType()==_key && who->collectedItems[c]->getSubtype()==this->getSubtype())
        {
#ifdef debug
            std::cout<<"Door Open!\n";
#endif
            this->open=true;
            this->locked=false;
            if(who->collectedItems[c]->getSubtype()%2==0)
            {
                std::cout<<who->removeFromcollection(c)<<"---<\n";
            }
            return true;
        }
    }
    return false;
}
bool door::isInteractive()
{
    return true;
}


door::~door()
{
    //dtor
}
