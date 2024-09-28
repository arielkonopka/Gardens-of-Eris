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


#ifndef CHAMBERPRESENTER_H
#define CHAMBERPRESENTER_H
#include "commons.h"

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
#include <thread>
#include <videoDriver.h>
#include <mutex>
#include <viewPoint.h>
#include <allegro5/allegro_primitives.h>
#include "videoManager.h"

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
    presenter();
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
    void shaderthing(int x, int y);
    void drawCloak();
    bool fin=false;
    bool mStarted=false;
    void eyeCandy(int flavour);
    std::vector<coords> chaosGamePoints;
    std::vector<coords> chaosGameTops;
    coords chaosGamelastPoint;
    ALLEGRO_FONT* myfont;
    std::string splashFname;
    int shaderId=-1;
    int sWidth;
    int sHeight;
    int scrHeight;
    int scrWidth;
    int spacing;
    int scrTilesX;
    int scrTilesY;
    // float viewRadius=6.3;
    coords previousPosition;
    coords positionOnScreen;
    std::vector<coords> bluredElement;
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

    ALLEGRO_DISPLAY* display;

    std::vector<float> radiuses;
    ALLEGRO_BITMAP *pointsTexture;
    int pointsTextureWidth = 2;
    int pointsTextureHeight = 100;
 //   ALLEGRO_SHADER *shader;

    ALLEGRO_BITMAP *sprites;
    typedef struct movingSprite
    {
        int x;
        int y;
        std::shared_ptr<bElem> elem;

    } movingSprite;

    std::thread myThread;
    std::mutex presenter_mutex;
    std::vector<coords> poses;

};

#endif // CHAMBERPRESENTER_H
}
