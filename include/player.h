#ifndef PLAYER_H
#define PLAYER_H

#include <movableElements.h>
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"
#include "plainGun.h"

class player : public killableElements,public movableElements,public nonSteppable,public mechanical
{
public:
    player(chamber *board);
    virtual ~player();
    static unsigned int countVisitedPlayers();
    virtual videoElement::videoElementDef* getVideoElementDef();
    static videoElement::videoElementDef* vd;
    static player* getActivePlayer();
    bool mechanics();
    bool interact(bElem *who);
    bool canPush();
    bool getVisited();
    int getType();
    void setActive(bool act);
    bool isActive();
    int getAnimPh();
    virtual bool shootGun();
    virtual oState disposeElement();

protected:


private:
    static player* activePlayer;
    static std::vector<player*> allPlayers;
    static std::vector<player*> visitedPlayers;
    bool visited=false;
    int animPh=0;
    int moved=0;
    int shot=0;
    bool activated=false;
    int used=0;
    int interacted=0;
};

#endif // PLAYER_H
