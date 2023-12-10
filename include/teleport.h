#ifndef TELEPORT_H
#define TELEPORT_H

#include "commons.h"
#include "videoElementDef.h"
#include "randomLevelGenerator.h"
#include "bElem.h"
class teleport : public bElem
{
public:
    int getType() const;
    teleport();
    explicit teleport(std::shared_ptr<chamber> board);
    explicit teleport(std::shared_ptr<chamber> board,int newSubtype);
    virtual ~teleport()=default;
    bool interact(std::shared_ptr<bElem> who) final;
    virtual bool teleportIt(std::shared_ptr<bElem> who);
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual bool createConnectionsWithinSUbtype();
    bool additionalProvisioning(int value,std::shared_ptr<teleport> t);
    bool mechanics() final;
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
    bool stepOnAction(bool step,std::shared_ptr<bElem> who);
private:
    static std::vector<std::weak_ptr<teleport>> allTeleporters;
    bool removeFromAllTeleporters();
    bool connectionsMade=false;
    bool checked=false;
    std::shared_ptr<teleport> theOtherEnd;
};

#endif // TELEPORT_H
