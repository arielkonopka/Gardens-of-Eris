#ifndef PLAYER_H
#define PLAYER_H

#include <movableElements.h>
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"

class player : public movableElements
{
public:
    virtual videoElement::videoElementDef* getVideoElementDef();
    static videoElement::videoElementDef* vd;
    bool mechanics(bool collected);
    bool canPush();
    bool canCollect();
    bool canInteract();
    bool canBeKilled();
    int getType();
    bool getActive();
    int getAnimPh();
    player(chamber *board,gCollect *garbage);
    virtual ~player();
 //   bool isDying();
    //     virtual bool tick(bool collected);
protected:

private:
    int animPh;
    int moved;
    int shot;
    int used;
    int interacted;
};

#endif // PLAYER_H
