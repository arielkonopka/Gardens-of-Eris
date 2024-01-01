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

#ifndef MONSTER_H
#define MONSTER_H
#include "objectTypes.h"
#include "chamber.h"
#include "videoElementDef.h"
#include "plainGun.h"
#include "bazooka.h"

class monster : public bElem
{
public:

    monster();
    explicit monster(std::shared_ptr<chamber> board);
    explicit monster(std::shared_ptr<chamber> board,int newSubtype);
    virtual ~monster()=default;
    int getType() const;
    virtual bool mechanics();
    virtual bool checkNeigh();
    virtual bool steppableNeigh();
    virtual bool additionalProvisioning(int subtype,std::shared_ptr<monster> sbe);

private:
    std::shared_ptr<plainGun> weapon=nullptr;
    bool inited=false;
    int rotA=3;
    int rotB=1;

};

#endif // MONSTER_H
