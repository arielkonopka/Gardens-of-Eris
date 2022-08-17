#include "presenter.h"




namespace presenter
{
presenter::presenter(chamber *board)
{
    ALLEGRO_MONITOR_INFO info;
    al_init();
    al_install_keyboard();

    if(!al_init_image_addon())
    {
        std::cout<<"Dupa nie inicjalizacja!\n";

    }
    this->alTimer = al_create_timer(1.0 / 15.0);
    this->scrTimer=al_create_timer(1.0/40);
    this->evQueue= al_create_event_queue();
    al_register_event_source(this->evQueue, al_get_keyboard_event_source());
    al_register_event_source(this->evQueue, al_get_timer_event_source(this->alTimer));
    al_register_event_source(this->evQueue, al_get_timer_event_source(this->scrTimer));
    this->_cp_attachedBoard=board;
    al_get_monitor_info(0, &info);
    this->scrWidth = info.x2 - info.x1; /* Assume this is 1366 */
    this->scrHeight= info.y2 - info.y1; /* Assume this is 768 */
    this->sWidth=0;
    this->sHeight=0;
    this->spacing=0;
    this->inpMngr=new inputManager();
    this->previousPosition=(coords)
    {
        0,0
    };
    this->positionOnScreen=(coords)
    {
        0,0
    };



}

presenter::~presenter()
{
    delete this->inpMngr;
}
bool presenter::initializeDisplay()
{
    this->display = al_create_display(this->scrWidth, this->scrHeight);
    al_register_event_source(this->evQueue, al_get_display_event_source(this->display));
    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP|ALLEGRO_FULLSCREEN);
    this->internalBitmap=al_create_bitmap(this->scrWidth,this->scrHeight);
    return true;

}

bool presenter::presentAChamber(presenterMode mod)
{
    _cp_gameReasonOut reason=USERREQ;
    switch(mod)
    {
    case MENU:
        break;
    case EDITOR:
        break;
    case DEMO:
        break;
    case SETTINGS:
        break;
    case GAME:
        reason=this->presentGamePlay();
        break;
    }
    return true;
}
_cp_gameReasonOut presenter::presentGamePlay()
{

    return USERREQ;
}


