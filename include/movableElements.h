#ifndef MOVABLEELEMENTS_H
#define MOVABLEELEMENTS_H

#include "nonSteppable.h"
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"

#include "chamber.h"
#include "gCollect.h"
//#include "objectTypes.h"


#define _mov_delay   6
#define _mov_delay_push   6

class movableElements : public nonSteppable
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        movableElements(chamber *board,gCollect *garbage);
        virtual ~movableElements();
        bool isMovable();
        bool moveInDirection(direction dir);
        virtual int getType();
        virtual direction getDirection();
        virtual bool setDirection(direction newDirection);
        virtual int getSubtype();
        virtual bool canPush();
        virtual bool mechanics(bool collected);
        int _me_moved;
        direction facing;
        bool movable;
    protected:

    private:

        bool _me_canPush;

};

#endif // MOVABLEELEMENTS_H
