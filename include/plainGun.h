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

#ifndef PLAINGUN_H
#define PLAINGUN_H
//#include "commons.h"
#include "videoElementDef.h"
#include "plainMissile.h"
#include "soundManager.h"
//#include "viewPoint.h"
class plainGun : public bElem
{
public:

    virtual int getType() const;
    virtual bool use(std::shared_ptr<bElem> who);
    plainGun()=default;
    virtual bool mechanics();
    virtual ~plainGun() =default;
    virtual std::shared_ptr<bElem> createProjectible(std::shared_ptr<bElem> who);
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<plainGun> sbe);

private:
    int ammo=1+(bElem::randomNumberGenerator()%_plainGunAmmo);
    unsigned int shot=0;
    int maxEnergy=20*(1+(bElem::randomNumberGenerator()&3));

};

#endif // PLAINGUN_H
