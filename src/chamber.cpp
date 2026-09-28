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

#include "chamber.h"
#include "floorElement.h"
#include "player.h"

std::atomic<int> chamber::lastid = 0;
std::vector<std::shared_ptr<chamber>> chamber::allChambers;
std::recursive_mutex chamber::worldMutex;
std::atomic<bool> chamber::worldLockWanted{false};

std::shared_ptr<chamber> chamber::makeNewChamber(coords csize)
{
#ifdef _VerbousMode_
    std::cout << "generate chamber" << csize.x << "," << csize.y << "\n";
#endif
    return makeNewChamber(myUtility::Coords(csize.x, csize.y));
}

std::shared_ptr<chamber> chamber::makeNewChamber(myUtility::Coords csize)
{
#ifdef _VerbousMode_
    std::cout << "generate chamber" << csize.getX() << "," << csize.getY() << "\n";
#endif
    std::shared_ptr<chamber> c = std::make_shared<chamber>(csize.getX(), csize.getY());
#ifdef _VerbousMode_
    std::cout << "generated object\n";
#endif
    c->createFloor();
    std::lock_guard<std::recursive_mutex> lock(chamber::worldMutex);
    chamber::allChambers.push_back(c);
    return c;
}

void chamber::createFloor()
{
    this->visitedElements.assign((std::size_t) this->width * this->height, 555);
    this->cells.resize((std::size_t) this->width * this->height);
    for (int c = 0; c < this->width; c++) {
        for (int d = 0; d < this->height; d++) {
            int subtype = 0;
            if (bElem::randomNumberGenerator() % 10 == 0)
                subtype = 1;
            if (bElem::randomNumberGenerator() % 100 == 0)
                subtype = 2;
            auto floor = elementFactory::generateAnElement<floorElement>(shared_from_this(), subtype);
            floor->setBoard(shared_from_this());
            floor->getStats()->setMyPosition(coords(c, d));
            floor->getAttrs()->setSubtype(subtype);
            this->cells[this->cellIndex(c, d)] = std::move(floor);
        }
    }
}

coords chamber::getSizeOfChamber()
{
    return this->cells.empty() ? coords(0, -1) : coords(this->width, this->height);
}

chamber::chamber(int x, int y)
    : std::enable_shared_from_this<chamber>()
    , width(x)
    , height(y)
{
    std::shared_ptr<randomWordGen> rwg = std::make_shared<randomWordGen>();
    this->setInstanceId(chamber::lastid++);
    this->chamberName = rwg->generateWord(3);
    this->chamberColour.a = 255;
    this->chamberColour.r = 30 + rwg->randomNumberGenerator() % 50;
    this->chamberColour.g = 30 + rwg->randomNumberGenerator() % 50;
    this->chamberColour.b = 50 + rwg->randomNumberGenerator() % 70;
    //this->createFloor();
}

chamber::chamber(coords csize)
    : chamber(csize.x, csize.y)
{}

colour chamber::getChColour()
{
    return this->chamberColour;
}

chamber::~chamber() {}

std::string chamber::getName()
{
    return this->chamberName;
}

bool chamber::visitPosition(coords point)
{
    bool res = false;
    if (point == NOCOORDS)
        return false;
    const int vradius = player::getActivePlayer()->getViewRadius() / 2;
    int x0 = ((point.x - vradius) < 0)
                 ? 0
                 : ((point.x - vradius >= this->width) ? this->width - 1 : point.x - vradius);
    int y0 = ((point.y - vradius) < 0)
                 ? 0
                 : ((point.y - vradius >= this->height) ? this->height - 1 : point.y - vradius);
    int x1 = ((point.x + vradius) < 0)
                 ? 0
                 : ((point.x + vradius >= this->width) ? this->width - 1 : point.x + vradius);
    int y1 = ((point.y + vradius) < 0)
                 ? 0
                 : ((point.y + vradius >= this->height) ? this->height - 1 : point.y + vradius);
    for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
            float distance = point.distance(coords(x, y));
            int &seen = this->visitedElements[this->cellIndex(x, y)];
            if (distance <= vradius && seen != 0) {
                res = true;
                seen = 0;
            }
        }
    }
    return res;
}

void chamber::setVisible(coords point, int v)
{
    if (point.x >= 0 && point.y >= 0 && point.x < this->width && point.y < this->height)
        this->visitedElements[this->cellIndex(point.x, point.y)] = v;
}

int chamber::isVisible(int x, int y)
{
    return this->isVisible(coords(x, y));
}

int chamber::isVisible(coords point)
{
    if (point.x < this->width && point.y < this->height && point.x >= 0 && point.y >= 0)
        return this->visitedElements[this->cellIndex(point.x, point.y)];
    return false;
}

void chamber::setElement(int x, int y, std::shared_ptr<bElem> elem)
{
    if (!elem || x < 0 || x > this->width - 1 || y < 0 || y > this->height - 1)
        return;
    if (this->cells.empty())
        return;
    elem->setBoard(shared_from_this());
    this->cells[this->cellIndex(x, y)] = std::move(elem);
}

void chamber::setElement(coords point, std::shared_ptr<bElem> elem)
{
    this->setElement(point.x, point.y, elem);
}

int chamber::getInstanceId()
{
    return this->instanceid;
}

void chamber::setInstanceId(int id)
{
    this->instanceid = id;
}

bool chamber::registerLiveElem(std::shared_ptr<bElem> in)
{
    auto iid = in->getStats()->getInstanceId();

    if (!in->getBoard())
        return false;
    for (unsigned int c = 0; c < in->getBoard()->toDeregister.size();)
        if (in->getBoard()->toDeregister[c] == iid)
            in->getBoard()->toDeregister.erase(in->getBoard()->toDeregister.begin() + c);
        else
            c++;

    this->liveElems.push_back(in);
    return true;
}

bool chamber::deregisterLiveElem(std::shared_ptr<bElem> in)
{
    if (in->getStats()->hasActivatedMechanics() && in->getBoard())
        in->getBoard()->toDeregister.push_back(in->getStats()->getInstanceId());
    in->getStats()->setActivatedMechanics(false);
    return true;
}

coords chamber::getSize()
{
    return coords(this->width, this->height);
}

myUtility::Coords chamber::getSizeCrd()
{
    return myUtility::Coords(this->width, this->height);
}

int chamber::calculateLine(myUtility::Coords position, dir::direction Odir)
{
    auto el = this->getLastInLine(position, Odir);
    if (el)
        return el->getStats()->getMyPosition().distance(coords(position.getX(), position.getY()));
    return 0;
}

std::shared_ptr<bElem> chamber::getElement(myUtility::Coords point)
{
    return getElement(point.getX(), point.getY());
}

std::shared_ptr<bElem> chamber::getLastInLine(myUtility::Coords pos, dir::direction mydir)
{
    auto el = this->getElement(pos);
    std::shared_ptr<bElem> el1 = el;
    int c = 0;
    el1 = el->getElementInDirection(mydir);
    while (el1 && el1->getAttrs()->isSteppable()) {
        el1 = el1->getElementInDirection(mydir);
        c++;
    }
    return el1;
}
