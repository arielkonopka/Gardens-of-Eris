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

#include "titleScreen.h"
#include "configManager.h"
#include "gameSettings.h"
#include "videoManager.h"
#include <algorithm>
#include <allegro5/allegro_ttf.h>
#include <cmath>

titleScreen::titleScreen(titleMenu &shown)
    : menu(shown)
{
    al_install_keyboard();
    al_install_joystick(); // pad buttons can be bound in Config
    al_init_font_addon();
    al_init_ttf_addon();
    auto cfg = configManager::getInstance()->getConfig();
    this->bigFont.reset(al_load_ttf_font(cfg->FontFile.c_str(), 72, 0));
    this->font.reset(al_load_ttf_font(cfg->FontFile.c_str(), 36, 0));
    this->helpFont.reset(al_load_ttf_font(cfg->FontFile.c_str(), 26, 0));
    this->splash.reset(al_load_bitmap(cfg->splashScr.c_str()));
    this->timer.reset(al_create_timer(1.0 / 30));
    this->queue.reset(al_create_event_queue());
    al_register_event_source(this->queue.get(), al_get_keyboard_event_source());
    if (auto *padEvents = al_get_joystick_event_source())
        al_register_event_source(this->queue.get(), padEvents);
    al_register_event_source(this->queue.get(), al_get_timer_event_source(this->timer.get()));
    if (auto *display = videoManager::getInstance().getCurrentDisplay())
        al_register_event_source(this->queue.get(), al_get_display_event_source(display));
}

titleMenu::action titleScreen::run()
{
    al_flush_event_queue(this->queue.get()); // keys pressed while a game was running
    this->pad.release();
    this->menu.refresh();                     // a game may have been saved meanwhile
    al_start_timer(this->timer.get());
    this->draw();
    ALLEGRO_EVENT ev;
    auto result = titleMenu::action::NONE;
    int swallowCharOf = -1;
    while (result == titleMenu::action::NONE) {
        al_wait_for_event(this->queue.get(), &ev);
        switch (ev.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            result = titleMenu::action::EXIT;
            break;
        case ALLEGRO_EVENT_KEY_DOWN:
            this->menu.pressed();
            // a key being bound is read as it goes down, so Shift, Ctrl and Alt count too;
            // the KEY_CHAR that follows for the same key must not also move the menu
            if (this->menu.getScreen() == titleMenu::screen::BINDING) {
                this->menu.keyDown(ev.keyboard.keycode);
                swallowCharOf = ev.keyboard.keycode;
            }
            break;
        case ALLEGRO_EVENT_KEY_CHAR:
            // KEY_CHAR repeats while a key is held, which is what menus and text fields want
        {
            if (ev.keyboard.keycode == swallowCharOf) {
                swallowCharOf = -1;
                break;
            }
            swallowCharOf = -1;
            if (this->menu.getScreen() == titleMenu::screen::BINDING)
                break;
            // only text typed inside the editor is text; the key that opens it is not
            bool wasEditing = this->menu.getScreen() == titleMenu::screen::EDITING;
            result = this->menu.keyDown(ev.keyboard.keycode);
            if (wasEditing && this->menu.getScreen() == titleMenu::screen::EDITING)
                this->menu.typed(ev.keyboard.unichar);
        }
            break;
        case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
            // a button being bound is only bound, it does not also confirm or go back
            if (this->menu.getScreen() == titleMenu::screen::BINDING) {
                this->menu.padButton(ev.joystick.button);
                break;
            }
            [[fallthrough]];
        case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
        case ALLEGRO_EVENT_JOYSTICK_AXIS:
            if (ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS && std::abs(ev.joystick.pos) > goe::controls::stickDeadzone)
                this->menu.pressed();
            if (auto c = this->padCommand(ev); c != goe::controls::menuCommand::none)
                result = this->menu.command(c);
            break;
        case ALLEGRO_EVENT_JOYSTICK_CONFIGURATION:
            al_reconfigure_joysticks();
            this->pad.release();
            break;
        case ALLEGRO_EVENT_TIMER: {
            const double tick = al_get_timer_speed(this->timer.get());
            // a stick or d-pad held down keeps moving
            if (auto c = this->pad.wait(tick); c != goe::controls::menuCommand::none)
                result = this->menu.command(c);
            if (result == titleMenu::action::NONE)
                result = this->menu.wait(tick);
            if (result == titleMenu::action::NONE && al_is_event_queue_empty(this->queue.get()))
                this->draw();
        }
            break;
        default:
            break;
        }
    }
    al_stop_timer(this->timer.get());
    return result;
}

