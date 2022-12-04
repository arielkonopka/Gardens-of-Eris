#ifndef NONSTEPPABLE_H
#define NONSTEPPABLE_H

#include "bElem.h"
//#include "gCollect.h"
//#include "chamber.h"


#include "objectTypes.h"


class nonSteppable : virtual public bElem
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        nonSteppable(chamber *board);
        nonSteppable(chamber *board,int x, int y);
        virtual ~nonSteppable();
        bool isSteppable();


};

#endif // NONSTEPPABLE_H
