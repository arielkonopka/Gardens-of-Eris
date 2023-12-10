#ifndef DOOR_H
#define DOOR_H

#include <bElem.h>
#include "videoElementDef.h"
#include "wall.h"
#include "commons.h"

class door : public bElem
{
public:

    virtual int getType() const;
    door();
    explicit door(std::shared_ptr<chamber>  board);
    virtual bool stepOnAction(bool step,std::shared_ptr<bElem> who);
    bool destroy();
    virtual bool interact(std::shared_ptr<bElem> who);

    virtual ~door()=default;
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<door> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
private:
    void _alignWithOpen();
//   int interacted=-1;

};

#endif // DOOR_H
