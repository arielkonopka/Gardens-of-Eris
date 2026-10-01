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


#include "gameEvents.h"
#include "bElem.h"

namespace goe::events {
namespace {
observer &watcher()
{
    static observer o;
    return o;
}

const bElem *&current()
{
    static const bElem *who = nullptr;
    return who;
}
} // namespace

void observe(observer o)
{
    watcher() = std::move(o);
}

void report(kind k, const bElem &subject, const bElem *actor)
{
    if (auto &w = watcher())
        w(k, subject, actor);
}

blame::blame(const bElem *who) : before(current())
{
    current() = who;
}

blame::~blame()
{
    current() = this->before;
}

const bElem *blamed()
{
    return current();
}

bool isDown(const bElem &e)
{
    return e.getStats()->isDying() || e.getStats()->isDestroying() || e.getStats()->isDisposed();
}
} // namespace goe::events