void titleScreen::draw()
{
    auto *display = al_get_current_display();
    if (!display || !this->font || !this->bigFont)
        return;
    const float w = (float) al_get_display_width(display);
    const float h = (float) al_get_display_height(display);
    const ALLEGRO_COLOR normal = al_map_rgb(170, 170, 190);
    const ALLEGRO_COLOR chosen = al_map_rgb(255, 215, 90);
    const ALLEGRO_COLOR note = al_map_rgb(140, 200, 140);
    al_clear_to_color(al_map_rgba(15, 15, 25, 255));

    const auto lines = this->menu.getLines();
    const float lineH = (float) al_get_font_line_height(this->font.get()) * 1.4f;
    float y = h * 0.05f;
    // the picture is for the main menu; the settings lists need the room
    if (this->splash && this->menu.getScreen() == titleMenu::screen::MAIN) {
        float sw = (float) al_get_bitmap_width(this->splash.get());
        float sh = (float) al_get_bitmap_height(this->splash.get());
        // keep the picture in the top third of the screen
        float scale = std::min(1.0f, std::min(w * 0.8f / sw, h * 0.33f / sh));
        al_draw_scaled_bitmap(this->splash.get(), 0, 0, sw, sh, (w - sw * scale) / 2, y, sw * scale, sh * scale, 0);
        y += sh * scale + h * 0.02f;
    }
    al_draw_text(this->bigFont.get(), al_map_rgb(235, 235, 255), w / 2, y, ALLEGRO_ALIGN_CENTER, "Gardens of Eris");
    y += (float) al_get_font_line_height(this->bigFont.get()) * 1.3f;

    if (this->menu.getScreen() != titleMenu::screen::MAIN) {
        al_draw_text(this->font.get(), normal, w / 2, y, ALLEGRO_ALIGN_CENTER, this->menu.getTitle().c_str());
        y += (float) al_get_font_line_height(this->font.get()) * 1.5f;
    }
    // lines that do not fit above the message and the help scroll, keeping the selection in view
    const int fits = std::max(1, (int) ((h - lineH * 3.5f - y) / lineH));
    const int first = std::clamp(this->menu.getSelected() - fits / 2, 0, std::max(0, (int) lines.size() - fits));
    for (int c = first; c < (int) lines.size() && c < first + fits; c++) {
        bool sel = c == this->menu.getSelected();
        std::string text = sel ? "> " + lines[c] + " <" : lines[c];
        al_draw_text(this->font.get(), sel ? chosen : normal, w / 2, y, ALLEGRO_ALIGN_CENTER, text.c_str());
        y += lineH;
    }
    if (!this->menu.getMessage().empty()) {
        y += lineH * 0.5f;
        al_draw_text(this->font.get(), note, w / 2, y, ALLEGRO_ALIGN_CENTER, this->menu.getMessage().c_str());
    }
    const auto bound = gameSettings::getInstance().getControls();
    const std::string ok = buttonLabel(goe::controls::menuPad::confirmButton(bound));
    const std::string back = buttonLabel(goe::controls::menuPad::backButton(bound));
    std::vector<std::string> help;
    switch (this->menu.getScreen()) {
    case titleMenu::screen::CONFIG:
        help = {"Enter or " + ok + " to edit, Left/Right to change a value, Esc or " + back + " to go back"};
        break;
    case titleMenu::screen::EDITING:
        help = {"Type the value, Enter or " + ok + " to keep it, Esc or " + back + " to cancel"};
        break;
    case titleMenu::screen::CONTROLS:
        help = {"Enter or " + ok + " to change a control, Esc or " + back + " to go back"};
        break;
    case titleMenu::screen::BINDING:
        help = {"Press the new key or pad button; Backspace clears it, Esc cancels"};
        break;
    default:
        help = {"Up/Down or the pad's stick to choose, Enter or " + ok + " to select"};
        break;
    }
    this->drawHelp(help);
    al_flip_display();
}

