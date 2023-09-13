#include "videoElementDef.h"
#include "elements.h"
namespace videoElement
{
videoElementDef::videoElementDef(ALLEGRO_BITMAP *sprites_)
{

    this->sprites=sprites_;



}



void videoElementDef::initializeDriver()
{

}








videoElementDef::~videoElementDef()
{
    //dtor
}

videoElementDef::videoElementDef(const videoElementDef& other)
{
    //copy ctor
}

}
