#ifndef BUNKER_H
#define BUNKER_H
#include "commons.h"
#include "plainGun.h"
#include <nonSteppable.h>


class bunker : public mechanical, public nonSteppable,public movableElements
{
    public:
        static videoElement::videoElementDef* vd;
        virtual videoElement::videoElementDef* getVideoElementDef();
        bunker(chamber *board);
        virtual ~bunker();
        virtual bool mechanics();
        direction findLongestShot();
        virtual bool selfAlign();
        virtual bool interact(bElem* Who);
        virtual bool isMovable();

    private:
        elemStats* backUp=nullptr;
        bElem* activatedBy=nullptr;
        int help=0;
        plainGun *myGun;
        int rotated=0;
};

#endif // BUNKER_H
