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
        chamber(int x,int y);
        ~chamber();
        int getInstanceId();
    protected:

    private:
        void setInstanceId(int id);
        int instanceid;
        static int lastid;
};

#endif // CHAMBER_H
