/*
 * Copyright (c) 2023, Ariel Konopka
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

#include "puppetMasterCollector.h"

std::shared_ptr<bElem> puppetMasterCollector::firstSolidInDirection(std::shared_ptr<bElem> body,
                                                                    dir::direction d)
{
    auto b = body->getElementInDirection(d);
    while (b && b->getAttrs()->isSteppable())
        b = b->getElementInDirection(d);
    return b;
}

bool puppetMasterCollector::drive(std::shared_ptr<bElem> body)
{
    dir::direction cdir = body->getStats()->getMyDirection();
    for (int c = 0; c < 4; c++) {
        auto d = (dir::direction) (((int) cdir + c) % 4);
        auto seen = this->firstSolidInDirection(body, d);
        if (!seen || !seen->getAttrs()->isCollectible() || seen->getType() == this->getType())
            continue;
        if (d == cdir)
            return this->step(body, d) || this->wander(body);
        // turn towards it one quarter at a time, like the original collector did
        this->turn(body, d == behind(cdir) ? ((goe::rng::gameplay()() % 2) ? leftOf(cdir) : rightOf(cdir)) : d);
        return true;
    }
    return this->wander(body);
}
