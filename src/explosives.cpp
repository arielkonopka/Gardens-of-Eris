#include "explosives.h"


videoElement::videoElementDef* explosives::vd=NULL;

explosives::explosives(chamber *board):movableElements(board)
{
    //ctor
}

explosives::explosives(chamber* board, int x, int y):movableElements(board,x,y)
{

}



explosives::~explosives()
{
    //dtor
}



videoElement::videoElementDef* explosives::getVideoElementDef()
{
    return explosives::vd;
}

int explosives::getType()
{
    return _explosivesType;
}
bool explosives::explode()
{
    bElem *el;
  //  if(this->isDestroyed() || this->isDying() || this->canBeDestroyed()==false)
  //      return true;

    for (int cnt=0 ; cnt<4; cnt++)
    {
        el=this->getElementInDirection((direction)(cnt));
        if (el!=NULL)
        {
            el->destroy();
            el=el->getElementInDirection((direction)((cnt+1)%4));
            if(el!=NULL)
                el->destroy();
            std::cout<<".";
        }
    }

    return false;
}

bool explosives::hurt(int points)
{
   // this->explode();


    return this->destroy();
}


bool explosives::canBeDestroyed()
{
    return true;
}

bool explosives::canBeKilled()
{
    return true;
}
bool explosives::kill()
{
   // this->explode();
    return this->destroy();
}

bool explosives::destroy()

{
  if(movableElements::destroy())
	this->explode();
return true;

}


