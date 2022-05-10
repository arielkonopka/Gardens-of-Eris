#ifndef NONSTEPPABLE_H
#define NONSTEPPABLE_H

#include "bElem.h"
//#include "gCollect.h"
//#include "chamber.h"


#include "objectTypes.h"


class nonSteppable : public bElem
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        nonSteppable(chamber *board);
        nonSteppable(chamber *board,int x, int y);
        virtual bool mechanics(bool collected);
        virtual ~nonSteppable();
        bool isSteppable();
        virtual int getType();
        virtual void setTeleporting(int time);
        virtual bool isTeleporting();
    protected:
        int teleporting;
    private:

};

#endif // NONSTEPPABLE_H
