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






bool kiki::mechanics() {
     if(!bElem::mechanics())
        return false;
    auto pos=this->getStats()->getMyPosition();
    const auto mdir=this->direction;
    auto e=this->getElementInDirection(mdir);

    while (e && e->getType()!=this->getType())
    {
        if((!e->getAttrs()->isSteppable())&&(!e->getAttrs()->isKillable()))
        {
            this->getStats()->setMyDirection(dir::direction::NODIRECTION);
            this->getStats()->setFacing(dir::direction::NODIRECTION);
            e=this->getElementInDirection(mdir);
            while ( e && e->getType()!= this->getType())
            {
                if(e->getType()==bElemTypes::_boubaType && ((dir::direction)e->getStats()->getMyDirection()==this->direction))
                {
                    auto e1=e->getElementInDirection(mdir);
                    e->disposeElement();
                    e=e1;
                    continue;
                }
                e=e->getElementInDirection(mdir);
            }
            return false;
        }
        e=e->getElementInDirection(mdir);
    }
    e=this->getElementInDirection(mdir);
    while ( e && e->getType()!=this->getType())
    {
        if (!e->getAttrs()->isSteppable() && e->getAttrs()->isKillable())
        {
            e->hurt(kikiSpace::kikiHurts);
        }
        if(e->getType()!=bElemTypes::_boubaType)
        {
            /**
             * @brief place boubas on steppable elements
             *
             */
            pos=e->getStats()->getMyPosition();
            auto ne=elementFactory::generateAnElement<bouba>(this->getBoard(),0);
            ne->getStats()->setMyDirection(mdir);
            ne->getStats()->setFacing(mdir);
            ne->stepOnElement(this->getBoard()->getElement(pos));
          //  registerLiveElement(ne);
            e=ne;
        }
        e->getStats()->setWaiting(boubaSpace::boubaRefresh);
        e=e->getElementInDirection(mdir);
    }
    this->getStats()->setWaiting(kikiSpace::kikiWaitTime);
    return true;
}

int kiki::getType() const {
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
    if(!bElem::stepOnElement(step))
        return false;

    // If the subtype of 'kiki' is even, proceed with additional behavior.
    if(this->getAttrs()->getSubtype() % 2 == 0) {

        // Randomly select a direction from all possible directions.
        auto md = dir::allDirections[randomNumberGenerator() % dir::allDirections.size()];
        int c=5;
        while (!this->getElementInDirection(md)->getAttrs()->isSteppable() && c>0)
        {
            md = dir::allDirections[randomNumberGenerator() % dir::allDirections.size()];
            c--;
        }
        // Update the element's facing direction and its primary movement direction.
        this->getStats()->setFacing(md);
        this->getStats()->setMyDirection(md);
        this->direction=md;
        // Register the element as a live element on the board.

        // Check the element in the direction of movement.
        auto e = this->getElementInDirection(md);
        auto e1 = e;

        // Continue moving while encountering steppable elements.
        while (e && e->getAttrs()->isSteppable()) {
            // Move further in the same direction.
            e1 = e->getElementInDirection(md);

            // If we can't move further, create a 'terminator' element.
            if (!e1 || !e1->getAttrs()->isSteppable()) {
                // Generate the 'terminator' element and set its direction.
                auto terminator = elementFactory::generateAnElement<kiki>(this->getBoard(), this->getType() + 1);
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
    }

    return true; // Action was successfully performed.
}
