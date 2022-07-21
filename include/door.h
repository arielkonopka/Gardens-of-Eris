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
    door(chamber* board,int x, int y);
    door(chamber* board,int subtype);
    door(chamber* board,int subtype,int x, int y);
    virtual bool interact(bElem *who);
    virtual bool isSteppable();
    virtual bool isOpen();
    virtual ~door();
    virtual bool isInteractive();
    virtual bool canBeKilled();
    virtual bool canBeDestroyed();
protected:
    bool open;
    bool locked;

private:
    void initMe();
    int interacted;

};

#endif // DOOR_H
