#include "presenter.h"




namespace presenter
{
presenter::presenter(std::shared_ptr<chamber> board): sWidth(0),sHeight(0),spacing(0),previousPosition({0,0}),positionOnScreen({0,0})
{
    ALLEGRO_MONITOR_INFO info;
    al_init();

    if(!al_init_image_addon())
    {
        std::cout<<"Could not initialize the image addon!!\n";
        exit(0);

    }
    this->alTimer = al_create_timer(1.0 / 50);
    this->evQueue= al_create_event_queue();
    al_register_event_source(this->evQueue, al_get_timer_event_source(this->alTimer));
    this->_cp_attachedBoard=board;
    al_get_monitor_info(0, &info);
    this->scrWidth = info.x2-50; /* Assume this is 1366 */
    this->scrHeight= info.y2-50; /* Assume this is 768 */
    // this->chaosGameTops.push_back((coords){0,0});
    this->chaosGameTops.push_back({0,0});
    this->chaosGameTops.push_back({this->scrWidth/2,this->scrHeight/2});
    this->chaosGameTops.push_back({this->scrWidth,0});
    this->chaosGameTops.push_back({this->scrWidth,this->scrHeight});
    this->chaosGameTops.push_back({0,this->scrHeight});
    this->chaosGamelastPoint= {1+this->scrWidth/2,1-this->scrHeight/2};


}


bool presenter::initializeDisplay()
{
    ALLEGRO_MONITOR_INFO info;
    //  al_set_new_display_flags(ALLEGRO_FULLSCREEN);
    al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_OPENGL_3_0 ); //ma| ALLEGRO_FULLSCREEN);
    al_inhibit_screensaver(true);
    al_set_new_display_option(ALLEGRO_VSYNC, 0, ALLEGRO_REQUIRE);
    al_get_monitor_info(0, &info);
    this->display = al_create_display(info.x2-info.x1, info.y2-info.y1);
    //  al_hide_mouse_cursor(this->display);
    al_register_event_source(this->evQueue, al_get_display_event_source(this->display));
    this->internalBitmap=al_create_bitmap(this->scrWidth+64,this->scrHeight+64);
    this->cloakBitmap=al_create_bitmap(this->scrWidth+128,this->scrHeight+128);
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
    std::cout<<"Splash"<<this->splashFname.c_str()<<"\n";
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
    this->bluredElement=gcfg->bluredElement;

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



/**
    * Shows a tile for an object.
    *
    * @param x The x-coordinate of the tile to show.
    * @param y The y-coordinate of the tile to show.
    * @param offsetX The offset of the tile in the x-direction.
    * @param offsetY The offset of the tile in the y-direction.
    * @param elem The element to show.
    * @param ignoreOffset Whether to ignore the offset.
    * @param mode The mode to show the object in.
    * @return True if the tile was shown successfully, false otherwise.
    */
bool presenter::showObjectTile(int x, int y, int offsetX, int offsetY, std::shared_ptr<bElem> elem,bool ignoreOffset,int mode)
{
    if(!elem) return false;
    coords coords,offset= {0,0};
    auto vd=videoDriver::getInstance();
    auto ve=(vd)?vd->getVideoElement(elem->getType()):nullptr;
    if(!ve) return false;
    auto draw_sprite = [&]()
    {
        int sx=(coords.x*this->sWidth)+((coords.x+1)*(this->spacing));
        int sy=(coords.y*this->sHeight)+((coords.y+1)*(this->spacing));

        if(!vd)
        {
            std::cout<<"No video driver in memory???\n";
            exit(0);
        };

        if (ve)
            al_draw_bitmap_region(ve->sprites, sx, sy, this->sWidth, this->sHeight, offsetX + (x * this->sWidth), offsetY + (y * this->sHeight), 0);
    };



    //m int sx,sy;
    bool res=false;
    if (x>this->scrTilesX+20 || y>this->scrTilesY+20 || elem.get()==nullptr || !ve ) return false;

    if(!ignoreOffset)
    {
        if(elem.get()!=nullptr)
            offset=elem->getOffset();
        offsetX=offset.x;
        offsetY=offset.y;
    }
    if(mode==1 && !elem->getStats()->isMoving())
        return false;

    if (elem->getStats()->getSteppingOn() && mode!=_mode_onlyTop)
        res=this->showObjectTile(x,y,offsetX,offsetY,elem->getStats()->getSteppingOn(),ignoreOffset,mode);
    if((mode==0 && elem->getStats()->isMoving() ))
        return true;
    int sType=elem->getAttrs()->getSubtype()%ve->defArray.size();
    int sDir=((int)elem->getStats()->getFacing())%ve->defArray[sType].size();
    int sPh=elem->getAnimPh()%ve->defArray[sType][sDir].size();
    if(elem->getType()==_floorType || ( !elem->getStats()->isDying() && !elem->getStats()->isDestroying() && !elem->getStats()->isTeleporting() ))
    {
        coords=ve->defArray[sType][sDir][sPh];
        draw_sprite();
        if (!elem->getType()==_floorType)
            return res;
    }
    if (elem->getStats()->isDying())
    {
        coords=ve->dying[elem->getAnimPh()%(ve->dying.size())];
        draw_sprite();
        return res;
    }
    if (elem->getStats()->isDestroying())
    {
        coords=ve->dying[elem->getAnimPh()%(ve->destroying.size())];
        draw_sprite();
        return res;
    }

    if(elem->getStats()->isFading())
    {
        coords=ve->dying[elem->getAnimPh()%(ve->fadingOut.size())];
        draw_sprite();
        return res;

    }


    if (elem->getStats()->isTeleporting())
    {
        coords=ve->teleporting[elem->getAnimPh()%(ve->teleporting.size())];
        draw_sprite();
        return res;
    }


    return res;
}

/**
    * Shows text.
    *
    * @param x The x-coordinate of the text.
    * @param y The y-coordinate of the text.
    * @param offsetX The offset of the text in the x-direction.
    * @param offsetY The offset of the text in the y-direction.
    * @param text The text to show.
    */
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
    this->showText(2,0,0,32,std::to_string(aPlayer->getAttrs()->getEnergy()));
    this->showObjectTile(4,0,0,0,aPlayer->getAttrs()->getInventory()->getActiveWeapon(),true,_mode_onlyTop);
    this->showObjectTile(6,0,0,0,aPlayer->getAttrs()->getInventory()->getUsable(),true,_mode_onlyTop);



