
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

#ifndef DOOR_H
#define DOOR_H

#include <bElem.h>
#include "videoElementDef.h"
#include "wall.h"
#include "commons.h"

class door : public bElem
{
public:
    using bElem::additionalProvisioning;

    virtual int getType() const;
    door()=default;
    virtual bool stepOnAction(bool step,std::shared_ptr<bElem> who);
    bool destroy();
    virtual bool interact(std::shared_ptr<bElem> who);

    virtual ~door()=default;
    bool additionalProvisioning(int subtype,std::shared_ptr<door> sbe);

private:
    void _alignWithOpen();
//   int interacted=-1;

};

#endif // DOOR_H
