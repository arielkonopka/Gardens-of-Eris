#include "soundManager.h"

soundManager *soundManager::instance = nullptr;
soundManager::soundManager()
{
    // ctor
}

soundManager::~soundManager()
{
    // dtor
}
soundManager *soundManager::getInstance()
{
    if (soundManager::instance == nullptr)
        soundManager::instance = new soundManager();
    return soundManager::instance;
}

int soundManager::registerSound(int instanceId, int typeId, int subtypeId, std::string eventType, std::string event)
{
    return -1;
};
bool soundManager::playSound(int sndId, int chmbrId)
{
    return false;
};
int soundManager::getSndStatus(int sndId)
{
    return -1;
};
bool soundManager::stopSnd(int sndId)
{
    return false;
};
bool soundManager::pauseSnd(int sndId)
{
    return false;
}
bool soundManager::resumeSnd(int sndId)
{
    return false;
}
void soundManager::deregisterSnd(int sndId)
{
}


bool soundManager::checkSmpExists(int instanceId, int typeId, int subtypeId, std::string eventType, std::string event)
{
    return this->registeredSounds[instanceId][typeId][subtypeId][eventType][event].isRegistered;
}

