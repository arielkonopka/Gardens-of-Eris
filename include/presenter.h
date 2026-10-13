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
#include "autopilot.h"
#include "fieldCamera.h"
#include "fieldView.h"
#include "hudView.h"
#include "storyScroller.h"
#include <memory>
#include <mutex>
#include <string>

namespace presenter
{

/// why presentEverything returned: the window closed, the last avatar was lost,
/// the player asked to save and go to the menu (and saving worked, or failed),
/// the player pressed something during the demo, or the demo's time was up
enum class gameEnd { QUIT, LOST, SAVED, SAVE_FAILED, DEMO_OVER, DEMO_DONE };

/**
 * The game loop: ticks the world 50 times a second and shows each tick. The game field is
 * drawn by a fieldView, the stats strip by the hudView; the presenter adds the story line and
 * the demo note over them.
 */
class presenter
{
public:
    presenter();
    ~presenter() = default;
    bool initializeDisplay();
    /// runs the game until the player quits or the last avatar is gone. In a demo the autopilot
    /// plays, nothing is saved, any key, pad button or stick push ends it (DEMO_OVER), and it
    /// stops by itself after demoSeconds (DEMO_DONE).
    gameEnd presentEverything(bool demo = false, int demoSeconds = 0);
    /// the active player's score, as last shown; still there after the last avatar died
    /// after a lost game: the best score of its avatars; otherwise the active avatar's last score
    int getLastScore() const;
    bool loadCofiguredData();
    void showSplash();

private:
    /// draws one frame: the field, the stats strip, the story line and the demo note
    void showGameField();
    bool fin=false;
    int lastScore=0;
    bool saveKeyDown=false;
    bool loadKeyDown=false;
    /// 0 nothing, 1 save, 2 load; waits here while a level is being generated
    int pendingSaveOp=0;
    /// F5 saves the game, F9 loads it; both act once per key press, between game ticks
    void handleSaveKeys();
    /// the story line over the game field, and the font it is drawn in
    goe::fontHandle storyFont;
    goe::storyScroller stories;
    /// worldBuilder::chunksGenerated() when the scroller last looked
    std::size_t seenChunks = 0;
    /// starts a story when the maze grew, and moves the one on screen; once per tick
    void tickStories();
    void drawStory();
    /// playing the demo: the autopilot has the player
    bool demo = false;
    goe::autopilot pilot;
    void drawDemoNote();
    std::string splashFname;
    /// the display, less a margin
    int scrHeight;
    int scrWidth;
    /// whole cells across and down the field, and the field's size in pixels
    coords fieldCells{0, 0};
    coords fieldPx{0, 0};
    std::unique_ptr<fieldCamera> camera;
    /// the 2D view, and the 3D isometric one (empty when this machine cannot draw it)
    std::unique_ptr<fieldView> flatView;
    std::unique_ptr<fieldView> isoView;
    /// the view the settings ask for, or the 2D one
    fieldView &chosenView();
    bool viewKeyDown = false;
    /// F3 switches between the 2D and the 3D view, and keeps the choice in the settings
    void handleViewKey();
    std::unique_ptr<hudView> hud;
    goe::timerHandle alTimer;
    goe::eventQueueHandle evQueue; // after the timer, so the queue goes first
    std::mutex presenter_mutex;
};

}
#endif // CHAMBERPRESENTER_H
