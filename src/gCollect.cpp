#include "gCollect.h"


gCollect* gCollect::instanceId=NULL;

gCollect::gCollect()
{
}

gCollect::~gCollect()
{
    this->purgeGarbage();
    gCollect::instanceId=NULL;

}
bool gCollect::purgeGarbage()
{
   for (auto p:this->garbageVector)
        delete p;
   this->garbageVector.clear();
   return true;
}

void gCollect::addToBin(bElem* element)
{
    this->garbageVector.push_back(element);
}
int gCollect::garbageQsize()
{
    return this->garbageVector.size();
}

gCollect* gCollect::getInstance()
{
    if(gCollect::instanceId==NULL)
        gCollect::instanceId=new gCollect();
    return gCollect::instanceId;
}
