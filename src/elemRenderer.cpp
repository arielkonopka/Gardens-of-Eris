//
// Created by c on 23.09.24.
//
#include "elemRenderer.h"

elemRenderer::~elemRenderer() {
    if(this->gameField)
        al_destroy_bitmap(this->gameField);
}

elemRenderer::elemRenderer() {
    coords size=videoManager::getInstance().getGameFieldSize();
    upLeft=coords(configManager::getInstance()->getConfig()->tileWidth*2,configManager::getInstance()->getConfig()->tileHeight*2);
    size=size+(upLeft*2);
    this->gameField= al_create_bitmap(size.x,size.y);
}

bool elemRenderer::renderElement(std::shared_ptr <bElem> element, coords ULpoint) {
    return false;
}

