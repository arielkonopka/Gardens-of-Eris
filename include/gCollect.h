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
        gCollect();
        ~gCollect();
        bool purgeGarbage();
        void addToBin(bElem *element);
        int garbageQsize();

    protected:

    private:
        std::vector<bElem *> garbageVector;
};

#endif // GCOLLECT_H
