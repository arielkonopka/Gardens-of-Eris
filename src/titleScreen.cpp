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
#include "videoManager.h"
#include <algorithm>
#include <allegro5/allegro_ttf.h>

titleScreen::titleScreen(titleMenu &menu)
    : menu(menu)
{
    al_install_keyboard();
    al_init_font_addon();
    al_init_ttf_addon();
    auto cfg = configManager::getInstance()->getConfig();
    this->bigFont.reset(al_load_ttf_font(cfg->FontFile.c_str(), 72, 0));
    this->font.reset(al_load_ttf_font(cfg->FontFile.c_str(), 36, 0));
    this->splash.reset(al_load_bitmap(cfg->splashScr.c_str()));
    this->timer.reset(al_create_timer(1.0 / 30));
    this->queue.reset(al_create_event_queue());
    al_register_event_source(this->queue.get(), al_get_keyboard_event_source());
    al_register_event_source(this->queue.get(), al_get_timer_event_source(this->timer.get()));
    if (auto *display = videoManager::getInstance().getCurrentDisplay())
        al_register_event_source(this->queue.get(), al_get_display_event_source(display));
}

titleMenu::action titleScreen::run()
{
    al_start_timer(this->timer.get());
    this->draw();
    ALLEGRO_EVENT ev;
    auto result = titleMenu::action::NONE;
    while (result == titleMenu::action::NONE) {
        al_wait_for_event(this->queue.get(), &ev);
        switch (ev.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            result = titleMenu::action::EXIT;
            break;
        case ALLEGRO_EVENT_KEY_CHAR:
            // KEY_CHAR repeats while a key is held, which is what menus and text fields want
        {
            // only text typed inside the editor is text; the key that opens it is not
            bool wasEditing = this->menu.getScreen() == titleMenu::screen::EDITING;
            result = this->menu.keyDown(ev.keyboard.keycode);
            if (wasEditing && this->menu.getScreen() == titleMenu::screen::EDITING)
                this->menu.typed(ev.keyboard.unichar);
        }
            break;
        case ALLEGRO_EVENT_TIMER:
            if (al_is_event_queue_empty(this->queue.get()))
                this->draw();
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

    float y = h * 0.05f;
    if (this->splash) {
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
        al_draw_text(this->font.get(), normal, w / 2, y, ALLEGRO_ALIGN_CENTER, "Config");
        y += (float) al_get_font_line_height(this->font.get()) * 1.5f;
    }
    const auto lines = this->menu.getLines();
    const float lineH = (float) al_get_font_line_height(this->font.get()) * 1.4f;
    for (int c = 0; c < (int) lines.size(); c++) {
        bool sel = c == this->menu.getSelected();
        std::string text = sel ? "> " + lines[c] + " <" : lines[c];
        al_draw_text(this->font.get(), sel ? chosen : normal, w / 2, y, ALLEGRO_ALIGN_CENTER, text.c_str());
        y += lineH;
    }
    if (!this->menu.getMessage().empty()) {
        y += lineH * 0.5f;
        al_draw_text(this->font.get(), note, w / 2, y, ALLEGRO_ALIGN_CENTER, this->menu.getMessage().c_str());
    }
    const char *help = "Up/Down to choose, Enter to select";
    if (this->menu.getScreen() == titleMenu::screen::CONFIG)
        help = "Enter to edit, Esc to go back";
    else if (this->menu.getScreen() == titleMenu::screen::EDITING)
        help = "Type the folder, Enter to keep it, Esc to cancel";
    al_draw_text(this->font.get(), al_map_rgb(110, 110, 130), w / 2, h - lineH * 2, ALLEGRO_ALIGN_CENTER, help);
    al_flip_display();
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
