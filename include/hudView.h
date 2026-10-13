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


#ifndef HUDVIEW_H
#define HUDVIEW_H
#include "allegroHandles.h"
#include "commons.h"
#include "spritePainter.h"
#include <memory>
#include <string>

class bElem;

/// The stats strip under the game field: the avatar, energy, weapons, keys, apples, score, Dex and D.
class hudView
{
public:
    /// the strip's height on screen, in pixels
    static constexpr int height = 128;

    /// needs the display; the strip is width x stripHeight pixels
    hudView(int width, int stripHeight, coords tileSize, int spacing);
    /// false when the font cannot be loaded
    bool loadFont(const std::string &fontFile);
    /// draws the player's stats at (dx, dy) of the current target, width pixels wide
    void draw(const std::shared_ptr<bElem> &player, int dx, int dy, int width);

private:
    void paint(const std::shared_ptr<bElem> &player);
    /// text at cell (x, y) of the strip's grid, moved by (offsetX, offsetY) pixels
    void text(int x, int y, int offsetX, int offsetY, const std::string &what);
    /// an element's icon at cell (x, 0)
    void icon(int x, const std::shared_ptr<bElem> &elem);
    spritePainter painter;
    goe::fontHandle font;
    goe::bitmapHandle strip;
};

#endif // HUDVIEW_H
