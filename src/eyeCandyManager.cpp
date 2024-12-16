#include "eyeCandyManager.h"

eyecandyManager& eyecandyManager::getInstance()
{
    static eyecandyManager instance;
    return instance;
}

eyecandyManager::eyecandyManager()
        : pixelMap(nullptr)
{
    pixelMap=al_create_bitmap(videoManager::getInstance().getGameFieldSize().x,videoManager::getInstance().getGameFieldSize().y);
}

eyecandyManager::~eyecandyManager()
{
    if (pixelMap) {
        al_destroy_bitmap(pixelMap);
        pixelMap = nullptr;
    }


}
