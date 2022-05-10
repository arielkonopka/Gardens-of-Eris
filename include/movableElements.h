#ifndef MOVABLEELEMENTS_H
#define MOVABLEELEMENTS_H

#include "mechanical.h"
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"

#include "chamber.h"
#include "gCollect.h"
//#include "objectTypes.h"



class movableElements : public mechanical
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        movableElements(chamber *board);
        movableElements(chamber *board,int x, int y);

        virtual ~movableElements();
        virtual bool isMovable();
        virtual bool moveInDirection(direction dir);
        virtual bool moveInDirectionSpeed(direction dir,int speed);
        virtual int getType();
        virtual direction getDirection();
        virtual bool setDirection(direction newDirection);
        virtual int getSubtype();
        virtual bool canPush();
        virtual bool mechanics(bool collected);
        virtual void setMoved(int time);
        virtual int getMoved();

        int _me_moved;
        direction facing;
        bool movable;
    protected:

    private:

        bool _me_canPush;

};

#endif // MOVABLEELEMENTS_H
