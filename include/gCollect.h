#ifndef GCOLLECT_H
#define GCOLLECT_H
#include <vector>
#include <iostream>
#include "bElem.h"
class bElem;


/*
 * Garbage collector class.
 * here will be linked objects that are to be purged from the memory
 *
 */
class gCollect
{
    public:

        ~gCollect();
        bool purgeGarbage();
        void addToBin(bElem *element);
        int garbageQsize();
        static gCollect* getInstance();
         std::vector<bElem *> garbageVector;

    private:
        gCollect();
        static gCollect *instanceId;

};

#endif // GCOLLECT_H
