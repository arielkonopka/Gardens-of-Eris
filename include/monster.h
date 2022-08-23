#ifndef MONSTER_H
#define MONSTER_H
#include "objectTypes.h"
#include "chamber.h"
#include "gCollect.h"
#include "videoElementDef.h"
#include "killableElements.h"


class monster : public killableElements
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        monster(chamber *board);
        monster(chamber *board,int newSubtype);
        ~monster();
        int getType();
        int getSubtype();
        virtual bool mechanics(bool collected);


        bool steppableNeigh();

    protected:

    private:

        int animph;
        int internalCnt;

};

#endif // MONSTER_H
