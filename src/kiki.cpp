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

#include "kiki.h"

bool kiki::mechanics()
{
    if (!bElem::mechanics())
        return false;
    const auto mdir = this->direction;
    const auto board = this->getBoard();
    coords pos = this->getStats()->getMyPosition();
    coords step = dir::dirToCoords(mdir);
    // Both passes walk the beam cell by cell on the board, reading the top elements in place:
    // this runs for every kiki on every few ticks, so it must not copy pointers per cell.
    if (board && pos != NOCOORDS && step != coords(0, 0)) {
        // is the way to the partner kiki still clear?
        for (coords p = pos + step;; p = p + step) {
            const auto &e = board->topAt(p);
            if (!e || e->getType() == this->getType())
                break;
            if (!e->getAttrs()->isSteppable() && !e->getAttrs()->isKillable())
                return this->beamBlocked(mdir);
        }
        // fill the beam with boubas where they are missing
        for (coords p = pos + step;; p = p + step) {
            const auto &e = board->topAt(p);
            if (!e || e->getType() == this->getType())
                break;
            if (e->getAttrs()->isSteppable()) {
                if (e->getType() != bElemTypes::_boubaType)
                    this->makeBouba(board, mdir)->stepOnElement(board->getElement(p));
            } else if (auto st = e->getStats()->getSteppingOn();
                       st && st->getType() != bElemTypes::_boubaType) {
                auto ne = this->makeBouba(board, mdir);
                ne->stepOnElement(st);
                this->registerLiveElement(ne);
            }
        }
    }
    this->getStats()->setWaiting(kikiSpace::kikiWaitTime);
    return true;
}

std::shared_ptr<bElem> kiki::makeBouba(const std::shared_ptr<chamber> &board, dir::direction mdir)
{
    auto ne = elementFactory::generateAnElement<bouba>(board, 0);
    ne->getStats()->setMyDirection(mdir);
    ne->getStats()->setFacing(mdir);
    return ne;
}

bool kiki::beamBlocked(dir::direction mdir)
{
    this->getStats()->setMyDirection(dir::direction::NODIRECTION);
    this->getStats()->setFacing(dir::direction::NODIRECTION);
    auto e = this->getElementInDirection(mdir);
    while (e
           && (e->getType() != this->getType()
               || (e->getType() == this->getType()
                   && e->getStats()->getMyDirection()
                          != dir::getOppositeDirection(this->getStats()->getMyDirection())
                   && e->getStats()->getMyDirection() != dir::direction::NODIRECTION
                   && e->getAttrs()->getSubtype() != this->getType() + 1))) {
        if (e->getType() == bElemTypes::_boubaType
            && ((dir::direction) e->getStats()->getMyDirection() == this->direction)) {
            auto e1 = e->getElementInDirection(mdir);
            e->disposeElement();
            e = e1;
            continue;
        }
        e = e->getElementInDirection(mdir);
    }
    return false;
}

int kiki::getType() const
{
    return bElemTypes::_kikiType;
}

/**
 * @brief stepOnElement for kiki, handles movement and creation of new elements based on certain conditions.
 *
 *
 * This method is called when the 'kiki' element steps on another element. It first checks if the basic element
 * functionality (inherited from `bElem`) is executed correctly. If not, it returns `false`.
 * If the 'kiki' element's subtype is even, it will attempt to move in a random direction and perform additional
 * actions based on the elements it encounters.
 *
 * Specifically:
 * - A random direction is chosen from all possible directions.
 * - The element's direction is updated, and it registers itself as a live element.
 * - The element continues moving in the chosen direction as long as it encounters elements that are steppable.
 * - Once it reaches a point where it cannot step further, a new 'terminator' element is created and moved in the
 *   opposite direction of the chosen direction.
 * - This new element is then placed on the board and steps onto the last encountered element.
 *
 * @param step A shared pointer to the element that is being stepped on.
 * @return `true` if the action was successfully performed, `false` otherwise.
 */
bool kiki::stepOnElement(std::shared_ptr<bElem> step)
{
    // Perform basic element behavior (from bElem class).
    if (!bElem::stepOnElement(step))
        return false;

    // If the subtype of 'kiki' is even, proceed with additional behavior.
    if (this->getAttrs()->getSubtype() % 2 == 0) {
        myUtility::Coords mycoords(this->getStats()->getMyPosition());
        std::vector<dir::direction> dirs{dir::direction::DOWN,
                                         dir::direction::UP,
                                         dir::direction::LEFT,
                                         dir::direction::RIGHT};
        for (auto it = 0; it < dirs.size();) {
            if (this->getBoard()->calculateLine(mycoords, dirs[it]) < 5) {
                dirs.erase(dirs.begin() + it);
                continue;
            }
            it++;
        }
        if (dirs.size() == 0) {
            this->getAttrs()->setSubtype(this->getType() + 1);
            this->getStats()->setFacing(dir::direction::NODIRECTION);
            this->getStats()->setMyDirection(dir::direction::NODIRECTION);
            return true;
        }
        auto md = dirs[this->randomNumberGenerator() % dirs.size()];

        // Update the element's facing direction and its primary movement direction.
        this->getStats()->setFacing(md);
        this->getStats()->setMyDirection(md);
        this->direction = md;
        // Check the element in the direction of movement.
        // Register the element as a live element on the board.
        auto e = this->getElementInDirection(md);
        auto e1 = e;
        // Continue moving while encountering steppable elements.
        while (e && e->getAttrs()->isSteppable()) {
            // Move further in the same direction.
            e1 = e->getElementInDirection(md);

            // If we can't move further, create a 'terminator' element.
            if (!e1 || !e1->getAttrs()->isSteppable()) {
                // Generate the 'terminator' element and set its direction.
                auto terminator = elementFactory::generateAnElement<kiki>(this->getBoard(),
                                                                          this->getType() + 1);
                terminator->getStats()->setMyDirection(dir::getOppositeDirection(md));
                terminator->getStats()->setFacing(dir::getOppositeDirection(md));
                // Step on the last valid element.
                terminator->stepOnElement(e);
                break;
            }

            // Move to the next steppable element.
            e = e1;
        }

        this->registerLiveElement(shared_from_this());
        this->mechanics();
    }

    return true; // Action was successfully performed.
}
