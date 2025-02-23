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
#include "monster.h"
monster::monster()
    : fNavigator(std::make_shared<fnordController::fnordNavigator>())
{}

bool monster::additionalProvisioning(int subtype)
{
    if (!bElem::additionalProvisioning(subtype))
        return false;

    return true;
}


int monster::getType() const
{
    return bElemTypes::_monster;
}

/**
 * @brief monster::mechanics we utilize here the fnordNavigator object, we have, this way, we do not have to deal with the monsters logic here
 * @return true if success, false if not. for ex. false could mean, the object is moving or waiting, generally not suitable for running the mechanics
 */
bool monster::mechanics()
{
    if (!bElem::mechanics())
        return false;
    this->fNavigator->attachBoard(this->getBoard());
    fnordEcho fec;
    fec = this->fNavigator->makeFnord(shared_from_this(), this->fNavigator->myFnord);
    this->fNavigator->myFnord = fec;
    auto chaosStep = this->fNavigator->makeUpMind(this->fNavigator);

    switch (chaosStep.action) {
    case fnordController::moveInfo::actionType::MOVE:
        this->moveInDirection(chaosStep.direction);
        break;
    case fnordController::moveInfo::actionType::ROTATE:
        this->getStats()->setMyDirection(chaosStep.direction);
        this->getStats()->setFacing(chaosStep.direction);
        this->getStats()->setWaiting(GoEConstants::_mov_delay);
        break;
    case fnordController::moveInfo::actionType::HOLD:
        this->getStats()->setWaiting(GoEConstants::_monstrHold);
        break;
    case fnordController::moveInfo::actionType::INTERACT: {
        std::shared_ptr<bElem> bbel = getElementInDirection(chaosStep.direction);
        if (bbel)
            bbel->interact(shared_from_this());
        this->getStats()->setWaiting(GoEConstants::_monsterWait);
        break;
    }
    case fnordController::moveInfo::actionType::ATTACK: {
        auto bEl = getElementInDirection(chaosStep.direction);
        if (bEl) {
            if (this->getAttrs()->canCollect() && this->getAttrs()->getInventory()->getActiveWeapon()) {
                auto awe = this->getAttrs()->getInventory()->getActiveWeapon();
                awe->use(shared_from_this());
            } else {
                bEl->hurt(GoEConstants::_monstrHurt);
            }
        }
        this->getStats()->setWaiting(GoEConstants::_monsterWait);
        break;
    }
    }

    return true;
}

bool monster::selfAlign()
{
    this->registerLiveElement(shared_from_this());
    return true;
}
