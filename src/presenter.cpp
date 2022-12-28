#include "presenter.h"




namespace presenter
{
presenter::presenter(std::shared_ptr<chamber> board): sWidth(0),sHeight(0),spacing(0),previousPosition({0,0}),positionOnScreen({0,0})
{
    ALLEGRO_MONITOR_INFO info;
    al_init();
    al_install_keyboard();
    if(!al_init_image_addon())
    {
        std::cout<<"Could not initialize the image addon!!\n";
        exit(0);

    }
    this->alTimer = al_create_timer(1.0 / 50);
    this->evQueue= al_create_event_queue();
    al_register_event_source(this->evQueue, al_get_keyboard_event_source());
    al_register_event_source(this->evQueue, al_get_timer_event_source(this->alTimer));
    this->_cp_attachedBoard=board;
    al_get_monitor_info(0, &info);
    this->scrWidth = info.x2-50; /* Assume this is 1366 */
    this->scrHeight= info.y2-50; /* Assume this is 768 */
    this->inpMngr=std::make_shared<inputManager>();
}


bool presenter::initializeDisplay()
{
    al_set_new_display_flags(ALLEGRO_WINDOWED);
    al_set_new_display_option(ALLEGRO_VSYNC, 0, ALLEGRO_REQUIRE);
    this->display = al_create_display(this->scrWidth, this->scrHeight);
    al_register_event_source(this->evQueue, al_get_display_event_source(this->display));
    this->internalBitmap=al_create_bitmap(this->scrWidth+64,this->scrHeight+64);
    this->statsStripe=al_create_bitmap(this->scrWidth,this->scrHeight/3);
    return true;

}




bool presenter::presentAChamber(presenterMode mod)
{
    switch(mod)
    {
    case presenterMode::MENU:
        break;
    case presenterMode::EDITOR:
        break;
    case presenterMode::DEMO:
        break;
    case presenterMode::SETTINGS:
        break;
    case presenterMode::GAME:
        this->presentGamePlay();
        break;
    }
    return true;
}
_cp_gameReasonOut presenter::presentGamePlay()
{

    return _cp_gameReasonOut::USERREQ;
}

void presenter::showSplash()
{
    ALLEGRO_BITMAP *splash=al_load_bitmap(this->splashFname.c_str());
    al_clear_to_color(al_map_rgba(15,15,25,255));
    al_draw_bitmap(splash,450,0,0);
    al_wait_for_vsync();
    al_flip_display();
}
bool presenter::loadCofiguredData()
{
    videoElement::videoElementDef::initializeDriver();
    std::shared_ptr<gameConfig> gcfg=configManager::getInstance()->getConfig();
    al_init_font_addon();
    al_init_ttf_addon();
    this->myfont=al_load_ttf_font(gcfg->FontFile.c_str(),32,0);
    if(this->myfont==nullptr)
    {
        std::cout<<"Font assets are not loaded properly, check the configuration.\n";
        return false;
    }
    this->splashFname=gcfg->splashScr;
    this->sWidth=gcfg->tileWidth;
    this->sHeight=gcfg->tileHeight;
    this->spacing=gcfg->spacing;
    this->scrTilesX=(this->scrWidth-(2*_offsetX))/this->sWidth;
    this->scrTilesY=((this->scrHeight-(2*_offsetY))/this->sHeight)-1;
    this->bsWidth=this->scrTilesX*this->sWidth;
    this->bsHeight=this->scrTilesY*this->sHeight;
    return true;
}


// let's draw a tile for an object
/*
   we take data from videElementDef and we use it against the element's inner state (like directions, subtype, animphase)
   We use width, height (tile), spacing (that is used to help drawing)
   and finally screen location (in tiles) and offsets, location of the left upper corner on the screen
   we take the coordinates on the sprites surface from videoElement, to do that we take direction, subtype and animation phase from the element
   calculate modulos (mas phases, mas subtypes and max directions) and then we take coordinates

*/

void presenter::showObjectTile(int x, int y, int offsetX, int offsetY, std::shared_ptr<bElem> elem,bool ignoreOffset,int mode)
{
    coords coords,offset= {0,0};
    int sx,sy;
    if (x>this->scrTilesX+20 || y>this->scrTilesY+20 || elem==nullptr) return;
    if(!ignoreOffset)
    {
        if(elem!=nullptr)
            offset=elem->getOffset();
        offsetX=offset.x;
        offsetY=offset.y;
    }
    /* We check, if the object is standing on anything, to draw it first, we also make sure, we were called the right way */
    if (elem->getSteppingOnElement()!=nullptr && (mode==_mode_all || mode==_mode_onlyFloor))
        this->showObjectTile(x,y,offsetX,offsetY,elem->getSteppingOnElement(),ignoreOffset,_mode_all);
    /*
        No video object definition? ignore This way we can have "invisible" objects if we want to.
        mode==_mode_onlyFLoor means, only object that are being stepped on, are drawn
    */
    if (elem->getVideoElementDef()==nullptr || (mode==_mode_onlyFloor))
        return;



    int sType=elem->getSubtype()%elem->getVideoElementDef()->defArray.size();
    int sDir=((int)elem->getDirection())%elem->getVideoElementDef()->defArray[sType].size();
    int sPh=elem->getAnimPh()%elem->getVideoElementDef()->defArray[sType][sDir].size();


    if( elem->getType()==_belemType || elem->isSteppable()==true || elem->canBeDestroyed()==false || (!elem->isDying() && !elem->isDestroyed() && !elem->isTeleporting()) )
    {
        coords=elem->getVideoElementDef()->defArray[sType][sDir][sPh];
        //now calculate the position on the sprites surface
        sx=(coords.x*this->sWidth)+((coords.x+1)*(this->spacing));
        sy=(coords.y*this->sHeight)+((coords.y+1)*(this->spacing));
        //finally draw that
        al_draw_bitmap_region(elem->getVideoElementDef()->sprites,sx,sy,this->sWidth,this->sHeight,offsetX+(x*this->sWidth),offsetY+(y*this->sHeight),0);
    }



    if (elem->isDying())
    {
        coords=elem->getVideoElementDef()->dying[elem->getAnimPh()%(elem->getVideoElementDef()->dying.size())];
        sx=(coords.x*this->sWidth)+((coords.x+1)*(this->spacing));
        sy=(coords.y*this->sHeight)+((coords.y+1)*(this->spacing));
        //finally draw that
        al_draw_bitmap_region(elem->getVideoElementDef()->sprites,sx,sy,this->sWidth,this->sHeight,offsetX+(x*this->sWidth),offsetY+(y*this->sHeight),0);
    }
    if (elem->isDestroyed())
    {
        coords=elem->getVideoElementDef()->dying[elem->getAnimPh()%(elem->getVideoElementDef()->destroying.size())];
        sx=(coords.x*this->sWidth)+((coords.x+1)*(this->spacing));
        sy=(coords.y*this->sHeight)+((coords.y+1)*(this->spacing));
        //finally draw that
        al_draw_bitmap_region(elem->getVideoElementDef()->sprites,sx,sy,this->sWidth,this->sHeight,offsetX+(x*this->sWidth),offsetY+(y*this->sHeight),0);
    }

    if(elem->isFading())
    {
        coords=elem->getVideoElementDef()->dying[elem->getAnimPh()%(elem->getVideoElementDef()->fadingOut.size())];
        sx=(coords.x*this->sWidth)+((coords.x+1)*(this->spacing));
        sy=(coords.y*this->sHeight)+((coords.y+1)*(this->spacing));
        //finally draw that
        al_draw_bitmap_region(elem->getVideoElementDef()->sprites,sx,sy,this->sWidth,this->sHeight,offsetX+(x*this->sWidth),offsetY+(y*this->sHeight),0);

    }


    if (elem->isTeleporting())
    {
        coords=elem->getVideoElementDef()->teleporting[elem->getAnimPh()%(elem->getVideoElementDef()->teleporting.size())];
        sx=(coords.x*this->sWidth)+((coords.x+1)*(this->spacing));
        sy=(coords.y*this->sHeight)+((coords.y+1)*(this->spacing));
        //finally draw that
        al_draw_bitmap_region(elem->getVideoElementDef()->sprites,sx,sy,this->sWidth,this->sHeight,offsetX+(x*this->sWidth),offsetY+(y*this->sHeight),0);
    }
}


void presenter::showText(int x, int y, int offsetX, int offsetY,std::string  text)
{
    ALLEGRO_COLOR c=al_map_rgb(255,255,200);
    int scrx=offsetX+(x*this->sWidth),scry=offsetY+(y*this->sHeight);
    if(this->myfont!=nullptr)
    {
        al_draw_text(this->myfont,c,(float)scrx,(float)scry,0,text.c_str());
    }
}


void presenter::prepareStatsThing()
{
    std::shared_ptr<bElem> aPlayer=player::getActivePlayer();
    al_set_target_bitmap(this->statsStripe);
    al_clear_to_color(al_map_rgba(0,0,0,255));
    this->showText(1,1,0,5,"Garden: "+aPlayer->getBoard()->getName());
    this->showObjectTile(1,0,0,0,aPlayer,true,_mode_onlyTop);
    this->showText(2,0,0,0,std::to_string(player::countVisitedPlayers()));
    this->showText(2,0,0,32,std::to_string(aPlayer->getEnergy()));
    this->showObjectTile(4,0,0,0,aPlayer->getInventory()->getActiveWeapon(),true,_mode_onlyTop);
    if( aPlayer->getInventory()->getActiveWeapon()!=nullptr)
    {
        this->showText(5,0,0,32,std::to_string(aPlayer->getInventory()->getActiveWeapon()->getEnergy()));
        this->showText(5,0,0,0,std::to_string(aPlayer->getInventory()->getActiveWeapon()->getAmmo()));
    }
    for (int cnt=0; cnt<5; cnt++)
    {
        int tokens;
        std::shared_ptr<bElem> key=aPlayer->getInventory()->getKey(_key,cnt,false);
        if(key!=nullptr)
        {
            tokens=aPlayer->getInventory()->countTokens(key->getType(),key->getSubtype());
            this->showObjectTile(7+(cnt*2),0,0,0,key,true,_mode_onlyTop);
            this->showText(8+(cnt*2),0,0,16,std::to_string(tokens));
        }
    }
    this->showObjectTile(18,0,0,0,goldenApple::getApple(1),true,_mode_onlyTop);
    this->showText(19,0,0,0,std::to_string(goldenApple::getAppleNumber()));
    this->showText(19,0,0,32,std::to_string(aPlayer->getInventory()->countTokens(_goldenAppleType,0)));
    this->showText(21,0,6,0,"Stats");
    this->showText(21,0,5,32,"P:");
    this->showText(21,1,5,0,"Dex:");
    if(aPlayer->getStats()!=nullptr)
    {
        this->showText(22,0,5,32,std::to_string(aPlayer->getStats()->getGlobalPoints()));
        this->showText(22,1,5,0,std::to_string(aPlayer->getStats()->getDexterity()));
    }
}


/* We kinda move a window in a big screen, that is whole board.We track the upper left point, which is placed in previousPosition
 */
void presenter::showGameField()
{
    std::shared_ptr<bElem> player=player::getActivePlayer();
    coords b=player->getCoords()-(coords)
    {
        (this->scrTilesX)/2,(this->scrTilesY)/2
    };
    std::vector<movingSprite> mSprites;
    mSprites.clear();
    int x,y;
    int bx,by;
    int dx,dy;
    int offX=0,offY=0;
    bx=(b.x>0)?((b.x>player->getBoard()->width-(this->scrTilesX))?player->getBoard()->width-(this->scrTilesX):b.x):0;
    by=(b.y>0)?((b.y>(player->getBoard()->height-(this->scrTilesY)))?player->getBoard()->height-(this->scrTilesY):b.y):0;
    dx=(bx-this->previousPosition.x);
    dy=(by-this->previousPosition.y);
    if (dx==0 && this->positionOnScreen.x % this->sWidth>0) dx=-1;
    if (dy==0 && this->positionOnScreen.y % this->sHeight>0) dy=-1;
    this->positionOnScreen.x+=dx*2;
    this->positionOnScreen.y+=dy*2;
    this->previousPosition.x=this->positionOnScreen.x/this->sWidth;
    this->previousPosition.y=this->positionOnScreen.y/this->sHeight;
    offX=(this->positionOnScreen.x % this->sWidth);
    offY=(this->positionOnScreen.y % this->sHeight);

    this->prepareStatsThing();
    al_set_target_bitmap(this->internalBitmap);
    colour c=this->_cp_attachedBoard->getChColour();
    al_clear_to_color(al_map_rgba(c.r,c.g,c.b,c.a));
    for(x=0; x<this->scrTilesX+1; x++)
        for(y=0; y<this->scrTilesY+1; y++)
        {
            std::shared_ptr<bElem> elemToDisplay=player->getBoard()->getElement(x+(this->previousPosition.x),y+(this->previousPosition.y));
            if(elemToDisplay!=nullptr && elemToDisplay->getMoved()>0)
            {
                mSprites.push_back({x,y,elemToDisplay});
                this->showObjectTile(x,y,0,0,elemToDisplay,false,_mode_onlyFloor);

                continue;
            }
            this->showObjectTile(x,y,0,0,elemToDisplay,false,_mode_all);
        }
    for(unsigned int cnt=0; cnt<mSprites.size(); cnt++)
    {
        movingSprite ms=mSprites.at(cnt);
        this->showObjectTile(ms.x,ms.y,0,0,ms.elem,false,_mode_onlyTop);

    }

    al_set_target_bitmap(al_get_backbuffer(display));
    al_clear_to_color(al_map_rgba(15,25,45,255));
    al_draw_bitmap_region(this->internalBitmap,offX,offY,this->bsWidth,this->bsHeight,_offsetX,_offsetY/2,0);
    al_draw_bitmap_region(this->statsStripe,0,0,this->bsWidth-1,128,_offsetX,this->bsHeight+(_offsetY/2),0);
    al_wait_for_vsync();
    al_flip_display();
}


int presenter::presentEverything()
{
    std::shared_ptr<bElem> currentPlayer=nullptr;
    ALLEGRO_EVENT event;
    controlItem cItem;
    bool fin=false;

    bElem::runLiveElements();
    al_start_timer(this->alTimer);
    while(!fin)
    {
        al_wait_for_event(this->evQueue, &event);

        if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            fin = true;
            break;
        }
        if(event.type == ALLEGRO_EVENT_TIMER)
        {
            this->_cp_attachedBoard->player=NOCOORDS;
            bElem::runLiveElements();
            if((currentPlayer=player::getActivePlayer())==nullptr)
            {
                return 2;
            }
            if(currentPlayer->getInventory()->countTokens(_goldenAppleType,0)==goldenApple::getAppleNumber())
                fin=true;
            this->_cp_attachedBoard=currentPlayer->getBoard();
            this->showGameField();
        }
        cItem=this->inpMngr->translateEvent(&event); //We always got a status on what to do. remember, everything must have a timer!
        // the idea is to serve the keyboard state constantly, we avoid actions that are too fast
        // by having timers on everything, like: once you shoot, you will be able to shoot in some defined time
        // same with movement, object cycling, gun cycling, using things, interacting with things.
        if (cItem.type==7)
            return 1;
        if(currentPlayer!=nullptr && currentPlayer->getBoard()!=nullptr)
            currentPlayer->getBoard()->cntrlItm=cItem;
    }
    return 1;
}



}  //namespace
