#ifndef PATROLLINGDRONE_H
#define PATROLLINGDRONE_H

#include <killableElements.h>
#include "commons.h"
#include "videoElementDef.h"

class patrollingDrone : public killableElements, public nonSteppable, public movableElements
{
public:
    virtual videoElement::videoElementDef* getVideoElementDef();
    static videoElement::videoElementDef* vd;
    patrollingDrone();
    explicit patrollingDrone(std::shared_ptr<chamber> board);
    virtual ~patrollingDrone();
    virtual bool interact(std::shared_ptr<bElem> who);
    int getType();

    virtual bool additionalProvisioning(int subtype,std::shared_ptr<patrollingDrone> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
private:
    bool brained=false;
    std::shared_ptr<bElem> brainModule;
};

#endif // PATROLLINGDRONE_H
