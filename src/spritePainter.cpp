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

#include "spritePainter.h"
#include "bElem.h"
#include "elementView.h"
#include "spriteFrame.h"
#include "videoDriver.h"

spritePainter::spritePainter(coords tile, int sheetSpacing)
    : tileSize(tile)
    , spacing(sheetSpacing)
{}

bool spritePainter::draw(const std::shared_ptr<bElem> &elem, coords cellPx, layer which, bool withMotion) const
{
    if (!elem)
        return false;
    const auto ve = videoDriver::getInstance().getVideoElement(elem->getType());
    if (!ve)
        return false;
    const bool moving = elem->getStats()->isMoving();
    if (which == layer::moving && !moving)
        return false;
    bool movingBelow = false;
    if (which != layer::top)
        movingBelow = this->draw(elem->getStats()->getSteppingOn(), cellPx, which, withMotion);
    if (which == layer::still && moving)
        return true;
    if (const auto cell = goe::sprites::frame(*elem, *ve)) {
        const coords src = goe::sprites::pixelOf(*cell, this->tileSize, this->spacing);
        const coords at = cellPx + (withMotion ? elementView::offset(*elem, this->tileSize) : coords(0, 0));
        al_draw_bitmap_region(ve->sprites.get(), src.x, src.y, this->tileSize.x, this->tileSize.y, at.x, at.y, 0);
    }
    return movingBelow;
}
