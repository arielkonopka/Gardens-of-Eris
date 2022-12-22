#ifndef BUNKER_H
#define BUNKER_H
#include "commons.h"
#include "plainGun.h"
#include <nonSteppable.h>
#include "bElem.h"


class bunker : public mechanical, public nonSteppable,public movableElements
{
    public:
        static videoElement::videoElementDef* vd;
        virtual videoElement::videoElementDef* getVideoElementDef();
        bunker();
        explicit bunker(std::shared_ptr<chamber> board);
        virtual ~bunker();
        virtual bool mechanics();
        direction findLongestShot();
        virtual bool selfAlign();
        virtual bool interact(std::shared_ptr<bElem> Who);
        virtual bool isMovable();

    private:
        std::shared_ptr<elemStats> backUp=nullptr;
        std::shared_ptr<bElem> activatedBy=nullptr;
        int help=0;
        std::shared_ptr<bElem> myGun;
};

#endif // BUNKER_H
