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
#include "presenter.h"
#include "configManager.h"
#include "difficulty.h"
#include "elements.h"
#include "flatFieldView.h"
#include "fogLayer.h"
#include "gameSerializer.h"
#include "gameSettings.h"
#include "inputManager.h"
#include "musicCues.h"
#include "soundManager.h"
#include "videoManager.h"
#include "viewPoint.h"
#include "worldBuilder.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <iostream>

namespace presenter {
namespace {
/// the game runs 50 ticks a second
constexpr double tickSeconds = 1.0 / 50;
/// the story line's letters, in pixels; fits the strip above the game field
constexpr int storyFontSize = 23;
/// the game field's distance from the left edge of the screen, and twice its distance from the top
constexpr int fieldMargin = 64;
} // namespace

presenter::presenter()
    : stories([this](const std::string &text) {
        return this->storyFont ? (float) al_get_text_width(this->storyFont.get(), text.c_str()) : 0.0f;
    })
{
    ALLEGRO_MONITOR_INFO info;
    videoManager::getInstance();
    if (!al_init_image_addon()) {
        std::cout << "Could not initialize the image addon!!\n";
        exit(0);
    }

    this->alTimer.reset(al_create_timer(tickSeconds));
    this->evQueue.reset(al_create_event_queue());
    al_register_event_source(this->evQueue.get(), al_get_timer_event_source(this->alTimer.get()));
    al_get_monitor_info(0, &info);
    this->scrWidth = info.x2 - 50;
    this->scrHeight = info.y2 - 50;
}

bool presenter::initializeDisplay()
{
    al_register_event_source(this->evQueue.get(),
                             al_get_display_event_source(
                                 videoManager::getInstance().getCurrentDisplay()));
    return true;
}

void presenter::showSplash()
{
    std::cout << "Splash" << this->splashFname.c_str() << "\n";
    goe::bitmapHandle splash(al_load_bitmap(this->splashFname.c_str()));
    al_clear_to_color(al_map_rgba(15, 15, 25, 255));
    if (splash)
        al_draw_bitmap(splash.get(), 450, 0, 0);
    al_wait_for_vsync();
    al_flip_display();
}

bool presenter::loadCofiguredData()
{
    std::shared_ptr<gameConfig> gcfg = configManager::getInstance()->getConfig();
    al_init_font_addon();
    al_init_ttf_addon();
    const coords tile(gcfg->tileWidth, gcfg->tileHeight);
    this->fieldCells = coords((this->scrWidth - 2 * fieldMargin) / tile.x,
                              (this->scrHeight - 2 * fieldMargin) / tile.y - 1);
    this->fieldPx = coords(this->fieldCells.x * tile.x, this->fieldCells.y * tile.y);
    this->camera = std::make_unique<fieldCamera>(tile);
    this->hud = std::make_unique<hudView>(this->scrWidth, this->scrHeight / 3, tile, gcfg->spacing);
    if (!this->hud->loadFont(gcfg->FontFile)) {
        std::cout << "Font assets are not loaded properly, check the configuration.\n";
        return false;
    }
    // without it the game runs fine, just with no stories
    this->storyFont.reset(al_load_ttf_font(gcfg->FontFile.c_str(), storyFontSize, 0));
    this->splashFname = gcfg->splashScr;
    // one cell more than the screen all round, for the cells scrolling in
    this->view = std::make_unique<flatFieldView>(coords(this->scrWidth + tile.x, this->scrHeight + tile.y),
                                                 tile,
                                                 gcfg->spacing,
                                                 gcfg->fogBitmap);
    return true;
}

void presenter::showGameField()
{
    const std::shared_ptr<bElem> player = player::getActivePlayer();
    std::optional<coords> bounds;
    // a bounded board keeps the view inside it; on the endless world the view just follows
    if (player->getBoard() && player->getBoard()->isBounded())
        bounds = player->getBoard()->getSize();
    this->camera->follow(viewPoint::get_instance().getViewPoint(), this->fieldCells, bounds);

    al_set_target_bitmap(al_get_backbuffer(videoManager::getInstance().getCurrentDisplay()));
    al_clear_to_color(fogLayer::plainColour());
    const coords fieldAt(fieldMargin, fieldMargin / 2);
    this->hud->draw(player, fieldAt.x, fieldAt.y + this->fieldPx.y, this->fieldPx.x - 1);
    this->view->draw({player, *this->camera, this->fieldCells, fieldAt, this->fieldPx});
    this->drawStory();
    this->drawDemoNote();
    al_flip_display();
}

void presenter::drawDemoNote()
{
    if (!this->demo || !this->storyFont)
        return;
    // at the top of the game field, on a dark band so the maze does not swallow it
    const char *text = "Demo - press any key";
    const float x = (float) (fieldMargin + this->fieldPx.x / 2);
    const float y = (float) (fieldMargin / 2) + 8.0f;
    const float halfW = (float) al_get_text_width(this->storyFont.get(), text) / 2 + 12.0f;
    const float h = (float) al_get_font_line_height(this->storyFont.get());
    al_draw_filled_rectangle(x - halfW, y - 4.0f, x + halfW, y + h + 4.0f, al_map_rgba(0, 0, 0, 170));
    al_draw_text(this->storyFont.get(), al_map_rgb(255, 205, 0), x, y, ALLEGRO_ALIGN_CENTER, text);
}

void presenter::tickStories()
{
    const std::size_t made = worldBuilder::chunksGenerated();
    const bool grew = made != this->seenChunks;
    this->seenChunks = made;
    if (!gameSettings::getInstance().getStoriesShown()) {
        this->stories.stop();
        return;
    }
    if (grew)
        this->stories.chunkGenerated(goe::rng::cosmetic(), (float) this->fieldPx.x);
    this->stories.advance((float) tickSeconds);
}

void presenter::drawStory()
{
    const auto &told = this->stories.current();
    if (!told || !this->storyFont)
        return;
    // the strip above the game field, as wide as the field
    const int stripHeight = fieldMargin / 2;
    int cx, cy, cw, ch;
    al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
    al_set_clipping_rectangle(fieldMargin, 0, this->fieldPx.x, stripHeight);
    float x = (float) fieldMargin + this->stories.x();
    const float y = (float) (stripHeight - al_get_font_line_height(this->storyFont.get())) / 2;
    if (!told->title.empty()) {
        const std::string title = told->title + goe::storyScroller::separator;
        al_draw_text(this->storyFont.get(), al_map_rgb(255, 205, 0), x, y, 0, title.c_str()); // Eris' gold
        x += (float) al_get_text_width(this->storyFont.get(), title.c_str());
    }
    al_draw_text(this->storyFont.get(), al_map_rgb(255, 255, 200), x, y, 0, told->body.c_str());
    al_set_clipping_rectangle(cx, cy, cw, ch);
}

void presenter::handleSaveKeys()
{
    auto &im = inputManager::getInstance();
    if (this->pendingSaveOp == 0) {
        if (im.pressed_keys[ALLEGRO_KEY_F5] && !this->saveKeyDown)
            this->pendingSaveOp = 1;
        else if (im.pressed_keys[ALLEGRO_KEY_F9] && !this->loadKeyDown)
            this->pendingSaveOp = 2;
        if (this->pendingSaveOp != 0)
            std::cout << (this->pendingSaveOp == 1 ? "Saving" : "Loading") << "...\n";
    }
    this->saveKeyDown = im.pressed_keys[ALLEGRO_KEY_F5];
    this->loadKeyDown = im.pressed_keys[ALLEGRO_KEY_F9];
    if (this->pendingSaveOp == 0)
        return;
    bool save = this->pendingSaveOp == 1;
    const std::string saveFile = gameSettings::getInstance().getSaveFile();
    bool ok = save ? gameSerializer::saveGame(saveFile) : gameSerializer::loadGame(saveFile);
    this->pendingSaveOp = 0;
    if (save)
        std::cout << (ok ? "Game saved to " : "Saving failed: ") << saveFile << "\n";
    else
        std::cout << (ok ? "Game loaded from " : "Loading failed: ") << saveFile << "\n";
}

gameEnd presenter::presentEverything(bool demoMode, int demoSeconds)
{
    std::shared_ptr<bElem> currentPlayer = nullptr;
    ALLEGRO_EVENT event;
    controlItem cItem;
    auto result = gameEnd::QUIT;

    this->fin = false;
    this->lastScore = 0;
    this->demo = demoMode;
    this->pilot.reset();
    const auto pressesBefore = inputManager::getInstance().activity();
    // the demo's time is wall time: a slow machine runs fewer ticks, not a longer demo
    const double demoEnds = al_get_time() + demoSeconds;
    inputManager::getInstance().takeExitRequest(); // a press from the title screen does not count
    al_flush_event_queue(this->evQueue.get()); // ticks queued while the title screen was up
    // read again each game, so a file picked in Config is used; seenChunks is kept, so the chunks
    // a new game was built with tell the first story, and a loaded game waits for the maze to grow
    this->stories.setStories(goe::loadStories(gameSettings::getInstance().getStoriesFile()));
    this->stories.stop();
    al_start_timer(this->alTimer.get());
    while (!this->fin) {
        al_wait_for_event(this->evQueue.get(), &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            this->fin = true;
            break;
        }
        if (event.type == ALLEGRO_EVENT_TIMER) {
            std::lock_guard<std::mutex> guard(this->presenter_mutex);
            if (this->demo) {
                // any press ends the demo; nothing is saved or loaded while it runs
                inputManager::getInstance().takeExitRequest();
                if (inputManager::getInstance().activity() != pressesBefore) {
                    result = gameEnd::DEMO_OVER;
                    this->fin = true;
                    break;
                }
                if (demoSeconds > 0 && al_get_time() >= demoEnds) {
                    result = gameEnd::DEMO_DONE;
                    this->fin = true;
                    break;
                }
                inputManager::getInstance().setControlItem(this->pilot.decide(player::getActivePlayer()));
            } else
                this->handleSaveKeys();
            if (!this->demo && inputManager::getInstance().takeExitRequest() && player::getActivePlayer()) {
                // between ticks, so the save holds one consistent moment of the world
                const std::string saveFile = gameSettings::getInstance().getSaveFile();
                const bool saved = gameSerializer::saveGame(saveFile);
                std::cout << (saved ? "Game saved to " : "Saving failed: ") << saveFile << "\n";
                result = saved ? gameEnd::SAVED : gameEnd::SAVE_FAILED;
                this->fin = true;
                break;
            }
            currentPlayer = player::getActivePlayer();
            if (currentPlayer.get() != nullptr) {
                this->lastScore = currentPlayer->getStats()->getPoints(TOTAL);
                // the maze grows ahead of the player and far chunks go to disk, one chunk per tick
                const coords at = currentPlayer->getStats()->getMyPosition();
                if (!worldBuilder::growAround(currentPlayer->getBoard(), at))
                    worldBuilder::shrinkAround(currentPlayer->getBoard(), at);
                soundManager::getInstance().followDifficulty(difficulty::of(currentPlayer));
                soundManager::getInstance().followSituation(goe::music::cues::now());
            }
            bElem::runLiveElements();
            this->tickStories();
            if (player::getActivePlayer().get() != nullptr)
                this->showGameField();
            else {
                // the last avatar is gone; the best of all avatars of this game is what counts
                this->lastScore = player::bestScore();
                std::cout << "Game over, best score " << this->lastScore << "\n";
                result = gameEnd::LOST;
                this->fin = true;
            }
        } else {
            if (player::getActivePlayer().get() == nullptr) {
                this->lastScore = player::bestScore();
                result = gameEnd::LOST;
                this->fin = true;
                break;
            }
            cItem = inputManager::getInstance()
                        .getCtrlItem(); //We always got a status on what to do. remember, everything must have a timer!
            // the idea is to serve the keyboard state constantly, we avoid actions that are too fast
            // by having timers on everything, like: once you shoot, you will be able to shoot in some defined time
            // same with movement, object cycling, gun cycling, using things, interacting with things.
            if (cItem.type == 7 && !this->demo) {
                this->fin = true;
                break;
            }
        }
    }
    al_stop_timer(this->alTimer.get());
    if (this->demo)
        inputManager::getInstance().setControlItem(controlItem(-1, dir::direction::NODIRECTION));
    this->demo = false;
    if (result == gameEnd::QUIT)
        inputManager::getInstance().stop();
    return result;
}

int presenter::getLastScore() const
{
    return this->lastScore;
}

} // namespace presenter
