/*
 * Copyright (c) 2026, Ariel Konopka
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

#ifndef TITLESCREEN_H
#define TITLESCREEN_H

#include "titleMenu.h"
#include "allegroHandles.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <string>
#include <vector>

/**
 * @brief Draws the title menu and runs it until the player starts the game or leaves.
 */
class titleScreen
{
public:
    explicit titleScreen(titleMenu &menu);
    /// shows the main menu and blocks until the player picks Continue, Start or Exit
    /// (closing the window counts as Exit), or nobody pressed anything for the menu's demo wait (Demo)
    titleMenu::action run();
    /// clears the menu and shows one line, for example while the first level is being built
    void showBusy(const std::string &text);
    /// shows a headline and a few lines until the player presses Enter, Space or Esc;
    /// returns false when the window was closed instead
    bool showMessage(const std::string &headline, const std::vector<std::string> &lines);

private:
    void draw();

    titleMenu &menu;
    goe::fontHandle bigFont;
    goe::fontHandle font;
    goe::bitmapHandle splash;
    goe::timerHandle timer;
    goe::eventQueueHandle queue; // declared last so it is destroyed before the timer it listens to
};

#endif // TITLESCREEN_H
