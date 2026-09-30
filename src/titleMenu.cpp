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
#include <exception>
#include <string>

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

titleMenu::titleMenu(gameSettings &edited, std::string file)
    : settings(edited)
    , settingsFile(std::move(file))
{
    this->options.push_back({"Save location",
                             [this] { return this->settings.getSaveDirectory(); },
                             [this](const std::string &v) {
                                 return this->settings.setSaveDirectory(v);
                             },
                             {}});
    // a volume line: shows "80%", takes a typed number, and Left/Right step it by five
    auto volume = [](std::string label, std::function<int()> get, std::function<void(int)> set) {
        return option{std::move(label),
                      [get] { return std::to_string(get()) + "%"; },
                      [set](const std::string &v) {
                          try {
                              size_t used = 0;
                              int percent = std::stoi(v, &used);
                              if (percent < 0 || percent > 100 || (used < v.size() && v.substr(used) != "%"))
                                  return false;
                              set(percent);
                              return true;
                          } catch (const std::exception &) {
                              return false;
                          }
                      },
                      [get, set](int by) { set(get() + by * gameSettings::volumeStep); }};
    };
    this->options.push_back(volume(
        "Music volume",
        [this] { return this->settings.getMusicVolume(); },
        [this](int v) { this->settings.setMusicVolume(v); }));
    this->options.push_back(volume(
        "Sound effects volume",
        [this] { return this->settings.getEffectsVolume(); },
        [this](int v) { this->settings.setEffectsVolume(v); }));
    this->options.push_back({"Controls", [] { return std::string(); }, {}, {}});
}

int titleMenu::controlsLine() const
{
    return (int) this->options.size() - 1;
}

titleMenu::action titleMenu::keyDown(int keycode)
{
    const bool enter = keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER
                       || keycode == ALLEGRO_KEY_SPACE;
    switch (this->current) {
    case screen::MAIN:
        if (keycode == ALLEGRO_KEY_UP)
            this->move(-1);
        else if (keycode == ALLEGRO_KEY_DOWN)
            this->move(1);
        else if (enter) {
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
        this->configKey(keycode);
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
    case screen::CONTROLS:
        this->controlsKey(keycode);
        break;
    case screen::BINDING:
        this->bindingKey(keycode);
        break;
    }
    return action::NONE;
}

void titleMenu::configKey(int keycode)
{
    const bool enter = keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER
                       || keycode == ALLEGRO_KEY_SPACE;
    const bool onOption = this->selected < (int) this->options.size();
    if (keycode == ALLEGRO_KEY_UP) {
        this->move(-1);
    } else if (keycode == ALLEGRO_KEY_DOWN) {
        this->move(1);
    } else if ((keycode == ALLEGRO_KEY_LEFT || keycode == ALLEGRO_KEY_RIGHT) && onOption
               && this->options[this->selected].adjust) {
        this->options[this->selected].adjust(keycode == ALLEGRO_KEY_LEFT ? -1 : 1);
        this->saveSettings();
    } else if (keycode == ALLEGRO_KEY_ESCAPE || (enter && !onOption)) { // Esc or "Back"
        this->current = screen::MAIN;
        this->selected = 1;
    } else if (enter) {
        this->message.clear();
        if (this->selected == this->controlsLine()) {
            this->current = screen::CONTROLS;
            this->selected = 0;
        } else {
            this->editBuffer = this->options[this->selected].value();
            this->current = screen::EDITING;
        }
    }
}

void titleMenu::controlsKey(int keycode)
{
    const int reset = goe::controls::actionCount, back = reset + 1;
    const bool enter = keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER
                       || keycode == ALLEGRO_KEY_SPACE;
    if (keycode == ALLEGRO_KEY_UP) {
        this->move(-1);
    } else if (keycode == ALLEGRO_KEY_DOWN) {
        this->move(1);
    } else if (keycode == ALLEGRO_KEY_ESCAPE || (enter && this->selected == back)) {
        this->current = screen::CONFIG;
        this->selected = this->controlsLine();
        this->message.clear();
    } else if (enter && this->selected == reset) {
        this->settings.setControls({});
        this->saveSettings();
    } else if (enter) {
        this->current = screen::BINDING;
        this->message.clear();
    }
}

void titleMenu::bindingKey(int keycode)
{
    auto what = (goe::controls::action) this->selected;
    auto controls = this->settings.getControls();
    if (keycode == ALLEGRO_KEY_ESCAPE) { // Esc cancels, so it can only be bound back by a reset
        this->current = screen::CONTROLS;
        return;
    }
    if (keycode == ALLEGRO_KEY_BACKSPACE || keycode == ALLEGRO_KEY_DELETE)
        controls.clear(what);
    else
        controls.bindKey(what, keycode);
    this->settings.setControls(controls);
    this->current = screen::CONTROLS;
    this->saveSettings();
}

void titleMenu::padButton(int button)
{
    if (this->current != screen::BINDING)
        return;
    auto controls = this->settings.getControls();
    controls.bindPadButton((goe::controls::action) this->selected, button);
    this->settings.setControls(controls);
    this->current = screen::CONTROLS;
    this->saveSettings();
}

void titleMenu::saveSettings()
{
    this->message = this->settings.save(this->settingsFile) ? "Saved" : "Changed, but settings file could not be written";
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
    this->saveSettings();
    this->current = screen::CONFIG;
}

void titleMenu::move(int by)
{
    int n = this->lineCount();
    this->selected = (this->selected + by + n) % n;
}

int titleMenu::lineCount() const
{
    switch (this->current) {
    case screen::MAIN:
        return (int) this->mainItems.size();
    case screen::CONTROLS:
    case screen::BINDING:
        return goe::controls::actionCount + 2; // "Reset to defaults" and "Back"
    default:
        return (int) this->options.size() + 1;
    }
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
    if (this->current == screen::CONTROLS || this->current == screen::BINDING) {
        const auto controls = this->settings.getControls();
        for (int c = 0; c < goe::controls::actionCount; c++) {
            auto a = (goe::controls::action) c;
            bool waiting = this->current == screen::BINDING && c == this->selected;
            lines.push_back(goe::controls::bindings::label(a) + ": "
                            + (waiting ? "press a key or pad button" : controls.describe(a)));
        }
        lines.push_back("Reset to defaults");
        lines.push_back("Back");
        return lines;
    }
    for (int c = 0; c < (int) this->options.size(); c++) {
        const auto &o = this->options[c];
        bool editing = this->current == screen::EDITING && c == this->selected;
        std::string value = editing ? this->editBuffer + "_" : o.value();
        lines.push_back(value.empty() ? o.label : o.label + ": " + value);
    }
    lines.push_back("Back");
    return lines;
}

std::string titleMenu::getTitle() const
{
    switch (this->current) {
    case screen::MAIN:
        return "";
    case screen::CONTROLS:
    case screen::BINDING:
        return "Controls";
    default:
        return "Config";
    }
}

const std::string &titleMenu::getEditBuffer() const
{
    return this->editBuffer;
}

const std::string &titleMenu::getMessage() const
{
    return this->message;
}
