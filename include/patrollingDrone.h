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

#ifndef PATROLLINGDRONE_H
#define PATROLLINGDRONE_H

#include <bElem.h>
#include "commons.h"
#include "videoElementDef.h"
#include "viewPoint.h"
class patrollingDrone : public bElem
{
public:
    using bElem::additionalProvisioning;

    patrollingDrone()=default;
    ~patrollingDrone() override=default;
    bool interact(std::shared_ptr<bElem> who) override;
    int getType() const override;
    float getViewRadius() const override;
    bool additionalProvisioning(int subtype) override;

private:
    bool brained=false;
    std::shared_ptr<bElem> brainModule;
};

#endif // PATROLLINGDRONE_H
