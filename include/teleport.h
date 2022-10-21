#ifndef TELEPORT_H
#define TELEPORT_H

#include "commons.h"
#include <nonSteppable.h>
#include "videoElementDef.h"
#include "randomLevelGenerator.h"
class teleport : public nonSteppable
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    teleport(chamber* board);
    teleport(chamber* board,int newSubtype);
    virtual ~teleport();
    virtual bool interact(bElem *who);
    virtual bool isInteractive();
    virtual bool teleportIt(bElem *who);
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual bool canBeKilled();
    virtual bool canBeDestroyed();
    virtual bool createConnectionsWithinSUbtype();
protected:
private:

    static std::vector<teleport*> allTeleporters;
    bool removeFromAllTeleporters();
    bool connectionsMade;
    teleport *theOtherEnd;
};

#endif // TELEPORT_H
