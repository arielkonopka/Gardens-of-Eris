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

#include "motion.h"
#include "bElem.h"
#include "elementSound.h"

namespace motion {
namespace {
/// places who onto target as a move that takes speed ticks
bool moveOnto(const std::shared_ptr<bElem> &who, const std::shared_ptr<bElem> &target, int speed)
{
    if (!target || !target->getAttrs()->isSteppable() || !who->stepOnElement(target))
        return false;
    who->getStats()->setMoved(speed);
    goe::sound::play(*who, "Move", "StepOn");
    return true;
}

bool canCollect(const std::shared_ptr<bElem> &who, const std::shared_ptr<bElem> &what)
{
    return who->getAttrs()->canCollect() && what->getAttrs()->isCollectible();
}

bool push(const std::shared_ptr<bElem> &who, const std::shared_ptr<bElem> &what,
          dir::direction d, int speed)
{
    auto beyond = what->getElementInDirection(d);
    if (!who->getAttrs()->canPush() || !what->getAttrs()->canBePushed()
        || !what->getAttrs()->isMovable() || !beyond || !beyond->getAttrs()->isSteppable()
        || !step(what, d, speed + 1))
        return false;
    // the pushed element has moved on, the pusher takes its cell
    return moveOnto(who, who->getElementInDirection(d), speed + 1);
}
} // namespace

bool collectAndStep(const std::shared_ptr<bElem> &who, const std::shared_ptr<bElem> &collectible,
                    int speed)
{
    if (!canCollect(who, collectible))
        return false;
    const coords cell = collectible->getStats()->getMyPosition();
    auto board = collectible->getBoard();
    if (!who->collect(collectible))
        return false;
    if (board && cell != NOCOORDS)
        moveOnto(who, board->getElement(cell), speed);
    return true;
}

bool step(const std::shared_ptr<bElem> &who, dir::direction d, int speed)
{
    const auto &st = *who->getStats();
    if (d == dir::direction::NODIRECTION || st.isMoving() || st.isDying() || st.isTeleporting()
        || st.isDestroying())
        return false;
    auto target = who->getElementInDirection(d);
    if (!target)
        return false;
    who->getStats()->setMyDirection(d);
    if (canCollect(who, target))
        if (collectAndStep(who, target, speed))
            return true;
    if (moveOnto(who, target, speed))
        return true;
    if (push(who, target, d, speed))
        return true;
    return who->getAttrs()->isInteractive() && target->interact(who);
}

bool drag(const std::shared_ptr<bElem> &who, dir::direction d, int speed)
{
    // the element to drag is behind the mover; failing that, behind the way it faces
    dir::direction pullDir = d;
    auto dragged = who->getElementInDirection(dir::getOppositeDirection(d));
    if (!dragged)
        return false;
    if (!dragged->getAttrs()->isMovable()) {
        pullDir = who->getStats()->getMyDirection();
        dragged = who->getElementInDirection(dir::getOppositeDirection(pullDir));
        if (!dragged || !dragged->getAttrs()->isMovable())
            return false;
    }
    step(who, d, speed);
    return step(dragged, pullDir, speed);
}
} // namespace motion
