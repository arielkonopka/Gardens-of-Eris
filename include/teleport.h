#ifndef TELEPORT_H
#define TELEPORT_H

#include "commons.h"
#include <nonSteppable.h>
#include "videoElementDef.h"

class teleport : public nonSteppable
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    teleport(chamber* board, gCollect* garbage);
    virtual ~teleport();
    virtual bool interact(bElem *who);
    virtual bool isInteractive();
    virtual bool teleportIt(bElem *who,coords teleportPosition);
protected:

private:
};

#endif // TELEPORT_H