void titleScreen::drawHelp(const std::vector<std::string> &help)
{
    auto *display = al_get_current_display();
    auto *f = this->helpFont ? this->helpFont.get() : this->font.get();
    if (!display || !f)
        return;
    const float w = (float) al_get_display_width(display);
    const float h = (float) al_get_display_height(display);
    const float lineH = (float) al_get_font_line_height(f) * 1.3f;
    float y = h - lineH * (float) (help.size() + 1);
    for (const auto &line : help) {
        al_draw_text(f, al_map_rgb(110, 110, 130), w / 2, y, ALLEGRO_ALIGN_CENTER, line.c_str());
        y += lineH;
    }
}

std::string titleScreen::buttonLabel(int button)
{
    // the first pad's own name for it, when it has one ("A" on an Xbox pad)
    if (al_is_joystick_installed() && al_get_num_joysticks() > 0)
        if (auto *joy = al_get_joystick(0); joy && button < al_get_joystick_num_buttons(joy))
            if (const char *name = al_get_joystick_button_name(joy, button); name && *name)
                return std::string("pad ") + name;
    return "pad button " + std::to_string(button);
}

goe::controls::menuCommand titleScreen::padCommand(const ALLEGRO_EVENT &ev)
{
    using goe::controls::menuCommand;
    using goe::controls::menuPad;
    auto *joy = ev.joystick.id;
    switch (ev.type) {
    case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN: {
        const char *name = joy ? al_get_joystick_button_name(joy, ev.joystick.button) : nullptr;
        return this->pad.buttonDown(ev.joystick.button, gameSettings::getInstance().getControls(),
                                    menuPad::directionOf(name ? name : ""));
    }
    case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
        this->pad.buttonUp(ev.joystick.button);
        return menuCommand::none;
    case ALLEGRO_EVENT_JOYSTICK_AXIS: {
        const char *name = joy ? al_get_joystick_stick_name(joy, ev.joystick.stick) : nullptr;
        const bool digital = joy && (al_get_joystick_stick_flags(joy, ev.joystick.stick) & ALLEGRO_JOYFLAG_DIGITAL);
        if (!menuPad::menuStick(ev.joystick.stick, digital, name ? name : ""))
            return menuCommand::none;
        return this->pad.axis(ev.joystick.stick, ev.joystick.axis, ev.joystick.pos);
    }
    default:
        return menuCommand::none;
    }
}

void titleScreen::showBusy(const std::string &text)
{
    auto *display = al_get_current_display();
    if (!display || !this->font)
        return;
    const float w = (float) al_get_display_width(display);
    const float h = (float) al_get_display_height(display);
    al_clear_to_color(al_map_rgba(15, 15, 25, 255));
    al_draw_text(this->font.get(), al_map_rgb(170, 170, 190), w / 2, h / 2, ALLEGRO_ALIGN_CENTER, text.c_str());
    al_flip_display();
}

bool titleScreen::showMessage(const std::string &headline, const std::vector<std::string> &lines)
{
    auto draw = [&]() {
        auto *display = al_get_current_display();
        if (!display || !this->font || !this->bigFont)
            return;
        const float w = (float) al_get_display_width(display);
        const float h = (float) al_get_display_height(display);
        const float lineH = (float) al_get_font_line_height(this->font.get()) * 1.4f;
        al_clear_to_color(al_map_rgba(15, 15, 25, 255));
        float y = h * 0.3f;
        al_draw_text(this->bigFont.get(), al_map_rgb(235, 235, 255), w / 2, y, ALLEGRO_ALIGN_CENTER, headline.c_str());
        y += (float) al_get_font_line_height(this->bigFont.get()) * 1.5f;
        for (const auto &line : lines) {
            al_draw_text(this->font.get(), al_map_rgb(170, 170, 190), w / 2, y, ALLEGRO_ALIGN_CENTER, line.c_str());
            y += lineH;
        }
        this->drawHelp({"Press Enter or " + buttonLabel(goe::controls::menuPad::confirmButton(
                                                gameSettings::getInstance().getControls()))
                        + " to continue"});
        al_flip_display();
    };
    // keys held while the game ended must not skip the message, so keys count after one second
    int ticksShown = 0;
    al_flush_event_queue(this->queue.get());
    this->pad.release();
    al_start_timer(this->timer.get());
    draw();
    ALLEGRO_EVENT ev;
    bool open = true;
    for (bool done = false; !done;) {
        al_wait_for_event(this->queue.get(), &ev);
        switch (ev.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            open = false;
            done = true;
            break;
        case ALLEGRO_EVENT_KEY_DOWN:
            done = ticksShown >= 30
                   && (ev.keyboard.keycode == ALLEGRO_KEY_ENTER || ev.keyboard.keycode == ALLEGRO_KEY_SPACE
                       || ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE);
            break;
        case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
        case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
        case ALLEGRO_EVENT_JOYSTICK_AXIS: {
            const auto c = this->padCommand(ev);
            done = ticksShown >= 30 && (c == goe::controls::menuCommand::confirm || c == goe::controls::menuCommand::back);
        }
            break;
        case ALLEGRO_EVENT_TIMER:
            ticksShown++;
            if (al_is_event_queue_empty(this->queue.get()))
                draw();
            break;
        default:
            break;
        }
    }
    al_stop_timer(this->timer.get());
    return open;
}

