#include "configManager.h"

std::shared_ptr<configManager> configManager::instance=nullptr;

configManager::configManager(): std::enable_shared_from_this<configManager>()
{
    this->gConfObj=std::make_shared<gameConfig>();
    this->configReload();
}

std::shared_ptr<configManager> configManager::getInstance()
{
    configManager cm=configManager();
    if(configManager::instance==nullptr)
    {
        configManager::instance=std::make_shared<configManager>();
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
    this->gConfObj->gDestroying.clear();
    this->gConfObj->gDying.clear();
    this->gConfObj->gFadingOut.clear();
    this->gConfObj->gTeleporting.clear();
    this->gConfObj->sylables.clear();
    this->gConfObj->sprites.clear();
    this->gConfObj->FontFile=this->skinDefJson["FontFile"].GetString();
    this->gConfObj->spriteFile=this->skinDefJson["SpriteFile"].GetString();
    this->gConfObj->splashScr=this->skinDefJson["splash screen"].GetString();
    this->gConfObj->tileWidth=this->skinDefJson["width"].GetInt();
    this->gConfObj->tileHeight=this->skinDefJson["height"].GetInt();
    this->gConfObj->spacing=this->skinDefJson["spacing"].GetInt();
    rapidjson::Value& dying=this->skinDefJson["Dying"];
    rapidjson::Value& teleporting=this->skinDefJson["Teleporting"];
    rapidjson::Value& destroying=this->skinDefJson["Destroying"];
    rapidjson::Value& fadingOut=this->skinDefJson["Fading"];
    rapidjson::Value& sprlist=this->skinDefJson["SpriteData"];
    for(unsigned int c=0; c<dying.Size(); c++)
    {
        int x=dying[c][0].GetInt();
        int y=dying[c][1].GetInt();
        this->gConfObj->gDying.push_back((coords)
        {
            x,y
        });
    }
    for(unsigned int c=0; c<teleporting.Size(); c++)
    {
        int x=teleporting[c][0].GetInt();
        int y=teleporting[c][1].GetInt();
        this->gConfObj->gTeleporting.push_back((coords)
        {
            x,y
        });
    }
    for(unsigned int c=0; c<destroying.Size(); c++)
    {
        int x=destroying[c][0].GetInt();
        int y=destroying[c][1].GetInt();
        this->gConfObj->gDestroying.push_back((coords)
        {
            x,y
        });
    }
    for(unsigned int c=0; c<fadingOut.Size(); c++)
    {
        int x=fadingOut[c][0].GetInt();
        int y=fadingOut[c][1].GetInt();
        this->gConfObj->gFadingOut.push_back((coords)
        {
            x,y
        });
    }

    for(unsigned int c=0; c<sprlist.Size(); c++)
    {
        spriteData sdata;
        sdata.name=sprlist[c]["Name"].GetString();
        sdata.eType=sprlist[c]["Type"].GetInt();
        if(sprlist[c].HasMember("Dying"))
        {
            for(unsigned int c1=0;c1<sprlist[c]["Dying"].Size();c1++)
            sdata.dying.push_back((coords){sprlist[c]["Dying"][c1][0].GetInt(),sprlist[c]["Dying"][c1][1].GetInt()});
        }
        if(sprlist[c].HasMember("Destroying"))
        {
            for(unsigned int c1=0;c1<sprlist[c]["Destroying"].Size();c1++)
            sdata.destroying.push_back((coords){sprlist[c]["Destroying"][c1][0].GetInt(),sprlist[c]["Destroying"][c1][1].GetInt()});
        }
        if(sprlist[c].HasMember("Teleporting"))
        {
            for(unsigned int c1=0;c1<sprlist[c]["Teleporting"].Size();c1++)
            sdata.teleporting.push_back((coords){sprlist[c]["Teleporting"][c1][0].GetInt(),sprlist[c]["Teleporting"][c1][1].GetInt()});
        }
        if(sprlist[c].HasMember("Fading"))
        {
            for(unsigned int c1=0;c1<sprlist[c]["Fading"].Size();c1++)
            sdata.fadingOut.push_back((coords){sprlist[c]["Fading"][c1][0].GetInt(),sprlist[c]["Fading"][c1][1].GetInt()});
        }


//Make it read individual dying and teleporting
        for(unsigned int c1=0; c1<sprlist[c]["AnimDef"].Size(); c1++)
        {
            std::vector<std::vector<coords>> dirs;
            for(unsigned int c2=0; c2<sprlist[c]["AnimDef"][c1].Size(); c2++)
            {
                std::vector<coords> phs;
                for(unsigned int c3=0; c3<sprlist[c]["AnimDef"][c1][c2].Size(); c3++)
                {
                    int x=sprlist[c]["AnimDef"][c1][c2][c3][0].GetInt();
                    int y=sprlist[c]["AnimDef"][c1][c2][c3][1].GetInt();
                    phs.push_back((coords){x,y});
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


