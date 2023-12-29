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

#ifndef TELEPORT_H
#define TELEPORT_H

#include "commons.h"
#include "videoElementDef.h"
#include "randomLevelGenerator.h"
#include "bElem.h"
class teleport : public bElem
{
public:
    int getType() const;
    teleport();
    explicit teleport(std::shared_ptr<chamber> board);
    explicit teleport(std::shared_ptr<chamber> board,int newSubtype);
    virtual ~teleport()=default;
    bool interact(std::shared_ptr<bElem> who) final;
    virtual bool teleportIt(std::shared_ptr<bElem> who);
    virtual oState disposeElement();
    virtual oState disposeElementUnsafe();
    virtual bool createConnectionsWithinSUbtype();
    bool additionalProvisioning(int value,std::shared_ptr<teleport> t);
    bool mechanics() final;
    virtual bool additionalProvisioning();
    virtual bool additionalProvisioning(int subtype,int typeId);
    bool stepOnAction(bool step,std::shared_ptr<bElem> who);
private:
    static std::vector<std::weak_ptr<teleport>> allTeleporters;
    bool removeFromAllTeleporters();
    bool connectionsMade=false;
    bool checked=false;
    std::shared_ptr<teleport> theOtherEnd;
};

#endif // TELEPORT_H
