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
        patrollingDrone();
        explicit patrollingDrone(std::shared_ptr<chamber> board);
        virtual ~patrollingDrone();
        virtual bool mechanics();
        virtual int findSomething(std::shared_ptr<bElem> elem,int n,int denyDir);
        virtual bool interact(std::shared_ptr<bElem> who);
        int getType();



    private:
        void setVisited(int x, int y);
        bool wasVisited(int x, int y);
        void clearVisited();
    coords boardSize;
        std::vector<std::vector<bool>> steppables;
};

#endif // PATROLLINGDRONE_H