    if( aPlayer->getAttrs()->getInventory()->getActiveWeapon()!=nullptr)
    {
        this->showText(5,0,0,32,std::to_string(aPlayer->getAttrs()->getInventory()->getActiveWeapon()->getAttrs()->getEnergy()));
        this->showText(5,0,0,0,std::to_string(aPlayer->getAttrs()->getInventory()->getActiveWeapon()->getAttrs()->getAmmo()));
    }
    if(aPlayer->getAttrs()->getInventory()->getUsable())
    {
        this->showText(7,0,0,32,std::to_string(aPlayer->getAttrs()->getInventory()->getUsable()->getAttrs()->getEnergy()));
        this->showText(7,0,0,0,std::to_string(
                           aPlayer->getAttrs()->getInventory()->countTokens(aPlayer->getAttrs()->getInventory()->getUsable()->getType(),
                                   aPlayer->getAttrs()->getInventory()->getUsable()->getAttrs()->getSubtype())));
    }


    for (int cnt=0; cnt<5; cnt++)
    {
        int tokens;
        std::shared_ptr<bElem> key=aPlayer->getAttrs()->getInventory()->getKey(_key,cnt,false);
        if(key!=nullptr)
        {
            tokens=aPlayer->getAttrs()->getInventory()->countTokens(key->getType(),key->getAttrs()->getSubtype());
            this->showObjectTile(8+(cnt*2),0,0,0,key,true,_mode_onlyTop);
            this->showText(9+(cnt*2),0,0,16,std::to_string(tokens));
        }
    }
    this->showObjectTile(18,0,0,0,goldenApple::getApple(1),true,_mode_onlyTop);
    this->showText(19,0,0,0,std::to_string(goldenApple::getAppleNumber()));
    this->showText(19,0,0,32,std::to_string(aPlayer->getAttrs()->getInventory()->countTokens(_goldenAppleType,0)));
    this->showText(21,0,6,0,"Stats");
    this->showText(21,0,5,32,"P:");
    this->showText(21,1,5,0,"Dex:");
    this->showText(22,0,5,32,std::to_string(aPlayer->getStats()->getPoints(TOTAL)));
    this->showText(22,1,5,0,std::to_string((int)(log(aPlayer->getStats()->getPoints(SHOOT)+1)/log(5))));

}


/* We kinda move a window in a big screen, that is whole board.We track the upper left point, which is placed in previousPosition
 */
