
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

#include "bazooka.h"

bazooka::bazooka():plainGun()
{
}



int bazooka::getType() const
{
    return _bazookaType;
}
std::shared_ptr<bElem> bazooka::createProjectible(std::shared_ptr<bElem>who)
{
    std::shared_ptr<bElem> pm=elementFactory::generateAnElement<bazookaMissile>(who->getBoard(),0);
    pm->getStats()->setStatsOwner(who);
    who->lockThisObject(pm);
    pm->getStats()->setMyDirection(who->getStats()->getFacing());
    pm->getStats()->setFacing(who->getStats()->getFacing());
    pm->stepOnElement(who->getElementInDirection(who->getStats()->getFacing()));
    pm->getAttrs()->setEnergy(this->getAttrs()->getEnergy());
    if(who->getType()==_player)
        viewPoint::get_instance()->setOwner(pm);
    pm->registerLiveElement(pm);
    return pm;
}


bool bazooka::additionalProvisioning(int subtype, std::shared_ptr<bazooka>sbe)
{
    return bElem::additionalProvisioning(subtype,sbe);
}
