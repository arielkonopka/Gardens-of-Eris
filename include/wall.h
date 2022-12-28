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
        wall();
        explicit wall(std::shared_ptr<chamber> board);
        explicit wall(std::shared_ptr<chamber> board,int subtype);
        virtual  bool isDying();
        int getType();
        virtual bool canBeKilled();
        virtual bool canBeDestroyed();


};

#endif // WALL_H
