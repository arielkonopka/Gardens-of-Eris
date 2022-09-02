#ifndef PATROLLINGDRONE_H
#define PATROLLINGDRONE_H

#include <killableElements.h>
#include "commons.h"
#include "videoElementDef.h"

class patrollingDrone : public killableElements
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        patrollingDrone(chamber* board);
        patrollingDrone(chamber* board,int x, int y);
        virtual ~patrollingDrone();
        virtual bool mechanics(bool collected);
        virtual int findSomething(bElem* elem,int n,int denyDir);
        virtual bool interact(bElem* who);
        int getType();
    protected:

    private:
    coords boardSize;
        bool **steppables;
};

#endif // PATROLLINGDRONE_H
