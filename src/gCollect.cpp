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
    for(int cnt=0;cnt<this->garbageVector.size();)
    {
        if(this->garbageVector.at(cnt)->isLocked()) //We do not touch the locked elements, these are probably waiting for something, and cannot be purged.
        {
            cnt++;
            continue;
        }
        delete this->garbageVector.at(cnt);
        this->garbageVector.erase(this->garbageVector.begin()+cnt);

    }
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
