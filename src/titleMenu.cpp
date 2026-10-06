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
#include "musicChips.h"
#include "musicGenres.h"
#include <algorithm>
#include <allegro5/keycodes.h>
#include <exception>
#include <filesystem>
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

/// the stories files next to this one (stories.json, stories.pl.json, ...), sorted by name
std::vector<std::string> storiesFilesBeside(const std::string &file)
{
    std::vector<std::string> found;
    std::error_code ec;
    const auto folder = std::filesystem::path(file).parent_path();
    for (const auto &e : std::filesystem::directory_iterator(folder.empty() ? "." : folder, ec)) {
        const auto name = e.path().filename().string();
        if (e.is_regular_file(ec) && name.starts_with("stories") && name.ends_with(".json"))
            found.push_back((folder / name).generic_string());
    }
    std::sort(found.begin(), found.end());
    return found;
}
} // namespace

titleMenu::titleMenu(gameSettings &edited, std::string file, std::function<bool()> readable)
    : settings(edited)
    , settingsFile(std::move(file))
    , saveReadable(std::move(readable))
{
    this->refresh();
    this->options.push_back({"Save location",
                             [this] { return this->settings.getSaveDirectory(); },
                             [this](const std::string &v) {
                                 return this->settings.setSaveDirectory(v);
                             },
                             {}});
    // a percent line: shows "80%", takes a typed number from lowest to highest, and Left/Right step it by five
    auto percent = [](std::string label, std::function<int()> get, std::function<void(int)> set, int lowest = 0,
                      int highest = 100) {
        return option{std::move(label),
                      [get] { return std::to_string(get()) + "%"; },
                      [set, lowest, highest](const std::string &v) {
                          try {
                              size_t used = 0;
                              int value = std::stoi(v, &used);
                              if (value < lowest || value > highest || (used < v.size() && v.substr(used) != "%"))
                                  return false;
                              set(value);
                              return true;
                          } catch (const std::exception &) {
                              return false;
                          }
                      },
                      [get, set](int by) { set(get() + by * gameSettings::volumeStep); }};
    };
    this->options.push_back(percent(
        "Music volume",
        [this] { return this->settings.getMusicVolume(); },
        [this](int v) { this->settings.setMusicVolume(v); }));
    this->options.push_back(percent(
        "Sound effects volume",
        [this] { return this->settings.getEffectsVolume(); },
        [this](int v) { this->settings.setEffectsVolume(v); }));
    // Skin samples, Performer or DJ: Enter and Right step forward, Left back
    this->options.push_back({"Music",
                             [this] {
                                 switch (this->settings.getMusicSource()) {
                                 case gameSettings::musicSource::performer:
                                     return std::string("Performer");
                                 case gameSettings::musicSource::dj:
                                     return std::string("DJ");
                                 default:
                                     return std::string("Skin samples");
                                 }
                             },
                             {},
                             [this](int by) {
                                 constexpr int n = 3;
                                 const int now = (int) this->settings.getMusicSource();
                                 this->settings.setMusicSource((gameSettings::musicSource) (((now + by) % n + n) % n));
                             }});
    // the performer's chip: Enter and Right step forward, Left back
    this->options.push_back({"Performer sound",
                             [this] { return std::string(goe::musician::nameOf(this->settings.getPerformerSound())); },
                             {},
                             [this](int by) {
                                 const int n = goe::musician::chipStyleCount;
                                 const int now = (int) this->settings.getPerformerSound();
                                 this->settings.setPerformerSound((goe::musician::chipStyle) (((now + by) % n + n) % n));
                             }});
    // the music style, stepped like the sound
    this->options.push_back({"Music style",
                             [this] { return std::string(goe::musician::nameOf(this->settings.getMusicStyle())); },
                             {},
                             [this](int by) {
                                 const int n = goe::musician::genreCount;
                                 const int now = (int) this->settings.getMusicStyle();
                                 this->settings.setMusicStyle((goe::musician::genre) (((now + by) % n + n) % n));
                             }});
    this->options.push_back(percent(
        "Music variety",
        [this] { return this->settings.getMusicVariety(); },
        [this](int v) { this->settings.setMusicVariety(v); }));
    this->options.push_back(percent(
        "Music tempo",
        [this] { return this->settings.getMusicTempo(); },
        [this](int v) { this->settings.setMusicTempo(v); },
        gameSettings::minMusicTempo,
        gameSettings::maxMusicTempo));
    this->options.push_back({"Story scroller",
                             [this] { return std::string(this->settings.getStoriesShown() ? "On" : "Off"); },
                             {},
                             [this](int) { this->settings.setStoriesShown(!this->settings.getStoriesShown()); }});
    // a typed path, or Left/Right step through the other stories files in the same folder
    this->options.push_back({"Stories file",
                             [this] { return this->settings.getStoriesFile(); },
                             [this](const std::string &v) { return this->settings.setStoriesFile(v); },
                             [this](int by) {
                                 const auto chosen = this->settings.getStoriesFile();
                                 const auto files = storiesFilesBeside(chosen);
                                 if (files.empty())
                                     return;
                                 auto at = std::find(files.begin(), files.end(), std::filesystem::path(chosen).generic_string());
                                 const int n = (int) files.size();
                                 const int i = at == files.end() ? (by > 0 ? -1 : 0) : (int) (at - files.begin());
                                 this->settings.setStoriesFile(files[(std::size_t) ((i + by + n) % n)]);
                             }});
    // a time in seconds: a typed number, or Left/Right step it by five
    auto seconds = [](std::string label, std::function<int()> get, std::function<void(int)> set) {
        return option{std::move(label),
                      [get] { return std::to_string(get()) + " s"; },
                      [set](const std::string &v) {
                          try {
                              size_t used = 0;
                              int s = std::stoi(v, &used);
                              while (used < v.size() && v[used] == ' ')
                                  used++;
                              if (s < gameSettings::minSeconds || s > gameSettings::maxSeconds
                                  || (used < v.size() && v.substr(used) != "s"))
                                  return false;
                              set(s);
                              return true;
                          } catch (const std::exception &) {
                              return false;
                          }
                      },
                      [get, set](int by) { set(get() + by * gameSettings::secondsStep); }};
    };
    this->options.push_back(seconds(
        "Demo after", [this] { return this->settings.getDemoWait(); }, [this](int v) { this->settings.setDemoWait(v); }));
    this->options.push_back(seconds(
        "Demo length", [this] { return this->settings.getDemoLength(); }, [this](int v) { this->settings.setDemoLength(v); }));
    this->options.push_back(seconds(
        "Hall of fame shown",
        [this] { return this->settings.getHallOfFameLength(); },
        [this](int v) { this->settings.setHallOfFameLength(v); }));
    this->options.push_back({"Controls", [] { return std::string(); }, {}, {}});
}

