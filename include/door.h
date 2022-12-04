#ifndef DOOR_H
#define DOOR_H

#include <bElem.h>
#include "videoElementDef.h"
#include "commons.h"

class door : public bElem
{
public:
    static videoElement::videoElementDef* vd;
    virtual videoElement::videoElementDef* getVideoElementDef();
    virtual int getType();
    door(chamber* board);
    door(chamber* board,int subtype);
    virtual bool interact(bElem *who);
    virtual bool isSteppable();
    virtual bool isOpen();
    virtual void stomp(bElem* who);
    virtual void unstomp();
    virtual ~door();
    virtual bool canBeKilled();
    virtual bool canBeDestroyed();
private:
    int interacted=-1;
    bool open=false;
    bool locked=true;

};

#endif // DOOR_H
