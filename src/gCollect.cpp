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

/*
 * Purge the garbage return false, if no objects were deleted
 *
 */
bool gCollect::purgeGarbage()
{
    bool res=false;
    for(unsigned int cnt=0;cnt<this->garbageVector.size();)
    {
        if(this->garbageVector.at(cnt)->isLocked()) //We do not touch the locked elements, these are probably waiting for something, and cannot be purged.
        {
            cnt++;
            continue;
        }
        res=true;
        delete this->garbageVector.at(cnt);
        this->garbageVector.erase(this->garbageVector.begin()+cnt);

    }
   return res;
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
