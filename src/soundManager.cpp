#include "soundManager.h"

soundManager* soundManager::instance=nullptr;
soundManager::soundManager()
{
    //ctor
}

soundManager::~soundManager()
{
    //dtor
}
soundManager* soundManager::getInstance()
{
    if (soundManager::instance==nullptr)
        soundManager::instance=new soundManager();
    return soundManager::instance;
}
