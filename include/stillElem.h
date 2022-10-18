#ifndef STILLELEM_H
#define STILLELEM_H

#include "nonSteppable.h"
#include "objectTypes.h"

class stillElem :  public nonSteppable
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        stillElem(chamber *board);
        ~stillElem();
        int getType();
    protected:

    private:
};

#endif // STILLELEM_H
