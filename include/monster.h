#ifndef MONSTER_H
#define MONSTER_H
#include "objectTypes.h"
#include "chamber.h"
#include "gCollect.h"
#include "videoElementDef.h"
#include <movableElements.h>


class monster : public movableElements
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        monster(chamber *board,gCollect *garbage);
        ~monster();
        int getType();
        int getSubtype();
        virtual bool mechanics(bool collected);
        int getAnimPh();

        bool steppableNeigh();
        bool isDying();
    protected:

    private:

        int animph;
        int internalCnt;

};

#endif // MONSTER_H
