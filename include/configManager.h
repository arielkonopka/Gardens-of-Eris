#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <stddef.h>
#include <iostream>
#include <memory>
#include "videoElementDef.h"
#include <map>
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

using sampleData= struct sampleData
{
    bool configured=false;
    std::string fname="";
    std::string name="UNKNOWN";
    std::string description="This sampleData is empty";
    bool stacking=false;
    bool allowMulti=false;
    int modeOfAction=-1; /* 0 - normal - play and forget, 1 - looped until stopped, or lost view, 2 - backwards, 3 - pingloop -forward and backward until stopped, or lost view */
} ;
using musicData=struct _musicData
{
    bool configured=false;
    std::string filename;
    std::string name;
    int chamberId=-1;
    coords3d position={0,0,0};
};



typedef struct gameConfig
{
    std::string FontFile,spriteFile,splashScr;
    int sndFifoSize=0;
    int soundDistance=1005;
    int tileWidth,tileHeight;

    int spacing; //We use spacing in the sprite png file, just for our comfort
    std::vector<musicData> music;
    std::vector<std::string> sylables;
    std::vector<coords> gDying;
    std::vector<coords> gDestroying;
    std::vector<coords> gFadingOut;
    std::vector<coords> gTeleporting;
    std::vector<spriteData> sprites;
    coords bluredElement=NOCOORDS;
    coords bluredElement25=NOCOORDS;
    coords bluredElement50=NOCOORDS;
    coords bluredElement75=NOCOORDS;

    //samples[ElementType][ElementSubtype][EventType][Event]
    std::map<int,std::map<int,std::map<std::string,std::map<std::string,sampleData>>>> samples;

} gameConfig;



class configManager: public std::enable_shared_from_this<configManager>
{
public:
    static std::shared_ptr<configManager> getInstance();
    std::shared_ptr<gameConfig> getConfig();
    void configReload();
    configManager();

private:
    std::shared_ptr<gameConfig> gConfObj;
    int tileWidth;
    int tileHeight;
    rapidjson::Document skinDefJson;
    static std::shared_ptr<configManager> instance;
};

#endif // CONFIGMANAGER_H
