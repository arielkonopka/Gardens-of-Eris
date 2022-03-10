#include "videoElementDef.h"
namespace videoElement
{
videoElementDef::videoElementDef(int sts,int dirs,int anims)
{
    aphases *aph=new aphases(boost::extents[sts][dirs][anims]);

    this->subtypes=sts;
    this->animphases=anims;
    this->directions=dirs;
    this->defArray=aph;
    this->teleporting=new coordVector();
    this->dying=new coordVector();



}
void videoElementDef::setValuesFor(ALLEGRO_BITMAP *sprites,int s, int d, int a, int x, int y)
{
    (*this->defArray)[s][d][a]={x,y};
    this->sprites=sprites;

}



videoElementDef::~videoElementDef()
{
    //dtor
}

videoElementDef::videoElementDef(const videoElementDef& other)
{
    //copy ctor
}

void videoElementDef::addKill(int x, int y)
{
    this->dying->push_back((coords){x,y});
}

void videoElementDef::addTeleporting(int x, int y)
{
    this->teleporting->push_back((coords){x,y});

}

}
