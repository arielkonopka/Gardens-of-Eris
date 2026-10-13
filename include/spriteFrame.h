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


#ifndef SPRITEFRAME_H
#define SPRITEFRAME_H
#include "commons.h"
#include "videoElementDef.h"
#include <optional>

class bElem;

/// Which picture of the sprite sheet an element shows right now; shared by every view.
namespace goe::sprites {
/**
 * The sprite sheet cell (column, row) the element shows: its subtype, facing and animation phase,
 * or the dying, destroying or teleporting (fading out, fading in) sequence while one runs.
 * Empty when the definition has no picture for it.
 */
std::optional<coords> frame(const bElem &elem, const videoElement::videoElementDef &def);
/// the top left pixel of a sheet cell, for tiles of tileSize with spacing pixels around each
constexpr coords pixelOf(coords cell, coords tileSize, int spacing)
{
    return coords(cell.x * tileSize.x + (cell.x + 1) * spacing, cell.y * tileSize.y + (cell.y + 1) * spacing);
}
} // namespace goe::sprites

#endif // SPRITEFRAME_H
