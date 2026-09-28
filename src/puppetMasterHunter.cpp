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

#include "puppetMasterHunter.h"
#include "player.h"
#include <cstdlib>

bool puppetMasterHunter::drive(std::shared_ptr<bElem> body)
{
    auto prey = player::getActivePlayer();
    if (!prey || prey->getBoard() != body->getBoard())
        return this->wander(body);
    auto from = body->getStats()->getMyPosition();
    auto to = prey->getStats()->getMyPosition();
    int dx = to.x - from.x, dy = to.y - from.y;
    if (std::abs(dx) + std::abs(dy) > sightRange)
        return this->wander(body);
    // walk around walls towards the player, never further than sightRange from where we are
    auto d = pathTowards(body, to, from, sightRange);
    if (d != dir::direction::NODIRECTION && this->step(body, d))
        return true;
    return this->wander(body);
}
