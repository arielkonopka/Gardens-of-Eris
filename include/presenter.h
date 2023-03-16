#ifndef CHAMBERPRESENTER_H
#define CHAMBERPRESENTER_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include "allegro5/allegro_native_dialog.h"
#include <vector>
#include <string>
#include "chamber.h"
#include "videoElementDef.h"
#include "elements.h"
#include "inputManager.h"
#include "objectTypes.h"
#include "configManager.h"
#include "soundManager.h"
#include "commons.h"
#include <thread>
#include <mutex>
/*
 This class is responsible for menu, settings, designer and game presenter.

Trzeba utworzyć relację między obiektami, a ich wizualną reprezentacją.
Najlepiej, chcielibyśmy móc przypisywać dowolną definicję do dowolnego obiektu, problem robi się gdy:
trzeba utworzyć nowy obiekt na planszy. - można pobrac z tabeli videoElementów-lub poprosić jakiś obiekt o to.
obiekt się teleportuje (animacja)
obiekt ginie (animacja)

*/

#define _offsetX 64
#define _offsetY 64

#define _mode_onlyFloor 4
#define _mode_onlyTop   5
#define _mode_all       6





namespace presenter
{

enum class presenterMode { MENU=0, SETTINGS=1,EDITOR=2,DEMO=3,GAME=4} ;
enum class _cp_gameReasonOut { LOST=0, USERREQ=1, PAUSE=2, TELEPORTREQ=3 };



class presenter
{
public:
    presenter(std::shared_ptr<chamber> board);
    ~presenter() = default;
    bool initializeDisplay();
    int presentEverything();
    bool presentAChamber(presenterMode mod);
    bool loadCofiguredData();
    void showSplash();
    bool showObjectTile(int x,int y,int offsetX,int offsetY,std::shared_ptr<bElem> elem,bool ignoreOffset,int mode);
    void showText(int x,int y,int offsetX,int offsetY,std::string text);
    //relX and relY are coordinates on a board, that indicate where the player is
    void showGameField();
    ALLEGRO_EVENT_QUEUE* evQueue;
    //void showGameFieldLoop();
    void prepareStatsThing();


private:
    bool fin=false;
    bool mStarted=false;
    void eyeCandy(int flavour);
    std::vector<coords> chaosGamePoints;
    std::vector<coords> chaosGameTops;
    coords chaosGamelastPoint;
    ALLEGRO_FONT* myfont;
    std::string splashFname;
    std::shared_ptr<inputManager> inpMngr;
    int sWidth;
    int sHeight;
    int scrHeight;
    int scrWidth;
    int spacing;
    int scrTilesX;
    int scrTilesY;
    float viewRadius=6.3;
    coords previousPosition;
    coords positionOnScreen;
    coords bluredElement=NOCOORDS;
    coords bluredElement25=NOCOORDS;
    coords bluredElement50=NOCOORDS;
    coords bluredElement75=NOCOORDS;
    ALLEGRO_BITMAP* internalBitmap;
    ALLEGRO_BITMAP* cloakBitmap;
    ALLEGRO_BITMAP* statsStripe;
    int bsHeight,bsWidth;
    _cp_gameReasonOut presentGamePlay();
    std::shared_ptr<chamber> _cp_attachedBoard;
    ALLEGRO_TIMER* alTimer;
    void mechanicLoop();

    ALLEGRO_DISPLAY* display;

    ALLEGRO_BITMAP *sprites;
    typedef struct movingSprite
    {
        int x;
        int y;
        std::shared_ptr<bElem> elem;

    } movingSprite;

    std::thread myThread;
    std::mutex presenter_mutex;

};

#endif // CHAMBERPRESENTER_H
}
