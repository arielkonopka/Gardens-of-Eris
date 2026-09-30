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
#include "worldBuilder.h"
#include "elementView.h"
#include "difficulty.h"

namespace presenter {
presenter::presenter()
    : sWidth(0)
    , sHeight(0)
    , spacing(0)
    , previousPosition({0, 0})
    , positionOnScreen({0, 0})
{
    ALLEGRO_MONITOR_INFO info;
    videoManager::getInstance();
    if (!al_init_image_addon()) {
        std::cout << "Could not initialize the image addon!!\n";
        exit(0);
    }

    this->alTimer.reset(al_create_timer(1.0 / 50));
    this->evQueue.reset(al_create_event_queue());
    al_register_event_source(this->evQueue.get(), al_get_timer_event_source(this->alTimer.get()));
    //  this->_cp_attachedBoard=board;
    al_get_monitor_info(0, &info);
    this->scrWidth = info.x2 - 50;  /* Assume this is 1366 */
    this->scrHeight = info.y2 - 50; /* Assume this is 768 */
    this->chaosGameTops.push_back({0, 0});
    this->chaosGameTops.push_back({this->scrWidth / 2, this->scrHeight / 2});
    this->chaosGameTops.push_back({this->scrWidth, 0});
    this->chaosGameTops.push_back({this->scrWidth, this->scrHeight});
    this->chaosGameTops.push_back({0, this->scrHeight});
    this->chaosGamelastPoint = {1 + this->scrWidth / 2, 1 - this->scrHeight / 2};
}

bool presenter::initializeDisplay()
{
    al_register_event_source(this->evQueue.get(),
                             al_get_display_event_source(
                                 videoManager::getInstance().getCurrentDisplay()));
    this->internalBitmap.reset(al_create_bitmap(this->scrWidth + 64, this->scrHeight + 64));
    this->statsStripe.reset(al_create_bitmap(this->scrWidth, this->scrHeight / 3));
    return true;
}

bool presenter::presentAChamber(presenterMode mod)
{
    switch (mod) {
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
    this->myfont.reset(al_load_ttf_font(gcfg->FontFile.c_str(), 32, 0));
    if (!this->myfont) {
        std::cout << "Font assets are not loaded properly, check the configuration.\n";
        return false;
    }
    this->bluredElement = gcfg->bluredElement;

    this->splashFname = gcfg->splashScr;
    this->sWidth = gcfg->tileWidth;
    this->sHeight = gcfg->tileHeight;
    this->spacing = gcfg->spacing;
    this->scrTilesX = (this->scrWidth - (2 * _offsetX)) / this->sWidth;
    this->scrTilesY = ((this->scrHeight - (2 * _offsetY)) / this->sHeight) - 1;
    this->bsWidth = this->scrTilesX * this->sWidth;
    this->bsHeight = this->scrTilesY * this->sHeight;
    this->fog.setup(al_get_bitmap_width(this->internalBitmap.get()),
                    al_get_bitmap_height(this->internalBitmap.get()),
                    coords(this->sWidth, this->sHeight),
                    gcfg->fogBitmap);
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
bool presenter::showObjectTile(
    int x, int y, int offsetX, int offsetY, std::shared_ptr<bElem> elem, bool ignoreOffset, int mode)
{
    if (!elem)
        return false;
    coords coords, offset = {0, 0};
    auto ve = videoDriver::getInstance().getVideoElement(elem->getType());
    if (!ve)
        return false;
    auto draw_sprite = [&]() {
        int sx = (coords.x * this->sWidth) + ((coords.x + 1) * (this->spacing));
        int sy = (coords.y * this->sHeight) + ((coords.y + 1) * (this->spacing));

        if (ve)
            al_draw_bitmap_region(ve->sprites.get(),
                                  sx,
                                  sy,
                                  this->sWidth,
                                  this->sHeight,
                                  offsetX + (x * this->sWidth),
                                  offsetY + (y * this->sHeight),
                                  0);
    };
    //m int sx,sy;
    bool res = false;
    if (x > this->scrTilesX + 20 || y > this->scrTilesY + 20 || elem.get() == nullptr || !ve)
        return false;

    if (!ignoreOffset) {
        if (elem.get() != nullptr)
            offset = elementView::offset(*elem);
        offsetX = offset.x;
        offsetY = offset.y;
    }
    if (mode == 1 && !elem->getStats()->isMoving())
        return false;

    if (elem->getStats()->getSteppingOn() && mode != _mode_onlyTop)
        res = this->showObjectTile(x,
                                   y,
                                   offsetX,
                                   offsetY,
                                   elem->getStats()->getSteppingOn(),
                                   ignoreOffset,
                                   mode);
    if ((mode == 0 && elem->getStats()->isMoving()))
        return true;
    int sType = elem->getAttrs()->getSubtype() % ve->defArray.size();
    int sDir = ((int) elem->getStats()->getFacing()) % ve->defArray[sType].size();
    int sPh = elem->getAnimPh() % ve->defArray[sType][sDir].size();
    if (elem->getType() == bElemTypes::_floorType
        || (!elem->getStats()->isDying() && !elem->getStats()->isDestroying()
            && !elem->getStats()->isTeleporting())) {
        coords = ve->defArray[sType][sDir][sPh];
        draw_sprite();
        if (elem->getType() != bElemTypes::_floorType)
            return res;
    }
    if (elem->getStats()->isDying()) {
        coords = ve->dying[elem->getAnimPh() % (ve->dying.size())];
        draw_sprite();
        return res;
    }
    if (elem->getStats()->isDestroying()) {
        coords = ve->destroying[elem->getAnimPh() % (ve->destroying.size())];
        draw_sprite();
        return res;
    }

    if (elem->getStats()->isFadingOut()) {
        coords = ve->fadingOut[elem->getAnimPh() % (ve->fadingOut.size())];
        draw_sprite();
        return res;
    }
    if (elem->getStats()->isFadingIn()) {
        coords = ve->fadingIn[elem->getAnimPh() % (ve->fadingIn.size())];
        draw_sprite();
        return res;
    }
    if (elem->getStats()->isTeleporting()) {
        coords = ve->teleporting[elem->getAnimPh() % (ve->teleporting.size())];
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
void presenter::showText(int x, int y, int offsetX, int offsetY, std::string text)
{
    ALLEGRO_COLOR c = al_map_rgb(255, 255, 200);
    int scrx = offsetX + (x * this->sWidth), scry = offsetY + (y * this->sHeight);
    if (this->myfont) {
        al_draw_text(this->myfont.get(), c, (float) scrx, (float) scry, 0, text.c_str());
    }
}

void presenter::prepareStatsThing()
{
    std::shared_ptr<bElem> aPlayer = player::getActivePlayer();
    al_set_target_bitmap(this->statsStripe.get());
    al_clear_to_color(al_map_rgba(0, 0, 0, 255));
    this->showText(1, 1, 0, 5, "Garden: " + aPlayer->getBoard()->getName());
    this->showObjectTile(1, 0, 0, 0, aPlayer, true, _mode_onlyTop);
    this->showText(2, 0, 0, 0, std::to_string(player::countVisitedPlayers()));
    this->showText(2, 0, 0, 32, std::to_string(aPlayer->getAttrs()->getEnergy()));
    this->showObjectTile(4,
                         0,
                         0,
                         0,
                         aPlayer->getAttrs()->getInventory()->getActiveWeapon(),
                         true,
                         _mode_onlyTop);
    this->showObjectTile(6,
                         0,
                         0,
                         0,
                         aPlayer->getAttrs()->getInventory()->getUsable(),
                         true,
                         _mode_onlyTop);

    if (aPlayer->getAttrs()->getInventory()->getActiveWeapon() != nullptr) {
        // how many weapons of this kind are held
        this->showText(4,
                       0,
                       0,
                       32,
                       "x" + std::to_string(aPlayer->getAttrs()->getInventory()->countActiveWeaponKind()));
        this->showText(
            5,
            0,
            0,
            32,
            std::to_string(
                aPlayer->getAttrs()->getInventory()->getActiveWeapon()->getAttrs()->getEnergy()));
        this->showText(
            5,
            0,
            0,
            0,
            std::to_string(
                aPlayer->getAttrs()->getInventory()->getActiveWeapon()->getAttrs()->getAmmo()));
    }
    if (aPlayer->getAttrs()->getInventory()->getUsable()) {
        this->showText(7,
                       0,
                       0,
                       32,
                       std::to_string(
                           aPlayer->getAttrs()->getInventory()->getUsable()->getAttrs()->getEnergy()));
        this->showText(
            7,
            0,
            0,
            0,
            std::to_string(aPlayer->getAttrs()->getInventory()->countTokens(
                aPlayer->getAttrs()->getInventory()->getUsable()->getType(),
                aPlayer->getAttrs()->getInventory()->getUsable()->getAttrs()->getSubtype())));
    }

    for (int cnt = 0; cnt < 5; cnt++) {
        int tokens;
        std::shared_ptr<bElem> key = aPlayer->getAttrs()->getInventory()->getKey(bElemTypes::_key,
                                                                                 cnt,
                                                                                 false);
        if (key != nullptr) {
            tokens = aPlayer->getAttrs()->getInventory()->countTokens(key->getType(),
                                                                      key->getAttrs()->getSubtype());
            this->showObjectTile(8 + (cnt * 2), 0, 0, 0, key, true, _mode_onlyTop);
            this->showText(9 + (cnt * 2), 0, 0, 16, std::to_string(tokens));
        }
    }
    this->showObjectTile(18, 0, 0, 0, goldenApple::getApple(0), true, _mode_onlyTop);
    this->showText(19, 0, 0, 0, std::to_string(goldenApple::getAppleNumber()));
    this->showText(19,
                   0,
                   0,
                   32,
                   std::to_string(
                       aPlayer->getAttrs()->getInventory()->countTokens(bElemTypes::_goldenAppleType,
                                                                        0)));
    this->showText(21, 0, 6, 0, "Stats");
    this->showText(21, 0, 5, 32, "P:");
    this->showText(21, 1, 5, 0, "Dex:");
    this->showText(22, 0, 5, 32, std::to_string(aPlayer->getStats()->getPoints(TOTAL)));
    this->showText(22, 1, 5, 0, std::to_string(difficulty::playerLevel(aPlayer)));
    this->showText(21, 1, 5, 32, "D:");
    this->showText(22, 1, 5, 32, std::to_string(difficulty::of(aPlayer)));
}

/* We kinda move a window in a big screen, that is whole board.We track the upper left point, which is placed in previousPosition
 */
void presenter::showGameField()
{
    int x, y;
    coords d;
    coords halfscreen = coords((this->scrTilesX) / 2, (this->scrTilesY) / 2);
    int offX = 0, offY = 0;
    std::vector<movingSprite> mSprites;
    std::shared_ptr<bElem> player = player::getActivePlayer();
    // Calculate LeftUpper corner of the viewpoint
    // BEGIN:upperLeft
    coords b = viewPoint::get_instance().getViewPoint() - halfscreen;
    // a bounded board keeps the view inside it; on the endless world the view just follows
    if (player && player->getBoard() && player->getBoard()->isBounded()) {
        const coords boardsize = player->getBoard()->getSize();
        b.x = std::max(0, std::min(boardsize.x - (this->scrTilesX), b.x));
        b.y = std::max(0, std::min(boardsize.y - (this->scrTilesY), b.y));
    }
    // END:upperLeft
    d = b - this->previousPosition;
    // rounded down, so the view scrolls the same way where the endless world goes negative
    if (d.x == 0 && floorMod(this->positionOnScreen.x, this->sWidth) > 0)
        d.x = -1;
    if (d.y == 0 && floorMod(this->positionOnScreen.y, this->sHeight) > 0)
        d.y = -1;
    this->positionOnScreen = this->positionOnScreen + (d * 8);
    this->previousPosition.x = floorDiv(this->positionOnScreen.x, this->sWidth);
    this->previousPosition.y = floorDiv(this->positionOnScreen.y, this->sHeight);
    offX = floorMod(this->positionOnScreen.x, this->sWidth);
    offY = floorMod(this->positionOnScreen.y, this->sHeight);
    this->prepareStatsThing();

    al_set_target_bitmap(this->internalBitmap.get());

    colour c = this->_cp_attachedBoard->getChColour();
    al_clear_to_color(al_map_rgba(c.r, c.g, c.b, c.a));
    /***
    draw only visible elements, walls are always visible.
    ***/
    this->poses.clear();
    if (player->getBoard()) {
        for (x = 0; x < this->scrTilesX + 1; x++)
            for (y = 0; y < this->scrTilesY + 1; y++) {
                coords np = coords(x + this->previousPosition.x, y + this->previousPosition.y);
                std::shared_ptr<bElem> elemToDisplay = player->getBoard()->getElement(np);
                if (player->getBoard()->isVisible(np) >= 255
                    && !viewPoint::get_instance().isPointVisible(np))
                    continue; // this element is not even discovered yet
                if (elemToDisplay) {
                    if (this->showObjectTile(x, y, 0, 0, elemToDisplay, false, 0))
                        mSprites.push_back({x, y, elemToDisplay});
                    continue;
                }
            }
    }
    int px = 0, py = 0;
    /***
    draw elements on the move, if the element is steppable and has a still element on it, then drawing will be broken.
    ***/
    for (unsigned int cnt = 0; cnt < mSprites.size(); cnt++) {
        movingSprite ms = mSprites.at(cnt);

        if (ms.elem->getStats()->getInstanceId() == player->getStats()->getInstanceId()) {
            px = ms.x;
            py = ms.y;

        } else
            this->showObjectTile(ms.x, ms.y, 0, 0, ms.elem, false, 1);
    }
    if (player->getStats()->isMoving() && player->getBoard()
        && player->getBoard()->getElement(viewPoint::get_instance().getViewPoint()))
        this->showObjectTile(px,
                             py,
                             0,
                             0,
                             player->getBoard()->getElement(player->getStats()->getMyPosition()),
                             false,
                             1);
    /***
    Draw the cloak on the game field
    ***/

    al_set_target_bitmap(al_get_backbuffer(videoManager::getInstance().getCurrentDisplay()));

    al_clear_to_color(fogLayer::plainColour());
    al_draw_bitmap_region(this->statsStripe.get(),
                          0,
                          0,
                          this->bsWidth - 1,
                          128,
                          _offsetX,
                          this->bsHeight + (_offsetY / 2),
                          0);
    const auto points = viewPoint::get_instance()
                            .getViewPoints(this->previousPosition,
                                           this->previousPosition
                                               + coords(this->scrTilesX + 1, this->scrTilesY + 1));
    this->fog.draw(this->internalBitmap.get(),
                   points,
                   coords(this->previousPosition.x * this->sWidth,
                          this->previousPosition.y * this->sHeight),
                   offX,
                   offY,
                   this->bsWidth,
                   this->bsHeight,
                   _offsetX,
                   _offsetY / 2);
    al_flip_display();
}

void presenter::drawCloak()
{
    if (viewPoint::get_instance().getViewPoint() != NOCOORDS) {
        auto ve = videoDriver::getInstance().getVideoElement(player::getActivePlayer()->getType());
        int obscured;
        int divider = GoEConstants::_dividerCloak;
        coords be = this->bluredElement[player::getActivePlayer()->getBoard()->getInstanceId()
                                        % this->bluredElement.size()];
        for (int x = -1; x < this->scrTilesX + 2; x++)
            for (int y = -1; y < this->scrTilesY + 2; y++) {
                int nx = x + this->previousPosition.x;
                int ny = y + this->previousPosition.y;
                coords np = coords(nx, ny);
                if (viewPoint::get_instance().isPointVisible(np)) {
                    for (int x1 = 0; x1 < divider; x1++)
                        for (int y1 = 0; y1 < divider; y1++) {
                            coords np1 = (np * divider) + coords(x1, y1);
                            obscured = std::min(255,
                                                viewPoint::get_instance()
                                                    .calculateObscured(np1, divider));
                            if (obscured > 0) {
                                int sx = (be.x * this->sWidth) + ((be.x + 1) * (this->spacing))
                                         + (x1 * this->sWidth) / divider;
                                int sy = (be.y * this->sHeight) + ((be.y + 1) * (this->spacing))
                                         + (y1 * this->sHeight) / divider;
                                al_draw_tinted_bitmap_region(ve->sprites.get(),
                                                             al_map_rgba(255, 255, 255, obscured),
                                                             sx,
                                                             sy,
                                                             this->sWidth / divider,
                                                             this->sHeight / divider,
                                                             ((x + 0) * this->sWidth)
                                                                 + (x1 * this->sWidth) / divider,
                                                             ((y + 0) * this->sHeight)
                                                                 + (y1 * this->sHeight) / divider,
                                                             0);
                            };
                        }
                    continue;
                }
                int sx = (be.x * this->sWidth) + ((be.x + 1) * (this->spacing));
                int sy = (be.y * this->sHeight) + ((be.y + 1) * (this->spacing));
                al_draw_bitmap_region(ve->sprites.get(),
                                      sx,
                                      sy,
                                      this->sWidth,
                                      this->sHeight,
                                      ((x + 0) * this->sWidth),
                                      ((y + 0) * this->sHeight),
                                      0);
            }
    }
}

void presenter::eyeCandy(int flavour)
{
    int top;
    coords np;
    if (flavour == 0) {
        for (int x = 0; x < 1000; x++) {
            np = {(int) (goe::rng::cosmetic()() % this->scrWidth),
                  (int) (goe::rng::cosmetic()() % this->scrHeight)};
            al_draw_pixel(np.x, np.y, al_map_rgba(255, 0, 255, goe::rng::cosmetic()() % 10));
        }
    }
    if (flavour == 2) {
        for (int c = 0; c < 1000; c++) {
            top = goe::rng::below(goe::rng::cosmetic(), this->chaosGameTops.size());
            np = {(this->chaosGameTops[top].x + this->chaosGamelastPoint.x) / 2,
                  (this->chaosGameTops[top].y + this->chaosGamelastPoint.y) / 2};
            this->chaosGamePoints.push_back(np);
            this->chaosGamelastPoint = np;
            if (this->chaosGamePoints.size() > 2000)
                this->chaosGamePoints.erase(this->chaosGamePoints.begin());
        }

        for (unsigned int c = 0; c < this->chaosGameTops.size(); c++) {
            al_draw_pixel(this->chaosGameTops[c].x,
                          this->chaosGameTops[c].y,
                          al_map_rgba(255, 0, 255, 10));
        }

        for (unsigned int c = 0; c < this->chaosGamePoints.size(); c++) {
            al_draw_pixel(this->chaosGamePoints[c].x,
                          this->chaosGamePoints[c].y,
                          al_map_rgba(255, 255, 255, 5));
        }
        return;
    }
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

gameEnd presenter::presentEverything()
{
    std::shared_ptr<bElem> currentPlayer = nullptr;
    ALLEGRO_EVENT event;
    controlItem cItem;
    auto result = gameEnd::QUIT;

    this->fin = false;
    this->lastScore = 0;
    al_flush_event_queue(this->evQueue.get()); // ticks queued while the title screen was up
    al_start_timer(this->alTimer.get());
    while (!this->fin) {
        al_wait_for_event(this->evQueue.get(), &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            this->fin = true;
            break;
        }
        if (event.type == ALLEGRO_EVENT_TIMER) {
            std::lock_guard<std::mutex> guard(this->presenter_mutex);
            this->handleSaveKeys();
            currentPlayer = player::getActivePlayer();
            if (currentPlayer.get() != nullptr) {
                this->_cp_attachedBoard = currentPlayer->getBoard();
                this->lastScore = currentPlayer->getStats()->getPoints(TOTAL);
                // the maze grows ahead of the player and far chunks go to disk, one chunk per tick
                const coords at = currentPlayer->getStats()->getMyPosition();
                if (!worldBuilder::growAround(currentPlayer->getBoard(), at))
                    worldBuilder::shrinkAround(currentPlayer->getBoard(), at);
            }
            bElem::runLiveElements();
            if (player::getActivePlayer().get() != nullptr)
                this->showGameField();
            else {
                // the last avatar is gone
                std::cout << "Game over, score " << this->lastScore << "\n";
                result = gameEnd::LOST;
                this->fin = true;
            }
        } else {
            if (player::getActivePlayer().get() == nullptr) {
                result = gameEnd::LOST;
                this->fin = true;
                break;
            }
            cItem = inputManager::getInstance()
                        .getCtrlItem(); //We always got a status on what to do. remember, everything must have a timer!
            // the idea is to serve the keyboard state constantly, we avoid actions that are too fast
            // by having timers on everything, like: once you shoot, you will be able to shoot in some defined time
            // same with movement, object cycling, gun cycling, using things, interacting with things.
            if (cItem.type == 7) {
                this->fin = true;
                break;
            }
        }
    }
    al_stop_timer(this->alTimer.get());
    if (result == gameEnd::QUIT)
        inputManager::getInstance().stop();
    return result;
}

int presenter::getLastScore() const
{
    return this->lastScore;
}

} // namespace namespace
