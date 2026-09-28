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
#include <functional>
#include <memory>
#include <optional>
#include <vector>

/// A rectangle of the level, split recursively into child areas while the maze is generated.
/// Each area owns its children; areas are found and removed by identity.
class chamberArea
{
public:
    using areaRef = std::reference_wrapper<chamberArea>;

    chamberArea(int xu, int yu, int xd, int yd);
    void addChildNode(std::unique_ptr<chamberArea> child);
    coords upLeft;
    coords downRight;
    int surface = 0;
    std::vector<std::unique_ptr<chamberArea>> children;
    bool childrenLock = false;
    int calculateInitialSurface();
    int calculateSurface(std::shared_ptr<chamber> mychamber);
    /// floor cells inside the leaves of this area
    std::vector<std::shared_ptr<bElem>> findElementsToStepOn(std::shared_ptr<chamber> myChamber) const;
    /// the smallest areas that still offer at least s cells
    std::vector<areaRef> findChambersCloseToSurface(int s, int tolerance);
    bool checkIfElementIsFree(int x, int y, std::shared_ptr<chamber> mychamber);
    /// the area that holds `area` as a direct child; empty for this node itself or an unknown area
    std::optional<areaRef> parentOf(const chamberArea &area);
    /// drops `area` and its subtree, and recalculates the surfaces of its ancestors
    bool removeArea(const chamberArea &area);
    void removeEmptyNodes();

private:
    void findElementsRec(const std::shared_ptr<chamber> &mychamber,
                         std::vector<std::shared_ptr<bElem>> &found) const;
    void findChambersRec(int s, std::vector<areaRef> &found);
};

#endif // CHAMBERAREA_H
