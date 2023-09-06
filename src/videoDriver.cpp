#include "videoDriver.h"
videoDriver* videoDriver::myInstance=nullptr;


videoDriver::videoDriver():std::enable_shared_from_this<videoDriver>()
{

    auto gameConfig=configManager::getInstance()->getConfig();
    if(gameConfig==nullptr || gameConfig->FontFile.size()==0 || gameConfig->spriteFile.size()==0)
    {
        std::cout<<"Graphical assets are not defined in the config file\n";
        exit(0);
    }
    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
    ALLEGRO_BITMAP *spriteTs=al_load_bitmap(gameConfig->spriteFile.c_str());
    if (spriteTs==nullptr)
    {
        std::cout<<"Could not load sprites tile-set, please check your configuration.\n"<<gameConfig->spriteFile.c_str()<<"\n";
        exit(0);
    }

    for(unsigned int c=0; c<gameConfig->sprites.size(); c++)
    {
        vElement ved=std::make_shared<videoElement::videoElementDef>(spriteTs);
        if(gameConfig->sprites[c].dying.size()>0)
        {
            for(auto e:gameConfig->sprites[c].dying)
                ved->dying.push_back(e);
        }
        else
        {
            for(auto e:gameConfig->gDying)
                ved->dying.push_back(e);
        }
        if(gameConfig->sprites[c].destroying.size()>0)
        {
            for(auto e:gameConfig->sprites[c].destroying)
                ved->destroying.push_back(e);
        }
        else
        {
            for(auto e:gameConfig->gDestroying)
                ved->destroying.push_back(e);
        }
        if(gameConfig->sprites[c].teleporting.size()>0)
        {
            for(auto e:gameConfig->sprites[c].teleporting)
                ved->teleporting.push_back(e);
        }
        else
        {
            for(auto e:gameConfig->gTeleporting)
                ved->teleporting.push_back(e);
        }

        if(gameConfig->sprites[c].fadingOut.size()>0)
        {
            for(auto e:gameConfig->sprites[c].fadingOut)
                ved->fadingOut.push_back(e);
        }
        else
        {
            for(auto e:gameConfig->gFadingOut)
                ved->fadingOut.push_back(e);
        }
        for(unsigned int c1=0; c1<gameConfig->sprites[c].animDef.size(); c1++)
        {
            std::vector<std::vector<coords>> dirs;
            for(unsigned int c2=0; c2<gameConfig->sprites[c].animDef[c1].size(); c2++)
            {
                std::vector<coords> phs;
                for(unsigned int c3=0; c3<gameConfig->sprites[c].animDef[c1][c2].size(); c3++)
                {
                    int x=gameConfig->sprites[c].animDef[c1][c2][c3].x;
                    int y=gameConfig->sprites[c].animDef[c1][c2][c3].y;
                    phs.push_back((coords)
                    {
                        x,y
                    });

                }
                dirs.push_back(phs);
            }
            ved->defArray.push_back(dirs);
        }
        this->sprites[gameConfig->sprites[c].eType]=ved;

    }
}

videoDriver::~videoDriver()
{
    //dtor
}

videoDriver* videoDriver::getInstance()
{
    if(!videoDriver::myInstance)
    {
        videoDriver* vd=new videoDriver();
        videoDriver::myInstance=vd;
    }

    return videoDriver::myInstance;
}


vElement videoDriver::getVideoElement(int typeId)
{
    if(typeId==this->lastSpriteT)
        return this->lSprite;
    auto fnd=this->sprites.find(typeId);
    if(fnd!=this->sprites.end())
    {
        this->lastSpriteT=typeId;
        this->lSprite=fnd->second;
        return this->lSprite;
    }
    return nullptr;
}

