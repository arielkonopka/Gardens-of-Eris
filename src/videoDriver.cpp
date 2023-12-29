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
#include "videoDriver.h"
videoDriver* videoDriver::myInstance=nullptr;
std::once_flag videoDriver::_onceFlag;

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
        if(gameConfig->sprites[c].fadingIn.size()>0)
        {
            for(auto e:gameConfig->sprites[c].fadingIn)
                ved->fadingIn.push_back(e);
        }
        else
        {
            for(auto e:gameConfig->gFadingIn)
                ved->fadingIn.push_back(e);
        }
        for(unsigned int c1=0; c1<gameConfig->sprites[c].animDef.size(); c1++)
        {
            std::vector<std::vector<coords>> dirs;
            for(unsigned int c2=0; c2<gameConfig->sprites[c].animDef[c1].size(); c2++)
            {
                std::vector<coords> phs;
                for(unsigned int c3=0; c3<gameConfig->sprites[c].animDef[c1][c2].size(); c3++)
                {
                    phs.push_back(gameConfig->sprites[c].animDef[c1][c2][c3]);
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
    std::call_once(videoDriver::_onceFlag,[]()
    {
        videoDriver::myInstance=new videoDriver();
    });
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

