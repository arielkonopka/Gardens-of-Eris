
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

#ifndef BUNKER_H
#define BUNKER_H
#include "commons.h"
#include "elementFactory.h"
#include "plainGun.h"
#include "bElem.h"


class bunker : public bElem
{
public:
    using bElem::additionalProvisioning;

    bunker();
    ~bunker() override = default;
    bool mechanics() final;
    dir::direction findLongestShot();
    bool selfAlign() override;
    int getType() const override;
    bool interact(std::shared_ptr<bElem> Who) final;

    bool additionalProvisioning(int subtype) final;
private:
    std::shared_ptr<bElem> activatedBy=nullptr;
    int help=0;
    std::shared_ptr<bElem> myGun;
    int sndHummHandle=-1;
    const int brange=10;
};

#endif // BUNKER_H
