#include "rubbish.h"


videoElement::videoElementDef* rubbish::vd=NULL;


rubbish::rubbish(chamber *board, gCollect *garbage) : stillElem(board,garbage)
{
   // wall::wall(borad,garbage);
    this->garbageBin=garbage;
    this->attachedBoard=board;
    this->x=-1;
    this->y=-1;
    this->steppingOn=NULL;
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


