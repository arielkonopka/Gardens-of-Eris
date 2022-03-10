#ifndef WALL_H
#define WALL_H

#include "nonSteppable.h"
#include "videoElementDef.h"

#include "objectTypes.h"



class wall : public nonSteppable
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        wall(chamber *board,gCollect *garbage);
        wall(chamber *board,gCollect *garbage,int subtype);
        virtual  bool isDying();
        virtual ~wall();
        int getSubtype();
        int getType();
    protected:

    private:
        int subtype;
};

#endif // WALL_H