void titleMenu::refresh()
{
    this->canContinue = this->saveReadable && this->saveReadable();
    this->current = screen::MAIN;
    this->selected = 0;
    this->message.clear();
    this->idle = 0;
}

void titleMenu::setDemoAfter(double seconds)
{
    this->demoAfter = seconds;
    this->idle = 0;
}

titleMenu::action titleMenu::wait(double seconds)
{
    if (this->current != screen::MAIN || this->demoAfter <= 0) {
        this->idle = 0;
        return action::NONE;
    }
    this->idle += seconds;
    return this->idle >= this->demoAfter ? action::DEMO : action::NONE;
}

void titleMenu::pressed()
{
    this->idle = 0;
}

std::vector<titleMenu::mainItem> titleMenu::mainItems() const
{
    if (this->canContinue)
        return {mainItem::CONTINUE, mainItem::START, mainItem::CONFIG, mainItem::EXIT};
    return {mainItem::START, mainItem::CONFIG, mainItem::EXIT};
}

std::string titleMenu::labelOf(mainItem item) const
{
    switch (item) {
    case mainItem::CONTINUE:
        return "Continue";
    case mainItem::START:
        // next to Continue it starts over, and the new game takes the old save's place
        return this->canContinue ? "New game" : "Start game";
    case mainItem::CONFIG:
        return "Config";
    default:
        return "Exit";
    }
}

int titleMenu::controlsLine() const
{
    return (int) this->options.size() - 1;
}

titleMenu::action titleMenu::keyDown(int keycode)
{
    this->pressed();
    const bool enter = keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER
                       || keycode == ALLEGRO_KEY_SPACE;
    switch (this->current) {
    case screen::MAIN:
        if (keycode == ALLEGRO_KEY_UP)
            this->move(-1);
        else if (keycode == ALLEGRO_KEY_DOWN)
            this->move(1);
        else if (enter) {
            switch (this->mainItems()[this->selected]) {
            case mainItem::CONTINUE:
                return action::CONTINUE;
            case mainItem::START:
                return action::START;
            case mainItem::EXIT:
                return action::EXIT;
            case mainItem::CONFIG:
                this->current = screen::CONFIG;
                this->selected = 0;
                this->message.clear();
                break;
            }
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
        const auto items = this->mainItems();
        this->selected = (int) (std::find(items.begin(), items.end(), mainItem::CONFIG) - items.begin());
    } else if (enter) {
        this->message.clear();
        const auto &opt = this->options[this->selected];
        if (this->selected == this->controlsLine()) {
            this->current = screen::CONTROLS;
            this->selected = 0;
        } else if (!opt.apply && opt.adjust) { // a switch: Enter flips it
            opt.adjust(1);
            this->saveSettings();
        } else {
            this->editBuffer = opt.value();
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
    this->pressed();
    if (this->current != screen::BINDING)
        return;
    auto controls = this->settings.getControls();
    controls.bindPadButton((goe::controls::action) this->selected, button);
    this->settings.setControls(controls);
    this->current = screen::CONTROLS;
    this->saveSettings();
}

titleMenu::action titleMenu::command(goe::controls::menuCommand c)
{
    using goe::controls::menuCommand;
    this->pressed();
    if (this->current == screen::BINDING)
        return action::NONE;
    if (c == menuCommand::confirm && this->current == screen::CONFIG && this->selected < (int) this->options.size()) {
        const auto &opt = this->options[this->selected];
        if (opt.apply && opt.adjust) {
            this->message = "Left and Right change it";
            return action::NONE;
        }
    }
    switch (c) {
    case menuCommand::up:
        return this->keyDown(ALLEGRO_KEY_UP);
    case menuCommand::down:
        return this->keyDown(ALLEGRO_KEY_DOWN);
    case menuCommand::left:
        return this->keyDown(ALLEGRO_KEY_LEFT);
    case menuCommand::right:
        return this->keyDown(ALLEGRO_KEY_RIGHT);
    case menuCommand::confirm:
        return this->keyDown(ALLEGRO_KEY_ENTER);
    case menuCommand::back:
        return this->keyDown(ALLEGRO_KEY_ESCAPE);
    default:
        return action::NONE;
    }
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
        return (int) this->mainItems().size();
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
    std::vector<std::string> lines;
    if (this->current == screen::MAIN) {
        for (auto item : this->mainItems())
            lines.push_back(labelOf(item));
        return lines;
    }
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
