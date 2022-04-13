#ifndef BUNKER_H
#define BUNKER_H
#include "commons.h"
#include "plainGun.h"
#include <nonSteppable.h>


class bunker : public nonSteppable
{
    public:
        static videoElement::videoElementDef* vd;
        virtual videoElement::videoElementDef* getVideoElementDef();
        bunker(chamber *board,gCollect *garbageBin);
        bunker(chamber *board,gCollect *garbageBin,int x, int y);
        virtual ~bunker();
        virtual bool mechanics(bool collected);
        direction findLongestShot();
        virtual bool selfAlign();
        virtual bool interact(bElem* Who);
    protected:
        int interacted;
        int help;
        plainGun *myGun;
        int rotated;
    private:
};

#endif // BUNKER_H