void presenter::showGameField()
{


    std::shared_ptr<bElem> player=player::getActivePlayer();
    coords b=viewPoint::get_instance()->getViewPoint()-(coords)
    {
        (this->scrTilesX)/2,(this->scrTilesY)/2
    };
    std::vector<movingSprite> mSprites;
    mSprites.clear();
    int x,y;
    int bx,by; /* middle of the screen calculated based on the player position - destination */
    int dx,dy;
    int offX=0,offY=0;
    bx=(b.x>0)?((b.x>player->getBoard()->width-(this->scrTilesX))?player->getBoard()->width-(this->scrTilesX):b.x):0;
    by=(b.y>0)?((b.y>(player->getBoard()->height-(this->scrTilesY)))?player->getBoard()->height-(this->scrTilesY):b.y):0;
    dx=(bx-this->previousPosition.x);
    dy=(by-this->previousPosition.y);
    if (dx==0 && this->positionOnScreen.x % this->sWidth>0) dx=-1;
    if (dy==0 && this->positionOnScreen.y % this->sHeight>0) dy=-1;
    this->positionOnScreen.x+=dx*8;
    this->positionOnScreen.y+=dy*8;
    this->previousPosition.x=this->positionOnScreen.x/this->sWidth;
    this->previousPosition.y=this->positionOnScreen.y/this->sHeight;
    offX=(this->positionOnScreen.x % this->sWidth);
    offY=(this->positionOnScreen.y % this->sHeight);
    soundManager::getInstance()->setListenerVelocity({dx*555,0,dy*555});
    this->prepareStatsThing();
    al_set_target_bitmap(this->internalBitmap);
    colour c=this->_cp_attachedBoard->getChColour();
    al_clear_to_color(al_map_rgba(c.r,c.g,c.b,c.a));
    /***
    draw only visible elements, walls are always visible.
    ***/
    if(player->getBoard().get()!=nullptr)
    {
        for(x=0; x<this->scrTilesX+1; x++)
            for(y=0; y<this->scrTilesY+1; y++)
            {
                int nx=x+this->previousPosition.x;
                int ny=y+this->previousPosition.y;
                coords np=(coords)
                {
                    nx,ny
                };
                std::shared_ptr<bElem> elemToDisplay=player->getBoard()->getElement(nx,ny);
                if(player->getBoard()->isVisible(nx,ny)>=255 && !viewPoint::get_instance()->isPointVisible(np))
                    continue; // this element is not even discovered yet
                if(elemToDisplay.get()!=nullptr)
                {
                    if (this->showObjectTile(x,y,0,0,elemToDisplay,false,0))
                        mSprites.push_back({x,y,elemToDisplay});
                    continue;
                }
            }
    }
    int px=0,py=0;
    /***
    draw elements on the move, if the element is steppable and has a still element on it, then drawing will be broken.
    ***/
    for(unsigned int cnt=0; cnt<mSprites.size(); cnt++)
    {
        movingSprite ms=mSprites.at(cnt);
        if(ms.elem->getStats()->getInstanceId()==player->getStats()->getInstanceId())
        {
            px=ms.x;
            py=ms.y;

        }
        else
            this->showObjectTile(ms.x,ms.y,0,0,ms.elem,false,1);
    }
    if(player->getStats()->isMoving() && player->getBoard()->width>viewPoint::get_instance()->getViewPoint().x && viewPoint::get_instance()->getViewPoint().x>=0 && player->getBoard()->height>viewPoint::get_instance()->getViewPoint().y && viewPoint::get_instance()->getViewPoint().y>=0)
        this->showObjectTile(px,py,0,0,player->getBoard()->getElement(viewPoint::get_instance()->getViewPoint()),false,1);
    /***
    Draw the cloak on the game field
    ***/
    this->drawCloak();
    al_set_target_bitmap(al_get_backbuffer(display));
    al_clear_to_color(al_map_rgba(15,25,45,255));
    al_draw_bitmap_region(this->internalBitmap,offX,offY,this->bsWidth,this->bsHeight,_offsetX,_offsetY/2,0);
    al_draw_bitmap_region(this->statsStripe,0,0,this->bsWidth-1,128,_offsetX,this->bsHeight+(_offsetY/2),0);
    al_flip_display();
}

void presenter::drawCloak()
{
    if(viewPoint::get_instance()->getViewPoint()!=NOCOORDS)
    {

        auto ve=videoDriver::getInstance()->getVideoElement(player::getActivePlayer()->getType());
        int obscured;
        int divider=8;
        coords be=this->bluredElement[player::getActivePlayer()->getBoard()->getInstanceId()%this->bluredElement.size()];
        for(int x=-1; x<this->scrTilesX+2; x++)
            for(int y=-1; y<this->scrTilesY+2; y++)
            {
                int nx=x+this->previousPosition.x;
                int ny=y+this->previousPosition.y;
                coords np=(coords)
                {
                    nx,ny
                };

                if(viewPoint::get_instance()->isPointVisible(np))
                {
                    for(int x1=0; x1<divider; x1++)
                        for(int y1=0; y1<divider; y1++)
                        {
                            coords np1=(np*divider)+ (coords)
                            {
                                x1,y1
                            };
                            obscured=std::min(255,viewPoint::get_instance()->calculateObscured(np1,divider));
                            if(obscured>0)
                            {
                                int sx=(be.x*this->sWidth)+((be.x+1)*(this->spacing))+(x1*this->sWidth)/divider;
                                int sy=(be.y*this->sHeight)+((be.y+1)*(this->spacing))+(y1*this->sHeight)/divider;
                                al_draw_tinted_bitmap_region(ve->sprites,al_map_rgba(255,255,255,obscured),sx,sy,this->sWidth/divider,this->sHeight/divider,((x+0)*this->sWidth)+(x1*this->sWidth)/divider,((y+0)*this->sHeight)+(y1*this->sHeight)/divider,0);
                            };
                        }
                    continue;
                }
                int sx=(be.x*this->sWidth)+((be.x+1)*(this->spacing));
                int sy=(be.y*this->sHeight)+((be.y+1)*(this->spacing));
                al_draw_bitmap_region(ve->sprites,sx,sy,this->sWidth,this->sHeight,((x+0)*this->sWidth),((y+0)*this->sHeight),0);
            }

    }
}




