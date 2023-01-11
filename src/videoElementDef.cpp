#include "videoElementDef.h"
#include "elements.h"
namespace videoElement
{
videoElementDef::videoElementDef(ALLEGRO_BITMAP *sprites_)
{

    this->sprites=sprites_;



}

bool videoElementDef::initialized=false;

/*
 This method is rather clumsy, but at the moment seems like it would do.
 First load the sprite tileset, if we want to have them separate with different sprites, here we should start the change - also modify configManager
 It will get configuration manager and then get the gameCOnfigurationObject
 Then it will create all videoElementDefs for each basic element from the board.
 */
void videoElementDef::initializeDriver()
{
    if(videoElementDef::initialized)
        return;
    videoElementDef::initialized=true;
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
        std::cout<<"Could not load sprites tile-set, please check your configuration.\n";

        exit(0);
    }

    for(unsigned int c=0; c<gameConfig->sprites.size(); c++)
    {
        videoElement::videoElementDef* ved=new videoElement::videoElementDef(spriteTs);
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

        switch(gameConfig->sprites[c].eType)
        {
        case _belemType:
            ::bElem::vd=ved;
            break;
        case _floorType:
            ::floorElement::vd=ved;
            break;
        case _wallType:
            ::wall::vd=ved;
            break;
        case _rubishType:
            ::rubbish::vd=ved;
            break;
        case _nonSteppableType:
            ::nonSteppable::vd=ved;
            break;
        case _movableType:
            ::movableElements::vd=ved;
            break;
        case _monster:
            ::monster::vd=ved;
            break;
        case _player:
            ::player::vd=ved;
            break;
        case _collectible:
            ::collectible::vd=ved;
            break;
        case _door:
            ::door::vd=ved;
            break;
        case _key:
            ::key::vd=ved;
            break;
        case _plainGun:
            ::plainGun::vd=ved;
            break;
        case _plainMissile:
            ::plainMissile::vd=ved;
            break;
        case _bunker:
            ::bunker::vd=ved;
            break;
        case _teleporter:
            ::teleport::vd=ved;
            break;
        case _goldenAppleType:
            ::goldenApple::vd=ved;
            break;
        case _simpleBombType:
            ::simpleBomb::vd=ved;
            break;
        case _patrollingDrone:
            ::patrollingDrone::vd=ved;
            break;
        case _brickClusterType:
            ::brickCluster::vd=ved;
            break;
        default: // This can happen with invalid json configuration, let's dispose the driver instance
            delete ved;
            break;
        }
    }
}








videoElementDef::~videoElementDef()
{
    //dtor
}

videoElementDef::videoElementDef(const videoElementDef& other)
{
    //copy ctor
}

}