titleScreen::screenEnd titleScreen::showHallOfFame(const goe::hallOfFame &fame, double seconds, int highlight)
{
    auto draw = [&]() {
        auto *display = al_get_current_display();
        if (!display || !this->font || !this->bigFont)
            return;
        const float w = (float) al_get_display_width(display);
        const float h = (float) al_get_display_height(display);
        const float lineH = (float) al_get_font_line_height(this->font.get()) * 1.3f;
        al_clear_to_color(al_map_rgba(15, 15, 25, 255));
        float y = h * 0.08f;
        al_draw_text(this->bigFont.get(), al_map_rgb(255, 205, 0), w / 2, y, ALLEGRO_ALIGN_CENTER, "Hall of Fame");
        y += (float) al_get_font_line_height(this->bigFont.get()) * 1.5f;
        if (fame.entries().empty())
            al_draw_text(this->font.get(), al_map_rgb(170, 170, 190), w / 2, y, ALLEGRO_ALIGN_CENTER,
                         "Nobody yet. Eris is waiting.");
        for (std::size_t i = 0; i < fame.entries().size(); i++) {
            const auto &e = fame.entries()[i];
            const ALLEGRO_COLOR c = (int) i == highlight ? al_map_rgb(255, 215, 90) : al_map_rgb(170, 170, 190);
            al_draw_textf(this->font.get(), c, w * 0.22f, y, ALLEGRO_ALIGN_RIGHT, "%zu.", i + 1);
            al_draw_text(this->font.get(), c, w * 0.25f, y, ALLEGRO_ALIGN_LEFT, e.name.c_str());
            al_draw_textf(this->font.get(), c, w * 0.66f, y, ALLEGRO_ALIGN_RIGHT, "%d", e.score);
            al_draw_text(this->font.get(), c, w * 0.70f, y, ALLEGRO_ALIGN_LEFT, e.date.c_str());
            y += lineH;
        }
        this->drawHelp({seconds > 0 ? std::string("Press any key")
                                    : "Press Enter or "
                                          + buttonLabel(goe::controls::menuPad::confirmButton(
                                              gameSettings::getInstance().getControls()))
                                          + " to continue"});
        al_flip_display();
    };
    const double tick = al_get_timer_speed(this->timer.get());
    double passed = 0;
    al_flush_event_queue(this->queue.get());
    this->pad.release();
    al_start_timer(this->timer.get());
    draw();
    ALLEGRO_EVENT ev;
    auto result = screenEnd::TIMEOUT;
    for (bool done = false; !done;) {
        al_wait_for_event(this->queue.get(), &ev);
        switch (ev.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            result = screenEnd::CLOSED;
            done = true;
            break;
        case ALLEGRO_EVENT_KEY_DOWN:
            // after a game, keys held while it ended must not skip the list, so keys count after a second
            if (seconds > 0 || (passed >= 1.0
                                && (ev.keyboard.keycode == ALLEGRO_KEY_ENTER || ev.keyboard.keycode == ALLEGRO_KEY_SPACE
                                    || ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE))) {
                result = screenEnd::PRESSED;
                done = true;
            }
            break;
        case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
        case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
        case ALLEGRO_EVENT_JOYSTICK_AXIS: {
            // between demos any push ends it; after a game only confirm or back, after a second
            const auto c = this->padCommand(ev);
            const bool pushed = ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN
                                || (ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS
                                    && std::abs(ev.joystick.pos) > goe::controls::stickDeadzone);
            if ((seconds > 0 && pushed)
                || (passed >= 1.0 && (c == goe::controls::menuCommand::confirm || c == goe::controls::menuCommand::back))) {
                result = screenEnd::PRESSED;
                done = true;
            }
        }
            break;
        case ALLEGRO_EVENT_TIMER:
            passed += tick;
            if (seconds > 0 && passed >= seconds)
                done = true;
            else if (al_is_event_queue_empty(this->queue.get()))
                draw();
            break;
        default:
            break;
        }
    }
    al_stop_timer(this->timer.get());
    return result;
}

