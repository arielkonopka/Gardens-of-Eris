#ifndef FLOOR_H
#define FLOOR_H

#include "bElem.h"


class floorElement : public bElem
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        floorElement();
        floorElement(chamber* board);
        virtual ~floorElement();
        virtual constexpr int getType() { return _floorType; };
        virtual constexpr bool canBeDestroyed() { return false; };
        virtual constexpr bool canBeKilled() { return false; };

};

#endif // FLOOR_H
