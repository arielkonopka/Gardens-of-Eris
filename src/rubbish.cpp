#include "rubbish.h"


videoElement::videoElementDef* rubbish::vd=NULL;


rubbish::rubbish(chamber *board) : stillElem(board)
{
   // wall::wall(borad,garbage);

}

rubbish::~rubbish()
{
    //dtor
}
int rubbish::getType()
{
    return _rubishType;
}

int rubbish::getSubtype()
{
    return 0;
}


videoElement::videoElementDef* rubbish::getVideoElementDef()
{
    return rubbish::vd;
}

bool rubbish::isCollectible()
{
    return true;
}

