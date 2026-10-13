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


#ifndef FIELDCAMERA_H
#define FIELDCAMERA_H
#include "commons.h"
#include <optional>

/**
 * Which part of the board the game field shows. It follows a centre cell smoothly, a few pixels
 * per frame, so the view glides instead of jumping a whole cell. Every view of the field uses it.
 */
class fieldCamera
{
public:
    /// pixels the view moves per frame towards where it should be
    static constexpr int step = 8;

    explicit fieldCamera(coords tileSize);

    /**
     * One frame: moves towards the window of tilesOnScreen cells centred on centre. A bounded
     * board of boardSize cells keeps the window inside it; the endless world does not.
     */
    void follow(coords centre, coords tilesOnScreen, std::optional<coords> boardSize = std::nullopt);
    /// the cell in the top left corner of the field
    coords topLeft() const { return this->cell; }
    /// how far the field is scrolled past topLeft, in pixels (0 to tile size - 1)
    coords scroll() const;
    /// the top left corner in cells, with the scroll as a fraction
    float leftCells() const { return (float) this->pixels.x / (float) this->tileSize.x; }
    float topCells() const { return (float) this->pixels.y / (float) this->tileSize.y; }

private:
    coords tileSize;
    /// the top left corner of the field, in board pixels
    coords pixels{0, 0};
    coords cell{0, 0};
};

#endif // FIELDCAMERA_H
