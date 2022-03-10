#ifndef NONSTEPPABLE_H
#define NONSTEPPABLE_H

#include "bElem.h"
#include "gCollect.h"
//#include "chamber.h"


#include "objectTypes.h"


class nonSteppable : public bElem
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        nonSteppable(chamber *board,gCollect *garbage);
        nonSteppable(chamber *board,gCollect *garbage,int x, int y);
        virtual bool mechanics(bool collected);
        virtual ~nonSteppable();
        bool isSteppable();
        virtual int getType();

    protected:

    private:
};

#endif // NONSTEPPABLE_H
