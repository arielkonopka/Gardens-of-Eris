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


#ifndef SPRITEPAINTER_H
#define SPRITEPAINTER_H
#include "commons.h"
#include <memory>

class bElem;

/// Draws elements as flat sprites from the sprite sheet onto the current target bitmap.
class spritePainter
{
public:
    /// which elements of a cell to draw
    enum class layer {
        /// everything standing still, with what it stands on; moving ones are left for later
        still,
        /// only elements on the move, drawn over the still ones
        moving,
        /// only the element itself (the HUD's icons)
        top
    };

    spritePainter(coords tileSize, int spacing);
    /**
     * Draws elem at pixel cellPx (the top left of its cell). withMotion shifts an element
     * between two cells to where it is on its way.
     * @return for layer::still, true when elem or something under it is on the move, so it
     * needs drawing again in the moving layer
     */
    bool draw(const std::shared_ptr<bElem> &elem, coords cellPx, layer which, bool withMotion) const;
    coords tile() const { return this->tileSize; }

private:
    coords tileSize;
    int spacing;
};

#endif // SPRITEPAINTER_H
