#include "soundManager.h"

soundManager* soundManager::instance=NULL;
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
    if (soundManager::instance==NULL)
        soundManager::instance=new soundManager();
    return soundManager::instance;
}
