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

#include "elementView.h"
#include "bElem.h"
#include "configManager.h"

namespace elementView {
coords offset(const bElem &elem, coords tileSize)
{
    const auto &st = *elem.getStats();
    if (!st.isMoving() || st.getMovingTotalTime() <= 0)
        return coords(0, 0);
    coords done = {(st.getMoved() * tileSize.x) / st.getMovingTotalTime(),
                   (st.getMoved() * tileSize.y) / st.getMovingTotalTime()};
    return dir::dirToCoords(st.getMyDirection()) * done * (-1);
}

coords offset(const bElem &elem)
{
    const auto &cfg = *configManager::getInstance()->getConfig();
    return offset(elem, coords(cfg.tileWidth, cfg.tileHeight));
}
} // namespace elementView
