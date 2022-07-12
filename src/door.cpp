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

door::door(chamber* board) : bElem(board)
{
    this->initMe();
}
door::door(chamber* board, int x, int y) :bElem(board,x,y)
{
    this->initMe();
}
door::door(chamber* board, int subtype) :bElem(board)
{
    this->setSubtype(subtype);
    this->initMe();
}

door::door(chamber* board, int subtype, int x, int y):bElem(board,x,y)
{
    this->setSubtype(subtype);
    this->initMe();
}

void door::initMe()
{
    this->interacted=-1;
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


/* open the door if you can */
bool door::interact(bElem* who)
{
    bool bres=bElem::interact(who);
    bElem* key=NULL;
    //That should not happen, as we do not expect object randomly interacting with others without being programmed to do so.
    if(who->canInteract()==false)
    {
        return false;
    }
    //std::cout<<"can interact\n";
    if (this->locked==false)
    {
        this->interacted=this->getCntr()+10;
        this->open=!this->open;
        return true;
    }
    //If it cannot collect, it cannot hold a key.
    if (who->canCollect()==false || who->myInventory==NULL)
    {
        return false;
    }
    //if Door is unlocked, only open/close thing
#ifdef _VerbousMode_
std::cout<<"can not collect\n";
#endif
    if (this->getSubtype()%2==0)
    {
        key=who->myInventory->getKey(_key,this->getSubtype(),true);
#ifdef _VerbousMode_
        std::cout<<"Get key, even\n";
#endif
    }else
    {
        key=who->myInventory->getKey(_key,this->getSubtype(),false);
#ifdef _VerbousMode_
        std::cout<<"Get key, odd\n";
#endif
    }
    if(key!=NULL)
    {
       this->open=true;
       this->locked=false;
    } else
    {
        return false;
    }
    if (this->getSubtype()%2==0)
    {
        key->disposeElement();
    }
    return true;
}
bool door::isInteractive()
{
    return true;
}



door::~door()
{
    //dtor
}
