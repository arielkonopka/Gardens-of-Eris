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
    teleport(chamber* board);
    virtual ~teleport();
    virtual bool interact(bElem *who);
    virtual bool isInteractive();
    virtual bool teleportIt(bElem *who);
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual bool canBeKilled();
    virtual bool canBeDestroyed();
protected:
private:
    static std::vector<teleport*> teleporters;
    bool removeFromTeleports();
    void purgeFromTeleporters();
    bool addToTeleports();
    teleport *theOtherEnd;
};

#endif // TELEPORT_H
