#include "rubbish.h"


videoElement::videoElementDef* rubbish::vd=nullptr;


rubbish::rubbish(chamber *board) : bElem(board),nonSteppable(board),collectible(board)
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

