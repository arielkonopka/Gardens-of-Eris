#ifndef PLAYER_H
#define PLAYER_H

#include <movableElements.h>
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"
#include "plainGun.h"

class player : public killableElements
{
public:
    virtual videoElement::videoElementDef* getVideoElementDef();
    static videoElement::videoElementDef* vd;
    bool mechanics(bool collected);
    bool canPush();
    bool canCollect();
    bool canInteract();
    int getType();
    bool getActive();
    int getAnimPh();
    player(chamber *board,gCollect *garbage);
    virtual ~player();
    virtual bool shootGun();

 //   bool isDying();
    //     virtual bool tick(bool collected);
protected:
//    std::vector<bElem*> weapons;
//    std::vector<bElem*> usables;

private:
    int animPh;
    int moved;
    int shot;
    int used;
    int interacted;
};

#endif // PLAYER_H
