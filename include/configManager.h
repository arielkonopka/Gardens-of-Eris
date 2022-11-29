#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <stddef.h>
#include <iostream>
#include "videoElementDef.h"

#include "inputManager.h"
#include "objectTypes.h"
#include "commons.h"
// Singleton pattern, we will not have multiple config files, therefore we do not need multiple instances of config manager
// this class will read the config json file, and expose gathered information to whatever class needing it


typedef struct spriteData
{
    int eType;
    std::string name;
    std::vector< std::vector<std::vector<coords>> > animDef;
    std::vector<coords > dying;
    std::vector<coords > destroying;
    std::vector<coords> fadingOut;
    std::vector<coords> teleporting;
} spriteData;


typedef struct gameConfig
{
    std::string FontFile,spriteFile,splashScr;
    int tileWidth,tileHeight;
    int spacing; //We use spacing in the sprite png file, just for our comfort
    std::vector<std::string> sylables;
    std::vector<coords> gDying;
    std::vector<coords> gDestroying;
    std::vector<coords> gFadingOut;
    std::vector<coords> gTeleporting;
    std::vector<spriteData> sprites;
} gameConfig;



class configManager
{
    public:
       static configManager* getInstance();
        ~configManager();
        gameConfig* getConfig();
        void configReload();

    protected:

    private:
        gameConfig* gConfObj;
        int tileWidth;
        int tileHeight;
        rapidjson::Document skinDefJson;
        configManager();
        static configManager* instance;
};

#endif // CONFIGMANAGER_H
