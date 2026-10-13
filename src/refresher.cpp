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
#include "refresher.h"

int refresher::getType() const
{
    return bElemTypes::_refresherType;
}

bool refresher::destroy()
{
    if (!this->getStats()->isDestroying())
        return this->explode(2.5);
    return false;
}

bool refresher::kill()
{
    return this->destroy();
}

void refresher::feed(bElem &who)
{
    if (this->getAttrs()->getEnergy() <= 0)
        return; // drained: it is blowing up
    this->getStats()->setWaiting(refresher::pause);
    who.getAttrs()->setEnergy(who.getAttrs()->getEnergy() + refresher::dose);
    this->hurt(refresher::dose);
}

bool refresher::mechanics()
{
    if (!this->getStats()->isActive() || !explosives::mechanics())
        return false;
    const std::shared_ptr<bElem> carrier = this->getStats()->getCollector().lock();
    if (!this->getStats()->isCollected() || !carrier || !carrier->getBoard())
        return false;
    if (carrier->getAttrs()->getEnergy() < refresher::fullUp)
        this->feed(*carrier);
    return true;
}

bool refresher::interact(std::shared_ptr<bElem> who)
{
    if (!who || !bElem::interact(who))
        return false;
    this->getStats()->setInteracted(refresher::pause);
    this->feed(*who);
    return true;
}

/// carried, it starts feeding (only the first one its carrier holds); dropped, it stops
bool refresher::collectOnAction(bool collected, std::shared_ptr<bElem> who)
{
    if (collected && who) {
        if (who->getAttrs()->getInventory()->countTokens(this->getType(), this->getAttrs()->getSubtype()) <= 1)
            this->getStats()->setActive(true);
        this->registerLiveElement(shared_from_this());
    } else {
        this->getStats()->setActive(false);
        this->deregisterLiveElement(this->getStats()->getInstanceId());
        this->getStats()->setActivatedMechanics(false);
    }
    return bElem::collectOnAction(collected, who);
}
