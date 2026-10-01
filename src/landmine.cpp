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

#include "landmine.h"
#include "gameEvents.h"

int landmine::getType() const
{
    return bElemTypes::_landmineType;
}

bool landmine::mechanics()
{
    // an explosion only reaches the cells around it, so the one standing on the mine is hit here
    auto rider = this->getStats()->getStandingOn().lock();
    bool exploded = simpleBomb::mechanics();
    if (exploded && rider && !rider->getStats()->isDisposed())
        rider->destroy();
    return exploded;
}

bool landmine::stepOnAction(bool step, std::shared_ptr<bElem> who)
{
    bool r = bElem::stepOnAction(step, who);
    if (step && who) {
        // a missile running into the mine sets it off for its shooter
        goe::events::blame by(who->getStats()->getStatsOwner().lock().get());
        this->destroy();
    }
    return r;
}
