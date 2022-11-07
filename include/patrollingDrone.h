#ifndef PATROLLINGDRONE_H
#define PATROLLINGDRONE_H

#include <killableElements.h>
#include "commons.h"
#include "videoElementDef.h"

class patrollingDrone : public killableElements, public nonSteppable, public mechanical, public movableElements
{
    public:
        virtual videoElement::videoElementDef* getVideoElementDef();
        static videoElement::videoElementDef* vd;
        patrollingDrone(chamber* board);
        patrollingDrone(chamber* board,int x, int y);
        virtual ~patrollingDrone();
        virtual bool mechanics();
        virtual int findSomething(bElem* elem,int n,int denyDir);
        virtual bool interact(bElem* who);
        int getType();

    protected:

    private:
        void setVisited(int x, int y);
        bool wasVisited(int x, int y);
        void clearVisited();
    coords boardSize;
        bool **steppables;
};

#endif // PATROLLINGDRONE_H
