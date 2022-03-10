#ifndef CHAMBER_H
#define CHAMBER_H
#include <boost/multi_array.hpp>
#include "commons.h"
//#include  "bElem.h"
#include "gCollect.h"
class bElem;
class gCollect;
//using boost::multi_array;
class chamber
{
    public:
        controlItem cntrlItm;
        int width;;
        int height;
        coords player;
        gCollect *garbageBin;
        bElem* getElement(int x, int y);
        boost::multi_array<bElem *,2> chamberArray;
        chamber(int x,int y,gCollect *garbageBin);
        ~chamber();

    protected:

    private:

};

#endif // CHAMBER_H
