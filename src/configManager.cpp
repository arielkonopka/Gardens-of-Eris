#include "configManager.h"


configManager* configManager::instance=NULL;

configManager::configManager()
{
    this->configReload();
}

configManager::~configManager()
{
    //dtor
}
configManager* configManager::getInstance()
{
    if(configManager::instance==NULL)
    {
        configManager::instance=new configManager();
    }
    return configManager::instance;
}
void configManager::configReload()
{
    FILE* fp = fopen("data/skins.json", "rb"); // non-Windows use "r"
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    this->skinDefJson.ParseStream(is);
    fclose(fp);
}
rapidjson::Document *configManager::getConfig()
{
    return &this->skinDefJson;
}
