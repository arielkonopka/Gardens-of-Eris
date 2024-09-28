//
// Created by c on 23.09.24.
//

#ifndef GARDENOFERIS_HUDMANAGER_H
#define GARDENOFERIS_HUDMANAGER_H
#include "commons.h"
#include "bElem.h"
#include "allegro5/allegro.h"
#include "videoManager.h"
#include "configManager.h"
#include "viewPoint.h"
class hudManager
{
public:
    static hudManager& getInstance();
    hudManager();
    ~hudManager();
    bool renderHUDforElement(std::shared_ptr<bElem> element,coords upLeft);
    bool renderHUD();


private:
    static std::once_flag initFlag;

    coords upperCorner;
    ALLEGRO_BITMAP *HUDGamefield= nullptr;
    ALLEGRO_BITMAP *HUDGlobal= nullptr;
};
#endif //GARDENOFERIS_HUDMANAGER_H
