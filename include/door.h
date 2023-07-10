#ifndef DOOR_H
#define DOOR_H

#include <audibleElement.h>
#include "videoElementDef.h"
#include "commons.h"

class door : public audibleElement
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    door();
    explicit door(std::shared_ptr<chamber>  board);

    virtual bool interact(std::shared_ptr<bElem> who);

    virtual ~door();
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<door> sbe);
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
private:
    int interacted=-1;
    bool open=false;
    bool locked=true;

};

#endif // DOOR_H
