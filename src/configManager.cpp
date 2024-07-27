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
#include "configManager.h"

std::shared_ptr<configManager> configManager::instance = nullptr;
std::once_flag configManager::_onceFlag;

configManager::configManager() : std::enable_shared_from_this<configManager>()
{
    this->gConfObj = std::make_shared<gameConfig>();
}

std::shared_ptr<configManager> configManager::getInstance()
{
    std::call_once(configManager::_onceFlag,[]()
    {
        configManager::instance = std::make_shared<configManager>();
        configManager::instance->configReload();
    });
    return configManager::instance;
}
void configManager::configReload()
{
    FILE *fp = fopen(confFname1, "rb"); // non-Windows use "r"
    if(fp==nullptr)
        fp = fopen(confFname2, "rb");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    this->skinDefJson.ParseStream(is);
    fclose(fp);
    this->gConfObj->gDestroying.clear();
    this->gConfObj->gDying.clear();
    this->gConfObj->gFadingOut.clear();
    this->gConfObj->gFadingIn.clear();
    this->gConfObj->gTeleporting.clear();
    this->gConfObj->sylables.clear();
    this->gConfObj->sprites.clear();
    if (this->skinDefJson.HasMember("Blur"))
    {
        rapidjson::Value &blur=this->skinDefJson["Blur"];
        for (unsigned int c = 0; c < blur.Size(); c++)
        {
            int x = blur[c][0].GetInt();
            int y = blur[c][1].GetInt();
            this->gConfObj->bluredElement.push_back((coords)
            {
                x, y
            });
        }
    }
    if(this->skinDefJson.HasMember("BaseDir"))
        this->gConfObj->baseDir=this->skinDefJson["BaseDir"].GetString();
    else
        this->gConfObj->baseDir="./";
    this->gConfObj->soundDistance=this->skinDefJson["MaxSoundDistance"].GetInt();
    this->gConfObj->sndFifoSize=this->skinDefJson["SndFifoSize"].GetInt();
    this->gConfObj->FontFile = this->skinDefJson["FontFile"].GetString();
    this->gConfObj->spriteFile = this->gConfObj->baseDir+this->skinDefJson["SpriteFile"].GetString();
    this->gConfObj->splashScr = this->skinDefJson["splash screen"].GetString();
    this->gConfObj->tileWidth = this->skinDefJson["width"].GetInt();
    this->gConfObj->tileHeight = this->skinDefJson["height"].GetInt();
    this->gConfObj->spacing = this->skinDefJson["spacing"].GetInt();
    rapidjson::Value &sprlist = this->skinDefJson["SpriteData"];
    rapidjson::Value &music=this->skinDefJson["Music"];
    for(unsigned int c=0; c<music.Size(); c++)
    {
        musicData md;
        md.filename=this->gConfObj->baseDir+music[c]["Filename"].GetString();
        md.name=music[c]["Name"].GetString();
        if(music[c].HasMember("gain"))
            md.gain=music[c]["gain"].GetFloat();
        if(music[c].HasMember("chamberId"))
            md.chamberId=music[c]["chamberId"].GetInt();
        if(music[c].HasMember("Position"))
        {
            md.position.x=music[c]["Position"][0].GetFloat();
            md.position.y=music[c]["Position"][1].GetFloat();
            md.position.z=music[c]["Position"][2].GetFloat();
        }
        this->gConfObj->music.push_back(md);
    }

    auto convTabsInt=[](rapidjson::Value &tab,std::vector<coords>& v)
    {
        for(unsigned int c=0; c<tab.Size(); c++)
        {
            coords p= {tab[c][0].GetInt(),tab[c][1].GetInt()};
            v.push_back(p);
        }
    };
    auto convTabsDoc=[&](rapidjson::Document &tab, auto& name,std::vector<coords>& v)
    {
        if(tab.HasMember(name))
            convTabsInt(tab[name],v);
    };
    auto convTabsValue=[&](rapidjson::Value &tab, auto& name,std::vector<coords>& v)
    {
        if(tab.HasMember(name))
            convTabsInt(tab[name],v);
    };
    /// add animation phases for various general things, like dying animation, destroying animation and so on...
    convTabsDoc(this->skinDefJson,"Dying",this->gConfObj->gDying);
    convTabsDoc(this->skinDefJson,"Teleporting",this->gConfObj->gTeleporting);
    convTabsDoc(this->skinDefJson,"Destroying",this->gConfObj->gDestroying);
    convTabsDoc(this->skinDefJson,"Fading Out",this->gConfObj->gFadingOut);
    convTabsDoc(this->skinDefJson,"Fading In",this->gConfObj->gFadingIn);
    ///
    for (unsigned int c = 0; c < sprlist.Size(); c++)
    {
        spriteData sdata;
        sdata.name = sprlist[c]["Name"].GetString();
        sdata.eType = sprlist[c]["Type"].GetInt();
        if(sprlist[c].HasMember("Attributes"))
        {
            for(unsigned int i =0; i<sprlist[c]["Attributes"].Size(); i++)
            {
                /*
                Here we read the player's attributes
                */
                attributeData ad;
                ad.subType=(sprlist[c]["Attributes"][i].HasMember("Subtype"))?sprlist[c]["Attributes"][i]["Subtype"].GetInt():-1;
                ad.killable= (sprlist[c]["Attributes"][i].HasMember("Killable")) &&
                             sprlist[c]["Attributes"][i]["Killable"].GetBool();
                ad.destroyable= (sprlist[c]["Attributes"][i].HasMember("Destroyable")) &&
                                sprlist[c]["Attributes"][i]["Destroyable"].GetBool();
                ad.steppable= (sprlist[c]["Attributes"][i].HasMember("Steppable")) &&
                              sprlist[c]["Attributes"][i]["Steppable"].GetBool();
                ad.isMovable= (sprlist[c]["Attributes"][i].HasMember("isMovable")) &&
                              sprlist[c]["Attributes"][i]["isMovable"].GetBool();
                ad.isInteractive= (sprlist[c]["Attributes"][i].HasMember("isInteractive")) &&
                                  sprlist[c]["Attributes"][i]["isInteractive"].GetBool();
                ad.isCollectible= (sprlist[c]["Attributes"][i].HasMember("isCollectible")) &&
                                  sprlist[c]["Attributes"][i]["isCollectible"].GetBool();
                ad.canPush= (sprlist[c]["Attributes"][i].HasMember("canPush")) &&
                            sprlist[c]["Attributes"][i]["canPush"].GetBool();
                ad.canBePushed= (sprlist[c]["Attributes"][i].HasMember("canBePushed")) &&
                                sprlist[c]["Attributes"][i]["canBePushed"].GetBool();
                ad.isWeapon= (sprlist[c]["Attributes"][i].HasMember("isWeapon")) &&
                             sprlist[c]["Attributes"][i]["isWeapon"].GetBool();
                ad.isOpen= (sprlist[c]["Attributes"][i].HasMember("isOpen")) &&
                           sprlist[c]["Attributes"][i]["isOpen"].GetBool();
                ad.isLocked= (sprlist[c]["Attributes"][i].HasMember("isLocked")) &&
                             sprlist[c]["Attributes"][i]["isLocked"].GetBool();
                ad.energy=(sprlist[c]["Attributes"][i].HasMember("energy"))?sprlist[c]["Attributes"][i]["energy"].GetInt():1;
                ad.maxEnergy=(sprlist[c]["Attributes"][i].HasMember("maxEnergy"))?sprlist[c]["Attributes"][i]["maxEnergy"].GetInt():1;
                ad.ammo=(sprlist[c]["Attributes"][i].HasMember("ammo"))?sprlist[c]["Attributes"][i]["ammo"].GetInt():0;
                ad.maxAmmo=(sprlist[c]["Attributes"][i].HasMember("maxAmmo"))?sprlist[c]["Attributes"][i]["maxAmmo"].GetInt():0;
                sdata.attributes.push_back(ad);
            }
        }
        if (sprlist[c].HasMember("Samples"))
        {
            for (unsigned int i = 0; i < sprlist[c]["Samples"].Size(); i++)
            {
                int st = sprlist[c]["Samples"][i]["SubType"].GetInt();
                for (unsigned int i2 = 0; i2 < sprlist[c]["Samples"][i]["stEvents"].Size(); i2++)
                {
                    std::string eventType = sprlist[c]["Samples"][i]["stEvents"][i2]["EventType"].GetString();
                    for (unsigned int i3 = 0; i3 < sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"].Size(); i3++)
                    {
                        std::string evname = sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3]["Event"].GetString();
                        sampleData sd;
                        //sprlist[c].HasMember("Destroying")
                        sd.fname =this->gConfObj->baseDir+ sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3]["fname"].GetString();
                        sd.name = sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3]["name"].GetString();
                        if(sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3].HasMember("description"))
                            sd.description = sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3]["description"].GetString();
                        if(sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3].HasMember("allowMulti"))
                            sd.allowMulti = sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3]["allowMulti"].GetBool();
                        if(sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3].HasMember("stacking"))
                            sd.stacking = sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3]["stacking"].GetBool();
                        sd.modeOfAction = sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3]["modeOfAction"].GetInt();
                        if(sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3].HasMember("gain"))
                            sd.gain = sprlist[c]["Samples"][i]["stEvents"][i2]["eventData"][i3]["gain"].GetFloat();
                        sd.configured=true;
                        this->gConfObj->samples[sdata.eType][st][eventType][evname] = sd;
                    }
                }
            }
        }
        // Adding custom to the element type, various things, if they exist, like dying, teleporting, fading in or out, etc.
        convTabsValue(sprlist[c],"Dying",sdata.dying);
        convTabsValue(sprlist[c],"Destroying",sdata.destroying);
        convTabsValue(sprlist[c],"Teleporting",sdata.teleporting);
        convTabsValue(sprlist[c],"Fading Out",sdata.fadingOut);
        convTabsValue(sprlist[c],"Fading In",sdata.fadingIn);
        // Make it read individual dying and teleporting
        for (unsigned int c1 = 0; c1 < sprlist[c]["AnimDef"].Size(); c1++)
        {
            std::vector<std::vector<coords>> dirs;
            for (unsigned int c2 = 0; c2 < sprlist[c]["AnimDef"][c1].Size(); c2++)
            {
                std::vector<coords> phs;
                for (unsigned int c3 = 0; c3 < sprlist[c]["AnimDef"][c1][c2].Size(); c3++)
                {
                    convTabsInt(sprlist[c]["AnimDef"][c1][c2],phs);
                }
                dirs.push_back(phs);
            }
            sdata.animDef.push_back(dirs);
        }
        this->gConfObj->sprites.push_back(sdata);
    }
}
std::shared_ptr<gameConfig> configManager::getConfig()
{
    return this->gConfObj;
}
