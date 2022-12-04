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
        bunker(chamber *board,int x, int y);
        virtual ~bunker();
        virtual bool mechanics();
        direction findLongestShot();
        virtual bool selfAlign();
        virtual bool interact(bElem* Who);
        virtual bool isMovable();
    protected:
        int help=0;
        plainGun *myGun;
        int rotated=0;
    private:
        elemStats* backUp=nullptr;
        bElem* activatedBy=nullptr;
};

#endif // BUNKER_H
