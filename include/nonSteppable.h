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
        nonSteppable();
        explicit nonSteppable(std::shared_ptr<chamber> board);
        explicit nonSteppable(std::shared_ptr<chamber> board,int x, int y);
        bool isSteppable();


};

#endif // NONSTEPPABLE_H
