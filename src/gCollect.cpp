#include "gCollect.h"

gCollect::gCollect()
{
    //ctor
}

gCollect::~gCollect()
{
    this->purgeGarbage();

}
bool gCollect::purgeGarbage()
{
   for (unsigned int cnt=0;cnt<this->garbageVector.size();cnt++)
    {
      //  std::cout<<".";
        bElem *tmp=this->garbageVector[cnt];
        delete tmp;
    }
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
