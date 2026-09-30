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
#include "allegroHandles.h"
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
#include "gameSerializer.h"
#include "gameSettings.h"
#include "fogLayer.h"
#include "storyScroller.h"

#define _offsetX 64
#define _offsetY 64

#define _mode_onlyFloor 4
#define _mode_onlyTop   5
#define _mode_all       6





namespace presenter
{

/// why presentEverything returned
enum class gameEnd { QUIT, LOST };
enum class presenterMode { MENU=0, SETTINGS=1,EDITOR=2,DEMO=3,GAME=4} ;
enum class _cp_gameReasonOut { LOST=0, USERREQ=1, PAUSE=2, TELEPORTREQ=3 };



class presenter
{
public:
    presenter();
    ~presenter() = default;
    bool initializeDisplay();
    /// runs the game until the player quits or the last avatar is gone
    gameEnd presentEverything();
    /// the active player's score, as last shown; still there after the last avatar died
    int getLastScore() const;
    bool presentAChamber(presenterMode mod);
    bool loadCofiguredData();
    void showSplash();
    bool showObjectTile(int x,int y,int offsetX,int offsetY,std::shared_ptr<bElem> elem,bool ignoreOffset,int mode);
    void showText(int x,int y,int offsetX,int offsetY,std::string text);
    //relX and relY are coordinates on a board, that indicate where the player is
    void showGameField();
    //void showGameFieldLoop();
    void prepareStatsThing();


private:
    void drawCloak();
    bool fin=false;
    int lastScore=0;
    bool saveKeyDown=false;
    bool loadKeyDown=false;
    /// 0 nothing, 1 save, 2 load; waits here while a level is being generated
    int pendingSaveOp=0;
    /// F5 saves the game, F9 loads it; both act once per key press, between game ticks
    void handleSaveKeys();
    bool mStarted=false;
    void eyeCandy(int flavour);
    std::vector<coords> chaosGamePoints;
    std::vector<coords> chaosGameTops;
    coords chaosGamelastPoint;
    goe::fontHandle myfont;
    /// the story line over the game field, and the font it is drawn in
    goe::fontHandle storyFont;
    goe::storyScroller stories;
    /// worldBuilder::chunksGenerated() when the scroller last looked
    std::size_t seenChunks = 0;
    /// starts a story when the maze grew, and moves the one on screen; once per tick
    void tickStories();
    void drawStory();
    std::string splashFname;
    fogLayer fog;
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
    goe::bitmapHandle internalBitmap;
    goe::bitmapHandle statsStripe;
    int bsHeight,bsWidth;
    _cp_gameReasonOut presentGamePlay();
    std::shared_ptr<chamber> _cp_attachedBoard;
    goe::timerHandle alTimer;
    goe::eventQueueHandle evQueue; // after the timer, so the queue goes first
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
