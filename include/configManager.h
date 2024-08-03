/*
 * Copyright (c) 2023, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <istream>
// *** END ***
#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <cstddef>
#include <iostream>
#include <memory>
#include "videoElementDef.h"
#include <map>
#include "inputManager.h"
#include "objectTypes.h"
#include "commons.h"

// Singleton pattern, we will not have multiple config files, therefore we do not need multiple instances of config manager
// this class will read the config json file, and expose gathered information to whatever class needing it
using attributeData=struct ErisAttData
{
    int subType=-1;
    bool killable=false;
    bool destroyable=false;
    bool steppable=false;
    bool isMovable=false;
    bool isInteractive=false;
    bool isCollectible=false;
    bool canPush=false;
    bool canBePushed=false;
    bool canCollect=false;
    bool isWeapon=false;
    bool isOpen=false;
    bool isLocked=false;
    int energy=100;
    int maxEnergy=100;
    int ammo=0;
    int maxAmmo=0;


};

typedef struct spriteData
{
    int eType;
    std::string name;
    std::vector< std::vector<std::vector<coords>> > animDef;
    std::vector<coords > dying;
    std::vector<coords > destroying;
    std::vector<coords> fadingOut;
    std::vector<coords> fadingIn;
    std::vector<coords> teleporting;
    std::vector<attributeData> attributes;
} spriteData;

using sampleData= struct sampleData
{
    bool configured=false;
    std::string fname;
    std::string name="UNKNOWN";
    std::string description="This sampleData is empty";
    bool stacking=false;
    bool allowMulti=false;
    float gain=1.0;
    int modeOfAction=-1; /* 0 - normal - play and forget, 1 - looped until stopped, or lost view, 2 - backwards, 3 - pingloop -forward and backward until stopped, or lost view */
} ;
using musicData=struct ErisMusicData
{
    bool configured=false;
    std::string filename;
    std::string name;
    int chamberId=-1;
    float gain=1.0;
    coords3d position={0,0,0};
};



typedef struct gameConfig
{
    std::string FontFile,spriteFile,splashScr;
    int sndFifoSize=0;
    int soundDistance=250;
    int tileWidth,tileHeight;
    std::string baseDir;
    int spacing; //We use spacing in the sprite png file, just for our comfort
    std::vector<musicData> music;
    std::vector<std::string> sylables;
    std::vector<coords> gDying;
    std::vector<coords> gDestroying;
    std::vector<coords> gFadingOut;
    std::vector<coords> gFadingIn;
    std::vector<coords> gTeleporting;
    std::vector<spriteData> sprites;
    std::vector<coords> bluredElement;


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
    static std::once_flag _onceFlag;
    std::shared_ptr<gameConfig> gConfObj;
    int tileWidth=-1;
    int tileHeight=-1;
    rapidjson::Document skinDefJson;
    static std::shared_ptr<configManager> instance;
};

#endif // CONFIGMANAGER_H
