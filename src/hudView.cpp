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

#include "hudView.h"
#include "difficulty.h"
#include "elements.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

hudView::hudView(int width, int stripHeight, coords tileSize, int spacing)
    : painter(tileSize, spacing)
    , strip(al_create_bitmap(width, stripHeight))
{}

bool hudView::loadFont(const std::string &fontFile)
{
    this->font.reset(al_load_ttf_font(fontFile.c_str(), 32, 0));
    return (bool) this->font;
}

void hudView::draw(const std::shared_ptr<bElem> &player, int dx, int dy, int width)
{
    if (!player || !this->strip)
        return;
    ALLEGRO_BITMAP *target = al_get_target_bitmap();
    this->paint(player);
    al_set_target_bitmap(target);
    al_draw_bitmap_region(this->strip.get(), 0, 0, width, height, dx, dy, 0);
}

void hudView::text(int x, int y, int offsetX, int offsetY, const std::string &what)
{
    if (!this->font)
        return;
    const coords tile = this->painter.tile();
    al_draw_text(this->font.get(),
                 al_map_rgb(255, 255, 200),
                 (float) (offsetX + x * tile.x),
                 (float) (offsetY + y * tile.y),
                 0,
                 what.c_str());
}

void hudView::icon(int x, const std::shared_ptr<bElem> &elem)
{
    this->painter.draw(elem, coords(x * this->painter.tile().x, 0), spritePainter::layer::top, false);
}

void hudView::paint(const std::shared_ptr<bElem> &player)
{
    const auto &inv = player->getAttrs()->getInventory();
    al_set_target_bitmap(this->strip.get());
    al_clear_to_color(al_map_rgba(0, 0, 0, 255));
    this->text(1, 1, 0, 5, "Garden: " + player->getBoard()->getName());
    this->icon(1, player);
    this->text(2, 0, 0, 0, std::to_string(player::countVisitedPlayers()));
    this->text(2, 0, 0, 32, std::to_string(player->getAttrs()->getEnergy()));
    const auto weapon = inv->getActiveWeapon();
    const auto usable = inv->getUsable();
    this->icon(4, weapon);
    this->icon(6, usable);
    if (weapon) {
        // how many weapons of this kind are held
        this->text(4, 0, 0, 32, "x" + std::to_string(inv->countActiveWeaponKind()));
        this->text(5, 0, 0, 32, std::to_string(weapon->getAttrs()->getEnergy()));
        this->text(5, 0, 0, 0, std::to_string(weapon->getAttrs()->getAmmo()));
    }
    if (usable) {
        this->text(7, 0, 0, 32, std::to_string(usable->getAttrs()->getEnergy()));
        this->text(7, 0, 0, 0, std::to_string(inv->countTokens(usable->getType(), usable->getAttrs()->getSubtype())));
    }
    for (int cnt = 0; cnt < 5; cnt++) {
        const auto key = inv->getKey(bElemTypes::_key, cnt, false);
        if (!key)
            continue;
        this->icon(8 + cnt * 2, key);
        this->text(9 + cnt * 2, 0, 0, 16, std::to_string(inv->countTokens(key->getType(), key->getAttrs()->getSubtype())));
    }
    this->icon(18, goldenApple::getApple(0));
    this->text(19, 0, 0, 0, std::to_string(goldenApple::getAppleNumber()));
    this->text(19, 0, 0, 32, std::to_string(inv->countTokens(bElemTypes::_goldenAppleType, 0)));
    this->text(21, 0, 6, 0, "Stats");
    this->text(21, 0, 5, 32, "P:");
    this->text(21, 1, 5, 0, "Dex:");
    this->text(22, 0, 5, 32, std::to_string(player->getStats()->getPoints(TOTAL)));
    this->text(22, 1, 5, 0, std::to_string(difficulty::playerLevel(player)));
    this->text(21, 1, 5, 32, "D:");
    this->text(22, 1, 5, 32, std::to_string(difficulty::of(player)));
}
