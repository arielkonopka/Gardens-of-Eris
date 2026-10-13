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

#include "flatFieldView.h"
#include "chamber.h"
#include "elements.h"
#include "viewPoint.h"
#include <vector>

flatFieldView::flatFieldView(coords scenePx, coords tileSize, int spacing, const std::string &fogBitmap)
    : painter(tileSize, spacing)
    , scene(al_create_bitmap(scenePx.x, scenePx.y))
{
    this->fog.setup(scenePx.x, scenePx.y, tileSize, fogBitmap);
}

void flatFieldView::draw(const fieldFrame &frame)
{
    const auto &focus = frame.focus;
    const auto board = focus ? focus->getBoard() : nullptr;
    if (!board)
        return;
    const coords tile = this->painter.tile();
    const coords topLeft = frame.camera.topLeft();
    ALLEGRO_BITMAP *target = al_get_target_bitmap();
    al_set_target_bitmap(this->scene.get());
    const colour c = board->getChColour();
    al_clear_to_color(al_map_rgba(c.r, c.g, c.b, c.a));
    // still elements first, walls are always shown; cells nobody has seen yet stay empty
    struct movingCell {
        coords px;
        std::shared_ptr<bElem> elem;
    };
    std::vector<movingCell> moving;
    for (int x = 0; x < frame.cells.x + 1; x++)
        for (int y = 0; y < frame.cells.y + 1; y++) {
            const coords np = topLeft + coords(x, y);
            if (board->isVisible(np) >= 255 && !viewPoint::get_instance().isPointVisible(np))
                continue;
            const auto elem = board->getElement(np);
            const coords px(x * tile.x, y * tile.y);
            if (elem && this->painter.draw(elem, px, spritePainter::layer::still, true))
                moving.push_back({px, elem});
        }
    // then the ones on the move, over them, and the avatar last of all
    std::optional<coords> focusPx;
    for (const auto &m : moving) {
        if (m.elem->getStats()->getInstanceId() == focus->getStats()->getInstanceId())
            focusPx = m.px;
        else
            this->painter.draw(m.elem, m.px, spritePainter::layer::moving, true);
    }
    if (focusPx && focus->getStats()->isMoving())
        this->painter.draw(board->getElement(focus->getStats()->getMyPosition()),
                           *focusPx,
                           spritePainter::layer::moving,
                           true);

    al_set_target_bitmap(target);
    const auto points = viewPoint::get_instance().getViewPoints(topLeft, topLeft + frame.cells + 1);
    const coords scroll = frame.camera.scroll();
    this->fog.draw(this->scene.get(),
                   points,
                   coords(topLeft.x * tile.x, topLeft.y * tile.y),
                   scroll.x,
                   scroll.y,
                   frame.size.x,
                   frame.size.y,
                   frame.at.x,
                   frame.at.y);
}
