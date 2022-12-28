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
    videoElement::videoElementDef* getVideoElementDef() final;
    int getType() final;
    teleport();
    explicit teleport(std::shared_ptr<chamber> board);
    explicit teleport(std::shared_ptr<chamber> board,int newSubtype);
    ~teleport() final;
    bool interact(std::shared_ptr<bElem> who) final;
    virtual bool teleportIt(std::shared_ptr<bElem> who);
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    bool canBeKilled() final;
    bool canBeDestroyed() final;
    virtual bool createConnectionsWithinSUbtype();
    bool isSteppable() final;
    void stomp(std::shared_ptr<bElem> who) final;
    void unstomp() final;
    bool mechanics() final;

private:
    static std::vector<teleport*> allTeleporters;
    bool removeFromAllTeleporters();
    bool connectionsMade;
    teleport *theOtherEnd;
};

#endif // TELEPORT_H
