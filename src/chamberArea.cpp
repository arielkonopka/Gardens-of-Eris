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
#include "chamberArea.h"

chamberArea::chamberArea(int xu, int yu, int xd, int yd)
    : upLeft(xu, yu)
    , downRight(xd, yd)
{}

void chamberArea::addChildNode(std::unique_ptr<chamberArea> child)
{
    this->children.push_back(std::move(child));
}

std::optional<chamberArea::areaRef> chamberArea::parentOf(const chamberArea &area)
{
    for (auto &child : this->children) {
        if (child.get() == &area)
            return std::ref(*this);
        if (auto parent = child->parentOf(area))
            return parent;
    }
    return std::nullopt;
}

bool chamberArea::removeArea(const chamberArea &area)
{
    for (auto it = this->children.begin(); it != this->children.end(); ++it) {
        if (it->get() == &area)
            this->children.erase(it);
        else if (!(*it)->removeArea(area))
            continue;
        // the area was below this node, so this node's surface shrinks with it
        this->surface = 0;
        for (const auto &child : this->children)
            this->surface += child->surface;
        return true;
    }
    return false;
}

// We calculate the nodes area, if it has no children, we assume it is the chamber without walls;
int chamberArea::calculateInitialSurface()
{
    int s = 0;
    if (!this->children.empty()) {
        for (unsigned int cnt = 0; cnt < this->children.size(); cnt++) {
            s = s + this->children[cnt]->calculateInitialSurface();
        }

    } else {
        int dx = this->downRight.x - this->upLeft.x;
        int dy = this->downRight.y - this->upLeft.y;
        s = dx * dy;
    }
    this->surface = s;
    return s;
}

//calculate surfaces in the children nodes
int chamberArea::calculateSurface(std::shared_ptr<chamber> mychamber)
{
    int surface_ = 0;
    if (this->children.empty()) {
        for (int x = this->upLeft.x; x <= this->downRight.x; x++) {
            for (int y = this->upLeft.y; y <= this->downRight.y; y++) {
                if (mychamber->getElement(x, y)->getAttrs()->isSteppable()
                    && this->checkIfElementIsFree(x, y, mychamber)) {
                    surface_++;
                }
            }
        }
    } else {
        for (unsigned int cnt = 0; cnt < this->children.size(); cnt++) {
            surface_ += this->children[cnt]->calculateSurface(mychamber);
        }
    }
    this->surface = surface_;
    return surface_;
}

void chamberArea::findElementsRec(const std::shared_ptr<chamber> &mychamber,
                                  std::vector<std::shared_ptr<bElem>> &found) const
{
    if (!this->children.empty()) {
        for (const auto &child : this->children)
            child->findElementsRec(mychamber, found);
        return;
    }
    for (int x = this->upLeft.x; x <= this->downRight.x; x++)
        for (int y = this->upLeft.y; y <= this->downRight.y; y++) {
            auto elem = mychamber->getElement(x, y);
            if (elem->getType() == bElemTypes::_floorType)
                found.push_back(std::move(elem));
        }
}

std::vector<std::shared_ptr<bElem>> chamberArea::findElementsToStepOn(
    std::shared_ptr<chamber> myChamber) const
{
    std::vector<std::shared_ptr<bElem>> found;
    this->findElementsRec(myChamber, found);
    return found;
}

void chamberArea::findChambersRec(int s, std::vector<areaRef> &found)
{
    bool last = true;
    if (this->surface > s) {
        for (auto &child : this->children) {
            if (child->surface >= s) {
                child->findChambersRec(s, found);
                last = false;
            }
        }
    }
    if (last && this->surface >= s)
        found.push_back(std::ref(*this));
}

std::vector<chamberArea::areaRef> chamberArea::findChambersCloseToSurface(int s, int /*tolerance*/)
{
    std::vector<areaRef> found;
    this->findChambersRec(s, found);
    return found;
}

bool chamberArea::checkIfElementIsFree(int x, int y, std::shared_ptr<chamber> mychamber)
{
    if (mychamber->getElement(x, y)->getAttrs()->isSteppable() == false)
        return false;
    sNeighboorhood neigh = mychamber->getElement(x, y)->getSteppableNeighborhood();
    for (int c = 0; c < 8; c++) {
        if (neigh.nTypes[c] == bElemTypes::_door)
            neigh.steppable[c] = true;
    }
    for (int c = 0; c < 8; c++) {
        if (c % 2 == 0) {
            if (neigh.steppable[c] == false && neigh.steppable[(c + 4) % 8] == false)
                return false;
            if (neigh.steppable[c] == false && neigh.steppable[(c + 2) % 8] == true
                && neigh.steppable[(c + 3) % 8] == false)
                return false;
        } else {
            if (neigh.steppable[c] == false && neigh.steppable[(c + 1) % 8] == true
                && neigh.steppable[(c + 2) % 8] == false)
                return false;
            if (neigh.steppable[c] == false && neigh.steppable[(c + 6) % 8] == false
                && neigh.steppable[(c + 7) % 8] == true)
                return false;
            if (neigh.steppable[c] == false && neigh.steppable[(c + 1) % 8] == true
                && neigh.steppable[(c + 3) % 8] == false)
                return false;
        }
    }
    return true;
}

// This should be run as a correct after deleting a node from the tree, we do not need nodes without a surface
void chamberArea::removeEmptyNodes()
{
    for (auto &child : this->children)
        child->removeEmptyNodes();
    std::erase_if(this->children, [](const auto &child) { return child->surface == 0; });
}
