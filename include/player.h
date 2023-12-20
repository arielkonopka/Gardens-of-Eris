#ifndef PLAYER_H
#define PLAYER_H
#include "commons.h"
#include "inputManager.h"
#include "objectTypes.h"
#include "videoElementDef.h"
#include "plainGun.h"
#include "soundManager.h"
#include "viewPoint.h"

class player : public killableElements
{
public:
    player();
    explicit player(std::shared_ptr<chamber> board);
    ~player() final=default;
    static unsigned int countVisitedPlayers() ;

    static std::shared_ptr<bElem> getActivePlayer();
    bool stepOnElement(std::shared_ptr<bElem> step);
    bool mechanics() final;
    bool interact(std::shared_ptr<bElem> who) final;
    int getType() const;
    int getAnimPh() const;
    bool shootGun();
    oState disposeElement() final;
    bool additionalProvisioning() final;
    bool additionalProvisioning(int subtype,int typeId) ;
    float getViewRadius() const;
    bool additionalProvisioning(int subtype,std::shared_ptr<player> sbe) ;

private:
    float vRadius=2;
    static std::shared_ptr<bElem> activePlayer;
    static std::vector<std::shared_ptr<bElem>> visitedPlayers;
    int animPh=0;
    bool activated=false;
    bool provisioned=false;
    int mysound;
};

#endif // PLAYER_H
