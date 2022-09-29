#ifndef CHAMBERPRESENTER_H
#define CHAMBERPRESENTER_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include "allegro5/allegro_native_dialog.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <vector>
#include <string>
#include "chamber.h"
#include "videoElementDef.h"
#include "elements.h"
#include "inputManager.h"
#include "objectTypes.h"
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

#define _mode_onlyFloor 1
#define _mode_onlyTop   2
#define _mode_all       3





namespace presenter
{

typedef enum { MENU=0, SETTINGS=1,EDITOR=2,DEMO=3,GAME=4} presenterMode;
typedef enum { LOST=0, USERREQ=1, PAUSE=2, TELEPORTREQ=3 } _cp_gameReasonOut;


/*
problemy do rozwiązania:
utworzyć viewport, który będzie pokazywał aktualny kawałek pola gry,
stworzyć metodę, która narysuje widoczny kawałek pola gry
żeby utworzyć viewport, chamber powinien zawierać metodę lokalizacji gracza (zwracać będzie koordynaty aktywnego gracza),
viewport powinien być tak ustawiony, że gracz powinien być w środku pola widzenia, za wyjątkiem sytuacji, gdy gracz dochodzi do krawędzi planszy

na początek jednak nie będziemy tego tak rysować, tylko będziemy rysowac pierwsze scrTilesX i scrTilesY elementów, żeby mieć jakąkolwiek prezentację

*/

class presenter
{
public:
    presenter(chamber *board);
    ~presenter();
    bool initializeDisplay();
    int presentEverything();
    bool presentAChamber(presenterMode mod);
    bool loadCofiguredData();
    void showSplash();
    void showObjectTile(int x,int y,int offsetX,int offsetY,bElem *elem,bool ignoreOffset,int mode);
    void showText(int x,int y,int offsetX,int offsetY,std::string text);
    //relX and relY are coordinates on a board, that indicate where the player is
    void showGameField(int relX, int relY);
    ALLEGRO_EVENT_QUEUE* evQueue;
    void showGameFieldLoop();

protected:
    bool attachGraphicsToSPrites;

private:
    ALLEGRO_FONT* myfont;
    std::string splashFname;
    inputManager* inpMngr;
    int sWidth;
    int sHeight;
    int scrHeight;
    int scrWidth;
    int spacing;
    int scrTilesX;
    int scrTilesY;
    coords previousPosition;
    coords positionOnScreen;

    ALLEGRO_BITMAP* internalBitmap;
    int bsHeight,bsWidth;
    rapidjson::Document skinDefJson;
    _cp_gameReasonOut presentGamePlay();
    chamber *_cp_attachedBoard;
    ALLEGRO_TIMER* alTimer;
   // ALLEGRO_TIMER* scrTimer;

    ALLEGRO_DISPLAY* display;

    ALLEGRO_BITMAP *sprites;
    typedef struct movingSprite
    {
        int x;
        int y;
        bElem* elem;

    } movingSprite;



};

#endif // CHAMBERPRESENTER_H
}
