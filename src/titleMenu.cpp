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

#include "titleMenu.h"
#include <allegro5/keycodes.h>

namespace {
void appendUtf8(std::string &s, int cp)
{
    if (cp < 0x80) {
        s += (char) cp;
    } else if (cp < 0x800) {
        s += (char) (0xC0 | (cp >> 6));
        s += (char) (0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        s += (char) (0xE0 | (cp >> 12));
        s += (char) (0x80 | ((cp >> 6) & 0x3F));
        s += (char) (0x80 | (cp & 0x3F));
    } else if (cp < 0x110000) {
        s += (char) (0xF0 | (cp >> 18));
        s += (char) (0x80 | ((cp >> 12) & 0x3F));
        s += (char) (0x80 | ((cp >> 6) & 0x3F));
        s += (char) (0x80 | (cp & 0x3F));
    }
}

void popUtf8(std::string &s)
{
    // drop continuation bytes, then the lead byte
    while (!s.empty() && (s.back() & 0xC0) == 0x80)
        s.pop_back();
    if (!s.empty())
        s.pop_back();
}
} // namespace

titleMenu::titleMenu(gameSettings &settings, std::string settingsFile)
    : settings(settings)
    , settingsFile(std::move(settingsFile))
{
    this->options.push_back({"Save location",
                             [this] { return this->settings.getSaveDirectory(); },
                             [this](const std::string &v) {
                                 return this->settings.setSaveDirectory(v);
                             }});
}

titleMenu::action titleMenu::keyDown(int keycode)
{
    switch (this->current) {
    case screen::MAIN:
        if (keycode == ALLEGRO_KEY_UP)
            this->move(-1);
        else if (keycode == ALLEGRO_KEY_DOWN)
            this->move(1);
        else if (keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER
                 || keycode == ALLEGRO_KEY_SPACE) {
            if (this->selected == 0)
                return action::START;
            if (this->selected == 2)
                return action::EXIT;
            this->current = screen::CONFIG;
            this->selected = 0;
            this->message.clear();
        }
        break;
    case screen::CONFIG:
        if (keycode == ALLEGRO_KEY_UP)
            this->move(-1);
        else if (keycode == ALLEGRO_KEY_DOWN)
            this->move(1);
        else if (keycode == ALLEGRO_KEY_ESCAPE) {
            this->current = screen::MAIN;
            this->selected = 1;
        } else if (keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER
                   || keycode == ALLEGRO_KEY_SPACE) {
            if (this->selected == (int) this->options.size()) { // "Back"
                this->current = screen::MAIN;
                this->selected = 1;
            } else {
                this->editBuffer = this->options[this->selected].value();
                this->current = screen::EDITING;
                this->message.clear();
            }
        }
        break;
    case screen::EDITING:
        if (keycode == ALLEGRO_KEY_ESCAPE) {
            this->current = screen::CONFIG;
            this->message.clear();
        } else if (keycode == ALLEGRO_KEY_BACKSPACE) {
            popUtf8(this->editBuffer);
        } else if (keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER) {
            this->applyEdit();
        }
        break;
    }
    return action::NONE;
}

void titleMenu::typed(int codepoint)
{
    if (this->current != screen::EDITING || codepoint < 32 || codepoint == 127)
        return;
    appendUtf8(this->editBuffer, codepoint);
}

void titleMenu::applyEdit()
{
    auto &opt = this->options[this->selected];
    if (!opt.apply(this->editBuffer)) {
        this->message = "Cannot use \"" + this->editBuffer + "\"";
        return; // stay in the editor so the player can correct it
    }
    this->message = this->settings.save(this->settingsFile) ? "Saved" : "Changed, but settings file could not be written";
    this->current = screen::CONFIG;
}

void titleMenu::move(int by)
{
    int n = this->lineCount();
    this->selected = (this->selected + by + n) % n;
}

int titleMenu::lineCount() const
{
    return this->current == screen::MAIN ? (int) this->mainItems.size() : (int) this->options.size() + 1;
}

titleMenu::screen titleMenu::getScreen() const
{
    return this->current;
}

int titleMenu::getSelected() const
{
    return this->selected;
}

std::vector<std::string> titleMenu::getLines() const
{
    if (this->current == screen::MAIN)
        return this->mainItems;
    std::vector<std::string> lines;
    for (int c = 0; c < (int) this->options.size(); c++) {
        const auto &o = this->options[c];
        bool editing = this->current == screen::EDITING && c == this->selected;
        lines.push_back(o.label + ": " + (editing ? this->editBuffer + "_" : o.value()));
    }
    lines.push_back("Back");
    return lines;
}

const std::string &titleMenu::getEditBuffer() const
{
    return this->editBuffer;
}

const std::string &titleMenu::getMessage() const
{
    return this->message;
}
