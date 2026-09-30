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

#include "elementSound.h"
#include "bElem.h"
#include "soundManager.h"

namespace goe::sound {
namespace {
observer &watcher()
{
    static observer o;
    return o;
}

/// the element whose place the sound comes from
std::shared_ptr<bElem> heardAt(const bElem &elem)
{
    auto &st = *elem.getStats();
    if (auto collector = st.getCollector().lock())
        return collector;
    if (elem.getBoard())
        return std::const_pointer_cast<bElem>(elem.shared_from_this());
    if (st.hasParent())
        return st.getStandingOn().lock();
    return st.getSteppingOn();
}
} // namespace

void observe(observer o)
{
    watcher() = std::move(o);
}

void play(const bElem &elem, const std::string &eventType, const std::string &event)
{
    if (auto &w = watcher())
        w(elem, eventType, event);
    auto where = heardAt(elem);
    auto board = where ? where->getBoard() : nullptr;
    if (!board)
        return;
    const coords at = where->getStats()->getMyPosition();
    soundManager::getInstance().registerSound(board->getInstanceId(),
                                              coords3d{(float) at.x, (float) at.y, 0.0f},
                                              coords3d{0.0f, 0.0f, 0.0f},
                                              elem.getStats()->getInstanceId(),
                                              elem.getType(),
                                              elem.getAttrs()->getSubtype(),
                                              eventType,
                                              event);
}
} // namespace goe::sound
