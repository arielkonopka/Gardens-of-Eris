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

#include "bouba.h"

/**
 * @brief Bouba mechanics
 * Although, the kikis should never be killed, this implementation assumes the kikis could be removed from the board, it just checks it rarely
 */

bool bouba::mechanics() {
    if(!bElem::mechanics())
        return false;
    this->disposeElement();
    return false;

}

int bouba::getType() const {
    return bElemTypes::_boubaType;
}

bool bouba::stepOnAction(bool step, std::shared_ptr<bElem> who)
{
    if(step)
    {
        this->disposeElement();
    }
    return bElem::stepOnAction(step, who);
}

int bouba::getAnimPh() const
{
    int ph=(this->getCntr()/3+this->getStats()->getInstanceId());
    return ph;
}
