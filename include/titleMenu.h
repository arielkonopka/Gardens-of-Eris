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

#ifndef TITLEMENU_H
#define TITLEMENU_H

#include "gameSettings.h"
#include <functional>
#include <string>
#include <vector>

/**
 * @brief What the title screen shows and how it reacts to keys, without any drawing.
 *
 * titleScreen draws it and feeds it Allegro key codes and typed characters.
 */
class titleMenu
{
public:
    /// CONTROLS lists the key layout; BINDING waits for the key or pad button of one action
    enum class screen { MAIN, CONFIG, EDITING, CONTROLS, BINDING };
    enum class action { NONE, CONTINUE, START, EXIT };

    /// one line of the config screen: a label, its current value, and how to change it
    struct option
    {
        std::string label;
        std::function<std::string()> value;
        /// takes a typed value; empty for a switch (Enter calls adjust) or the line that opens the controls screen
        std::function<bool(const std::string &)> apply;
        /// Left and Right step the value (-1 or +1); empty when they do nothing
        std::function<void(int)> adjust;
    };

    /// saveReadable says whether there is a save to continue; without it, Continue is never offered
    titleMenu(gameSettings &settings,
              std::string settingsFile = gameSettings::settingsFile,
              std::function<bool()> saveReadable = {});

    /// back to the main menu, asking again whether there is a save; Continue is selected when there is
    void refresh();
    /// menus take repeated key presses (KEY_CHAR); on the BINDING screen send key downs,
    /// so that Shift, Ctrl and Alt can be bound too
    action keyDown(int keycode);
    /// a pad button, used on the BINDING screen
    void padButton(int button);
    /// a typed character (Unicode code point), used while editing a value
    void typed(int codepoint);

    screen getScreen() const;
    int getSelected() const;
    /// the lines of the current screen, in order; the selected one is getSelected()
    std::vector<std::string> getLines() const;
    /// the name of the current screen ("Config", "Controls"); empty on the main menu
    std::string getTitle() const;
    const std::string &getEditBuffer() const;
    /// feedback after applying a value, empty when there is nothing to say
    const std::string &getMessage() const;

private:
    enum class mainItem { CONTINUE, START, CONFIG, EXIT };
    /// the main menu's lines; Continue only while a save can be read
    std::vector<mainItem> mainItems() const;
    static std::string labelOf(mainItem item);
    void move(int by);
    int lineCount() const;
    void applyEdit();
    void configKey(int keycode);
    void controlsKey(int keycode);
    void bindingKey(int keycode);
    /// stores the settings and says whether that worked
    void saveSettings();
    /// the option that opens the controls screen
    int controlsLine() const;

    gameSettings &settings;
    std::string settingsFile;
    std::function<bool()> saveReadable;
    bool canContinue = false;
    std::vector<option> options;
    screen current = screen::MAIN;
    int selected = 0;
    std::string editBuffer;
    std::string message;
};

#endif // TITLEMENU_H
