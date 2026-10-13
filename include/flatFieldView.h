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


#ifndef FLATFIELDVIEW_H
#define FLATFIELDVIEW_H
#include "allegroHandles.h"
#include "fieldView.h"
#include "fogLayer.h"
#include "spritePainter.h"
#include <string>

/// The game field as flat sprites seen from above, under the fog of view.
class flatFieldView : public fieldView
{
public:
    /// needs the display; the scene is a bitmap of scenePx pixels, a little larger than the field
    flatFieldView(coords scenePx, coords tileSize, int spacing, const std::string &fogBitmap);
    void draw(const fieldFrame &frame) override;

private:
    spritePainter painter;
    goe::bitmapHandle scene;
    fogLayer fog;
};

#endif // FLATFIELDVIEW_H