bool presenter::loadCofiguredData()
{
    /*
        read the configuration file
    */
    FILE* fp = fopen("data/skins.json", "rb"); // non-Windows use "r"
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    this->skinDefJson.ParseStream(is);
    fclose(fp);
    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
// ok the sprite filename should be available now, let's load it
    al_init_font_addon();
    al_init_ttf_addon();
    std::cout<<this->skinDefJson["FontFile"].GetString()<<"\n";
    this->myfont=al_load_ttf_font(this->skinDefJson["FontFile"].GetString(),32,0);
    if(this->myfont==NULL)
    {
        std::cout<<"Fonty się nie załadowały\n";
        return false;
    }

    this->sprites=al_load_bitmap(this->skinDefJson["SpriteFile"].GetString());

    if (this->sprites==NULL)
    {
        std::cout<<"Sprity się nie załadowały!\n";
        return false;
    }
//    std::cout<<"Sprites!"<<this->sprites<<" "<<this->skinDefJson["SpriteFile"].GetString()<<"\n";


    //load all the necessary constants about the graphics in the game
    this->sWidth=this->skinDefJson["width"].GetInt();
    this->sHeight=this->skinDefJson["height"].GetInt();
    this->spacing=this->skinDefJson["spacing"].GetInt();
    this->scrTilesX=(this->scrWidth-(2*_offsetX))/this->sWidth;
    this->scrTilesY=((this->scrHeight-(2*_offsetY))/this->sHeight)-2;
    rapidjson::Value& dying=this->skinDefJson["Dying"];
    rapidjson::Value& teleporting=this->skinDefJson["Teleporting"];
    rapidjson::Value& sprlist=this->skinDefJson["SpriteData"];
    this->bsWidth=this->scrTilesX*this->sWidth;
    this->bsHeight=this->scrTilesY*this->sHeight;




//ok, now time for reading sprite data for individual sprites and kill/teleport patterns (TODO:we could make them individual for each object)
    for(unsigned int c=0; c<sprlist.Size(); c++)
    {
        //OK, time to create VideoElementDefinitions
        //read the animation phases array sizes - warning unlike the python counterpart, we will not implement
        // variable length of different anim phases in the same object - that should not be a problem, really
        rapidjson::Value& spr=sprlist[c];
        auto subtypes=sprlist[c]["AnimDef"].Size();
        auto directions=sprlist[c]["AnimDef"][0].Size();
        auto animphases=sprlist[c]["AnimDef"][0][0].Size();
        //     std::cout<<subtypes<<" "<<directions<<" "<<animphases<<"\n";
        videoElement::videoElementDef* ved=new videoElement::videoElementDef(subtypes,directions,animphases);

        for(unsigned int c0=0; c0<dying.Size(); c0++)
        {
            int x=dying[c0][0].GetInt();
            int y=dying[c0][1].GetInt();
            ved->addKill(x,y);
            //std::cout<<x<<" "<<y<<" kill\n";
        }
        for(unsigned int c0=0; c0<teleporting.Size(); c0++)
        {
            int x=teleporting[c0][0].GetInt();
            int y=teleporting[c0][1].GetInt();
            ved->addTeleporting(x,y);
            //std::cout<<x<<" "<<y<<" telep\n";
        }

        //load the animation sprites into videoElementDefinition
        for(unsigned int c1=0; c1<subtypes; c1++)
        {
            for(unsigned int c2=0; c2<directions; c2++)
            {
                for(unsigned int c3=0; c3<animphases; c3++)
                {
                    int x=sprlist[c]["AnimDef"][c1][c2][c3][0].GetInt();
                    int y=sprlist[c]["AnimDef"][c1][c2][c3][1].GetInt();
                    //      std::cout<<c1<<" "<<c2<<" "<<c3<<" "<<x<<" "<<y<<"\n";
                    ved->setValuesFor(this->sprites,c1,c2,c3,x,y); //We set the sprite frame for that particlular anim phase


                }
            }
        }

        //attatch video definition thinggy to the object class
        // Add here all objects that are created
        switch(spr["Type"].GetInt())
        {
        case _belemType:
            ::bElem::vd=ved;
            break;
        case _wallType:
            ::wall::vd=ved;
            break;
        case _stillElemType:
            ::stillElem::vd=ved;
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
        case _explosivesType:
            ::explosives::vd=ved;
            break;


        }
    }
    //Here we should have the videoElementDefinitionsTable filled



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

void presenter::showObjectTile(int x, int y, int offsetX, int offsetY, bElem* elem)
{
    coords coords;
    int sx,sy;
    int aphs,subtyps,dirs;
    videoElement::aphases *phs; //Video element, that defines the animation of an object
//the tile would not fit in the screen? ignore
    // if(elem!=NULL)
    //     std::cout<<elem->getType()<<".";

    if (x>this->scrTilesX+20 || y>this->scrTilesY+20) return;
//empty element? ignore or end the recursion cycle
    if (elem==NULL)
        return;
//check if object is standing on something, if so, draw that first.
    if (elem->steppingOn!=NULL)
        this->showObjectTile(x,y,offsetX,offsetY,elem->steppingOn);

//No video object definition? ignore This way we can have "invisible" objects if we want to.
    if (elem->getVideoElementDef()==NULL)
        return;

    phs=elem->getVideoElementDef()->defArray;
    //get maximum values of subtypes, animphases and directions that are present in the video element definition
    aphs=elem->getVideoElementDef()->animphases;
    subtyps=elem->getVideoElementDef()->subtypes;
    dirs=elem->getVideoElementDef()->directions;
    //get the spprite location in the sprite surface
    if (elem->isDying())
    {
        coords=(*elem->getVideoElementDef()->dying)[elem->getAnimPh()%(elem->getVideoElementDef()->dying->size())];
    }
    else if (elem->isTeleporting())
    {
        coords=(*elem->getVideoElementDef()->teleporting)[elem->getAnimPh()%(elem->getVideoElementDef()->teleporting->size())];
    }
    else
    {
        coords=(*phs)[elem->getSubtype()%subtyps][elem->getDirection()%dirs][elem->getAnimPh()%aphs];
    }
    //now calculate the position on the sprites surface
    sx=(coords.x*this->sWidth)+((coords.x+1)*(this->spacing));
    sy=(coords.y*this->sHeight)+((coords.y+1)*(this->spacing));
    //finally draw that
    al_draw_bitmap_region(elem->getVideoElementDef()->sprites,sx,sy,this->sWidth,this->sHeight,offsetX+(x*this->sWidth),offsetY+(y*this->sHeight),0);
}


void presenter::showText(int x, int y, int offsetX, int offsetY,std::string  text)
{
    ALLEGRO_COLOR c=al_map_rgb(255,255,255);
    int scrx=offsetX+(x*this->sWidth),scry=offsetY+(y*this->sHeight);
    if(this->myfont!=NULL)
    {
        al_draw_text(this->myfont,c,(float)scrx,(float)scry,0,text.c_str());
       // std::cout<<"drawing text\n";
    }
}


// This method shows the gameField. now it uses only one chamber, no chamber selection or other fancy stuff - will probably move that to other class
void presenter::showGameField(int relX,int relY)
{
    bElem* player;

    int x,y;
    int bx=(relX)-((this->scrTilesX)/2);
    int by=(relY)-((this->scrTilesY)/2);
    int dx;
    int dy;
    int offX=0,offY=0;
    if (bx<0)
        bx=0;
    if (by<0)
        by=0;
    if (bx>this->_cp_attachedBoard->width-(this->scrTilesX))
        bx=this->_cp_attachedBoard->width-(this->scrTilesX);

    if (by>(this->_cp_attachedBoard->height-(this->scrTilesY)))
        by=this->_cp_attachedBoard->height-(this->scrTilesY);
    dx=(bx-this->previousPosition.x);
    dy=(by-this->previousPosition.y);
    if (dx==0 && this->positionOnScreen.x % this->sWidth>0) dx=-1;
    if (dy==0 && this->positionOnScreen.y % this->sHeight>0) dy=-1;


    this->positionOnScreen.x+=4*dx;
    this->positionOnScreen.y+=4*dy;
    this->previousPosition.x=this->positionOnScreen.x/this->sWidth;
    this->previousPosition.y=this->positionOnScreen.y/this->sHeight;


    offX=(this->positionOnScreen.x % this->sWidth);
    offY=(this->positionOnScreen.y % this->sHeight);


    ALLEGRO_BITMAP* screen=al_get_target_bitmap();
    // chamber *myChamber=this->_cp_attachedBoard;
//std::cout<<bx<<" "<<by<<" "<<this->positionOnScreen.x<<" "<<this->positionOnScreen.y<<"\n";
    // check if there is a need for top left corner to be recalculated
    if (this->internalBitmap==NULL)
    {
        //  std::cout<<"******************* Dupa!!!\n";
    }

    al_set_target_bitmap(this->internalBitmap);
    al_clear_to_color(al_map_rgba(50,70,120,255));
    for(x=0; x<this->scrTilesX+1; x++)
        for(y=0; y<this->scrTilesY+1; y++)
        {
            this->showObjectTile(x,y,0,0,this->_cp_attachedBoard->getElement(x+(this->positionOnScreen.x/this->sWidth),y+(this->positionOnScreen.y/this->sHeight)));
        }

    al_set_target_bitmap(screen);
    al_clear_to_color(al_map_rgba(25,25,25,255));
    al_draw_bitmap_region(this->internalBitmap,offX,offY,this->bsWidth,this->bsHeight,_offsetX,_offsetY,0);
    player=player::getActivePlayer();
    if(player!=NULL)
    {
        this->showObjectTile(1,this->scrTilesY+2,0,0,player);
        this->showText(2,this->scrTilesY+2,0,0,std::to_string(player->getEnergy()));
        this->showObjectTile(4,this->scrTilesY+2,0,0,player->myInventory->getActiveWeapon());
        if( player->myInventory->getActiveWeapon()!=NULL)
        {
            this->showText(5,this->scrTilesY+2,0,0,std::to_string(player->myInventory->getActiveWeapon()->getEnergy()));

        }
        this->showObjectTile(7,this->scrTilesY+2,0,0,player->myInventory->getKey(_key,0,false));
        this->showObjectTile(10,this->scrTilesY+2,0,0,player->myInventory->getKey(_key,1,false));
        this->showObjectTile(13,this->scrTilesY+2,0,0,player->myInventory->getKey(_key,2,false));
        this->showObjectTile(16,this->scrTilesY+2,0,0,player->myInventory->getKey(_key,3,false));
        this->showObjectTile(19,this->scrTilesY+2,0,0,player->myInventory->getKey(_key,4,false));




    }
    al_wait_for_vsync();

    al_flip_display();
}

void presenter::showGameFieldLoop()
{
    ALLEGRO_TIMER* alTimer = al_create_timer(1000.0 );
    ALLEGRO_EVENT event;
    ALLEGRO_EVENT_QUEUE* evQueue= al_create_event_queue();
    bool fin=false;
    al_register_event_source(evQueue,al_get_timer_event_source(alTimer));
//al_start_timer(alTimer);
    while(!fin)
    {
        al_wait_for_event(evQueue, &event);
        if(event.type == ALLEGRO_EVENT_TIMER)
        {
            this->showGameField(this->_cp_attachedBoard->player.x,this->_cp_attachedBoard->player.y);
            //std::cout<<"zZz";
        }

    }
}



int presenter::presentEverything()
{
    player* currentPlayer=NULL;
    ALLEGRO_EVENT event;
//   presenter* instance=this;
    controlItem cItem;
    bool fin=false;

    // ALLEGRO_THREAD* visual=al_create_thread(shGFL,&instance);

    //  al_start_thread(visual);
    al_start_timer(this->alTimer);
    al_start_timer(this->scrTimer);
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
            if(event.timer.source==this->alTimer)
            {
                this->_cp_attachedBoard->player=NOCOORDS;
                bElem::runLiveElements();
                gCollect::getInstance()->purgeGarbage();
                if((currentPlayer=player::getActivePlayer())==NULL)
                {
                    return 2;
                }
                this->_cp_attachedBoard=currentPlayer->getBoard();

            }

            if(event.timer.source==this->scrTimer)
            {


                this->showGameField(this->_cp_attachedBoard->player.x,this->_cp_attachedBoard->player.y);
                //  std::cout<<"\033[0G x,y ->"<<this->_cp_attachedBoard->player.x<<","<<this->_cp_attachedBoard->player.y;
                //  std::cout<<blue<<"\n";
            }
        }
        cItem=this->inpMngr->translateEvent(&event); //We always got a status on what to do. remember, everything must have a timer!
        // the idea is to serve the keyboard state constantly, we avoid actions that are too fast
        // by having timers on everything, like: once you shoot, you will be able to shoot in some defined time
        // same with movement, object cycling, gun cycling, using things, interacting with things.

        if (cItem.type==7)
            return 1;
        this->_cp_attachedBoard->cntrlItm=cItem;


    }
    return 1;
}



}  //namespace
