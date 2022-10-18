#include "rubbish.h"


videoElement::videoElementDef* rubbish::vd=NULL;


rubbish::rubbish(chamber *board) : stillElem(board)
{
   // wall::wall(borad,garbage);
   this->setSubtype(0);

}

rubbish::~rubbish()
{
    //dtor
}
int rubbish::getType()
{
    return _rubishType;
}




videoElement::videoElementDef* rubbish::getVideoElementDef()
{
    return rubbish::vd;
}

bool rubbish::isCollectible()
{
    return true;
}

