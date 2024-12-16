//
// Created by c on 23.09.24.
//

#ifndef GARDENOFERIS_ELEMRENDERER_H
#define GARDENOFERIS_ELEMRENDERER_H
#include <memory>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>


#include "bElem.h"
#include "videoManager.h"
#include "commons.h"
class elemRenderer
{
    elemRenderer();
    ~elemRenderer();
    bool renderElement(std::shared_ptr<bElem> element,coords ULpoint);
    bool getCPURendering();
    void setCPURendering(bool value);
private:
    bool CPURendering=true;
    ALLEGRO_BITMAP *gameField=nullptr;
    coords upLeft;
};
#endif //GARDENOFERIS_ELEMRENDERER_H
