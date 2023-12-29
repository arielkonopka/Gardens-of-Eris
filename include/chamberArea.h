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

#ifndef CHAMBERAREA_H
#define CHAMBERAREA_H
#include "commons.h"
#include "chamber.h"
#include "bElem.h"
#include <vector>
class chamberArea
{

public:
    static std::vector<chamberArea*> foundAreas;
    static std::vector<std::shared_ptr<bElem> > foundElements;
    chamberArea(int xu, int yu, int xd, int yd);
    virtual ~chamberArea();
    bool addChildNode(chamberArea *child);
    coords upLeft;
    coords downRight;
    int surface;
    std::vector<chamberArea* > children;
    chamberArea* parent;
    int calculateInitialSurface();
    int calculateSurface(std::shared_ptr<chamber> mychamber);
    bool findElementsToStepOn(std::shared_ptr<chamber> myChamber);
    void findElementsRec(std::shared_ptr<chamber>  mychamber);
    void findChambersCloseToSurface(int s,int tolerance);
    bool checkIfElementIsFree(int x,int y, std::shared_ptr<chamber> mychamber);
    bool childrenLock;
    void removeEmptyNodes();
};

#endif // CHAMBERAREA_H