bool titleScreen::askName(const std::string &headline, const std::vector<std::string> &lines, std::string &name)
{
    auto draw = [&]() {
        auto *display = al_get_current_display();
        if (!display || !this->font || !this->bigFont)
            return;
        const float w = (float) al_get_display_width(display);
        const float h = (float) al_get_display_height(display);
        const float lineH = (float) al_get_font_line_height(this->font.get()) * 1.4f;
        al_clear_to_color(al_map_rgba(15, 15, 25, 255));
        float y = h * 0.25f;
        al_draw_text(this->bigFont.get(), al_map_rgb(235, 235, 255), w / 2, y, ALLEGRO_ALIGN_CENTER, headline.c_str());
        y += (float) al_get_font_line_height(this->bigFont.get()) * 1.5f;
        for (const auto &line : lines) {
            al_draw_text(this->font.get(), al_map_rgb(170, 170, 190), w / 2, y, ALLEGRO_ALIGN_CENTER, line.c_str());
            y += lineH;
        }
        y += lineH * 0.5f;
        al_draw_text(this->font.get(), al_map_rgb(255, 215, 90), w / 2, y, ALLEGRO_ALIGN_CENTER, (name + "_").c_str());
        this->drawHelp({"Type your name, Enter to keep it",
                        "Pad: Up/Down change the letter, Right adds one, Left removes one, "
                            + buttonLabel(goe::controls::menuPad::confirmButton(gameSettings::getInstance().getControls()))
                            + " keeps it"});
        al_flip_display();
    };
    // keys held while the game ended must not type or end the entry, so keys count after a second
    int ticksShown = 0;
    al_flush_event_queue(this->queue.get());
    this->pad.release();
    al_start_timer(this->timer.get());
    draw();
    ALLEGRO_EVENT ev;
    bool open = true;
    bool done = false;
    auto padPress = [&](goe::controls::menuCommand c) {
        using goe::controls::menuCommand;
        if (ticksShown < 30 || c == menuCommand::none)
            return;
        if (c == menuCommand::confirm) {
            done = true;
        } else if (c == menuCommand::back) {
            name.clear();
            done = true;
        } else if (!goe::controls::editName(name, c, goe::hallOfFame::nameLength)) {
            return;
        }
        draw();
    };
    while (!done) {
        al_wait_for_event(this->queue.get(), &ev);
        switch (ev.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            open = false;
            done = true;
            break;
        case ALLEGRO_EVENT_KEY_CHAR:
            if (ticksShown < 30)
                break;
            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER || ev.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER) {
                done = true;
            } else if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                name.clear();
                done = true;
            } else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                while (!name.empty() && (name.back() & 0xC0) == 0x80)
                    name.pop_back();
                if (!name.empty())
                    name.pop_back();
            } else if (ev.keyboard.unichar >= 32 && ev.keyboard.unichar != 127
                       && goe::controls::letterCount(name) < goe::hallOfFame::nameLength) {
                char buf[5] = {};
                al_utf8_encode(buf, ev.keyboard.unichar);
                name += buf;
            }
            draw();
            break;
        case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
        case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
        case ALLEGRO_EVENT_JOYSTICK_AXIS:
            padPress(this->padCommand(ev));
            break;
        case ALLEGRO_EVENT_TIMER:
            ticksShown++;
            padPress(this->pad.wait(al_get_timer_speed(this->timer.get())));
            if (al_is_event_queue_empty(this->queue.get()))
                draw();
            break;
        default:
            break;
        }
    }
    al_stop_timer(this->timer.get());
    return open;
}
