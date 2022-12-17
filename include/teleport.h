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
    teleport();
    teleport(std::shared_ptr<chamber> board);
    teleport(std::shared_ptr<chamber> board,int newSubtype);
    virtual ~teleport();
    virtual bool interact(std::shared_ptr<bElem> who);
    virtual bool teleportIt(std::shared_ptr<bElem> who);
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual bool canBeKilled();
    virtual bool canBeDestroyed();
    virtual bool createConnectionsWithinSUbtype();
    virtual bool isSteppable();
    virtual void stomp(std::shared_ptr<bElem> who);
    virtual void unstomp();
    virtual bool mechanics();

private:
    static std::vector<teleport*> allTeleporters;
    bool removeFromAllTeleporters();
    bool connectionsMade;
    teleport *theOtherEnd;
};

#endif // TELEPORT_H
