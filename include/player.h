#ifndef PLAYER_H
#define PLAYER_H

#include <movableElements.h>
#include "commons.h"
#include "objectTypes.h"
#include "videoElementDef.h"
#include "plainGun.h"
#include "soundManager.h"


class player : public killableElements,public movableElements,public nonSteppable,public mechanical
{
public:
    player();
    explicit player(std::shared_ptr<chamber> board);
    ~player() final=default;
    static unsigned int countVisitedPlayers();
    virtual videoElement::videoElementDef* getVideoElementDef();
    static videoElement::videoElementDef* vd;
    static std::shared_ptr<bElem> getActivePlayer();
    bool stepOnElement(std::shared_ptr<bElem> step);
    bool mechanics() final;
    bool interact(std::shared_ptr<bElem> who) final;
    bool getVisited();
    int getType() const;
    int getAnimPh() final;
    bool shootGun();
    oState disposeElement() final;
    bool additionalProvisioning() final;
    bool additionalProvisioning(int subtype,int typeId) ;
    const float getViewRadius() { return this->vRadius; };
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<player> sbe);

private:
    float vRadius=5.5;
    static std::shared_ptr<bElem> activePlayer;
    static std::vector<std::shared_ptr<bElem>> allPlayers;
    static std::vector<std::shared_ptr<bElem>> visitedPlayers;
    bool visited=false;
    int animPh=0;
//    int moved=0;
//    int shot=0;
    bool activated=false;
//    int used=0;
//    int interacted=0;
    bool provisioned=false;
    int mysound;
};

#endif // PLAYER_H