void presenter::eyeCandy(int flavour)
{
    int top;
    coords np;
    if(flavour==0)
    {
        for(int x=0; x<1000; x++)
        {
            np= {(int)(bElem::randomNumberGenerator()%this->scrWidth),(int)(bElem::randomNumberGenerator()%this->scrHeight)};
            al_draw_pixel(np.x,np.y,al_map_rgba(255,0,255,bElem::randomNumberGenerator()%10));
        }
    }
    if(flavour==2)
    {
        for(int c=0; c<1000; c++)
        {
            top=bElem::randomNumberGenerator()%this->chaosGameTops.size();
            np= {(this->chaosGameTops[top].x+this->chaosGamelastPoint.x)/2,(this->chaosGameTops[top].y+this->chaosGamelastPoint.y)/2};
            this->chaosGamePoints.push_back(np);
            this->chaosGamelastPoint=np;
            if (this->chaosGamePoints.size()>2000)
                this->chaosGamePoints.erase(this->chaosGamePoints.begin());

        }

        for(unsigned int c=0; c<this->chaosGameTops.size(); c++)
        {
            al_draw_pixel(this->chaosGameTops[c].x,this->chaosGameTops[c].y,al_map_rgba(255,0,255,10));
        }

        for(unsigned int c=0; c<this->chaosGamePoints.size(); c++)
        {
            al_draw_pixel(this->chaosGamePoints[c].x,this->chaosGamePoints[c].y,al_map_rgba(255,255,255,5));
        }
        return;
    }
}

void presenter::mechanicLoop()
{
    while(!this->fin)
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        {
            std::lock_guard<std::mutex> guard(this->presenter_mutex);
            if (player::getActivePlayer().get()!=nullptr)
                this->showGameField();
            else
            {
                std::cout<<"Player not created!\n";
                //this->showSplash();
                this->fin=true;
            }
        }
    }
}

int presenter::presentEverything()
{
    std::shared_ptr<bElem> currentPlayer=nullptr;
    ALLEGRO_EVENT event;
    controlItem cItem;
    if(!this->mStarted)
    {
        this->mStarted=true;
        std::thread nt=std::thread(&presenter::mechanicLoop, this);
        nt.detach();
    }

    al_start_timer(this->alTimer);
    while(!this->fin)
    {
        al_wait_for_event(this->evQueue, &event);
        if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            inputManager::getInstance()->stop();
            this->fin = true;
            break;
        }
        if(event.type == ALLEGRO_EVENT_TIMER)
        {
            std::lock_guard<std::mutex> guard(this->presenter_mutex);
            currentPlayer=player::getActivePlayer();
            this->_cp_attachedBoard->player=NOCOORDS;
            if(currentPlayer.get()!=nullptr)
            {
                this->_cp_attachedBoard=player::getActivePlayer()->getBoard();
                if(currentPlayer->getAttrs()->getInventory()->countTokens(_goldenAppleType,0)==goldenApple::getAppleNumber())
                {
                    inputManager::getInstance()->stop();
                    this->fin=true;
                }

            }
            bElem::runLiveElements();
        }
        else
        {

            if((currentPlayer=player::getActivePlayer()).get()==nullptr)
            {
                this->fin=true;
                inputManager::getInstance()->stop();
                return 2;
            }
            cItem=inputManager::getInstance()->getCtrlItem(); //We always got a status on what to do. remember, everything must have a timer!
            // the idea is to serve the keyboard state constantly, we avoid actions that are too fast
            // by having timers on everything, like: once you shoot, you will be able to shoot in some defined time
            // same with movement, object cycling, gun cycling, using things, interacting with things.
            if (cItem.type==7)
            {

                this->fin=true;
                inputManager::getInstance()->stop();
                return 1;
            }

        }
    }
    this->fin=true;
    inputManager::getInstance()->stop();
    return 1;
}



}  //namespace
