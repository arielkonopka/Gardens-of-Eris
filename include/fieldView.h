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


#ifndef FIELDVIEW_H
#define FIELDVIEW_H
#include "commons.h"
#include "fieldCamera.h"
#include <memory>

class bElem;

/// What a view needs to draw one frame of the game field.
struct fieldFrame {
    /// the active avatar; its board is drawn
    std::shared_ptr<bElem> focus;
    /// which part of the board, and how far it is scrolled
    const fieldCamera &camera;
    /// whole cells across and down the field
    coords cells;
    /// the field's top left corner on the current target, and its size, in pixels
    coords at;
    coords size;
};

/**
 * One way of drawing the game field (the 2D sprites, the 3D isometric view...). The game loop
 * picks one each frame and knows nothing else about how the field is drawn. A view draws onto
 * the current target bitmap and leaves it the current target.
 */
class fieldView
{
public:
    virtual ~fieldView() = default;
    virtual void draw(const fieldFrame &frame) = 0;
    /// eighths of a turn (anticlockwise, as dir::turned) that bring a way pressed on the screen
    /// to the way it goes on the board, so up always moves up the screen
    virtual int keyTurn() const { return 0; }
};

#endif // FIELDVIEW_H
