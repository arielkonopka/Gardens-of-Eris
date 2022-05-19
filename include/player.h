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
    bool interact(bElem *who);
    bool canPush();
    bool canCollect();
    bool canInteract();
    bool getVisited();
    int getType();
    void setActive(bool act);
    bool isActive();
    int getAnimPh();
    player(chamber *board);
    virtual ~player();
    virtual bool shootGun();
    virtual oState disposeElement();
//   bool isDying();
    //     virtual bool tick(bool collected);
protected:
//    std::vector<bElem*> weapons;
//    std::vector<bElem*> usables;

private:
    static std::vector<player*> allPlayers;
    static std::vector<player*> visitedPlayers;
    bool visited=false;
    int animPh;
    int moved;
    int shot;
    bool activated;
    int used;
    int interacted;
};

#endif // PLAYER_H
