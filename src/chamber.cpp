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
#include "elementFactory.h"

std::atomic<int> chamber::lastid = 0;
std::vector<std::shared_ptr<chamber>> chamber::allChambers;
std::recursive_mutex chamber::worldMutex;

std::shared_ptr<chamber> chamber::makeNewChamber(coords csize)
{
    return makeNewChamber(myUtility::Coords(csize.x, csize.y));
}

std::shared_ptr<chamber> chamber::makeNewChamber(myUtility::Coords csize)
{
    std::shared_ptr<chamber> c = std::make_shared<chamber>(csize.getX(), csize.getY());
    for (int x = 0; x < c->width; x += chunkSize)
        for (int y = 0; y < c->height; y += chunkSize)
            c->createFloor(chunkOf(coords(x, y)));
    std::lock_guard<std::recursive_mutex> lock(chamber::worldMutex);
    chamber::allChambers.push_back(c);
    return c;
}

std::shared_ptr<chamber> chamber::makeWorld()
{
    std::shared_ptr<chamber> c = std::make_shared<chamber>(0, 0);
    c->makeEndless();
    std::lock_guard<std::recursive_mutex> lock(chamber::worldMutex);
    chamber::allChambers.push_back(c);
    return c;
}

chamber::fence::fence(chamber &board, coords lo, coords hi)
    : board(board)
    , savedLo(board.limitLo)
    , savedHi(board.limitHi)
{
    board.limitLo = coords(std::max(lo.x, savedLo.x), std::max(lo.y, savedLo.y));
    board.limitHi = coords(std::min(hi.x, savedHi.x), std::min(hi.y, savedHi.y));
}

chamber::fence::~fence()
{
    this->board.limitLo = this->savedLo;
    this->board.limitHi = this->savedHi;
}

void chamber::makeEndless()
{
    this->bounded = false;
    this->limitLo = coords(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());
    this->limitHi = coords(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
}

bool chamber::hasChunk(coords chunkKey) const
{
    return this->chunkByKey.contains(keyOf(chunkKey));
}

void chamber::addChunk(coords chunkKey)
{
    if (this->bounded || this->hasChunk(chunkKey))
        return;
    this->createFloor(chunkKey);
}

chamber::chunk &chamber::chunkAt(coords chunkKey)
{
    const auto key = keyOf(chunkKey);
    auto it = this->chunkByKey.find(key);
    if (it != this->chunkByKey.end())
        return *this->chunks[it->second];
    this->chunkByKey.emplace(key, this->chunks.size());
    this->keys.push_back(chunkKey);
    this->chunks.push_back(std::make_unique<chunk>());
    return *this->chunks.back();
}

void chamber::createFloor(coords chunkKey)
{
    chunk &ch = this->chunkAt(chunkKey);
    const coords first = chunkOrigin(chunkKey);
    for (int x = first.x; x < first.x + chunkSize; x++) {
        for (int y = first.y; y < first.y + chunkSize; y++) {
            if (this->bounded && (x >= this->width || y >= this->height))
                continue;
            int subtype = 0;
            if (goe::rng::gameplay()() % 10 == 0)
                subtype = 1;
            if (goe::rng::gameplay()() % 100 == 0)
                subtype = 2;
            auto floor = elementFactory::generateAnElement<floorElement>(shared_from_this(), subtype);
            floor->setBoard(shared_from_this());
            floor->getStats()->setMyPosition(coords(x, y));
            floor->getAttrs()->setSubtype(subtype);
            ch.cells[cellIndex(coords(x, y))] = std::move(floor);
        }
    }
}

coords chamber::getSizeOfChamber()
{
    return this->chunks.empty() ? coords(0, -1) : coords(this->width, this->height);
}

chamber::chamber(int x, int y)
    : std::enable_shared_from_this<chamber>()
    , width(x)
    , height(y)
    , limitHi(x - 1, y - 1)
{
    std::shared_ptr<randomWordGen> rwg = std::make_shared<randomWordGen>();
    this->setInstanceId(chamber::lastid++);
    this->chamberName = rwg->generateWord(3);
    this->chamberColour.a = 255;
    this->chamberColour.r = 30 + goe::rng::gameplay()() % 50;
    this->chamberColour.g = 30 + goe::rng::gameplay()() % 50;
    this->chamberColour.b = 50 + goe::rng::gameplay()() % 70;
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
    for (int x = point.x - vradius; x <= point.x + vradius; x++) {
        for (int y = point.y - vradius; y <= point.y + vradius; y++) {
            const coords cell(x, y);
            const auto idx = this->chunkIndex(cell);
            if (idx < 0)
                continue;
            int &seen = this->chunks[(std::size_t) idx]->visited[cellIndex(cell)];
            if (seen != 0 && point.distance(cell) <= vradius) {
                res = true;
                seen = 0;
            }
        }
    }
    return res;
}

void chamber::setVisible(coords point, int v)
{
    if (const auto idx = this->chunkIndex(point); idx >= 0)
        this->chunks[(std::size_t) idx]->visited[cellIndex(point)] = v;
}

int chamber::isVisible(int x, int y)
{
    return this->isVisible(coords(x, y));
}

int chamber::isVisible(coords point)
{
    if (const auto idx = this->chunkIndex(point); idx >= 0)
        return this->chunks[(std::size_t) idx]->visited[cellIndex(point)];
    return false;
}

void chamber::setElement(int x, int y, std::shared_ptr<bElem> elem)
{
    const coords cell(x, y);
    const auto idx = this->chunkIndex(cell);
    if (!elem || idx < 0)
        return;
    elem->setBoard(shared_from_this());
    this->chunks[(std::size_t) idx]->cells[cellIndex(cell)] = std::move(elem);
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
    return this->bounded ? coords(this->width, this->height) : coords(0, 0);
}

myUtility::Coords chamber::getSizeCrd()
{
    const coords size = this->getSize();
    return myUtility::Coords(size.x, size.y);
}

int chamber::calculateLine(myUtility::Coords position, dir::direction Odir)
{
    auto el = this->getLastInLine(position, Odir);
    if (el)
        return el->getStats()->getMyPosition().distance(coords(position.getX(), position.getY()));
    return 0;
}

std::shared_ptr<bElem> chamber::getElement(myUtility::Coords point) const
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

void chamber::place(const std::shared_ptr<bElem> &elem, const std::shared_ptr<bElem> &onto)
{
    auto &st = *elem->getStats();
    auto &below = *onto->getStats();
    elem->setBoard(onto->getBoard());
    st.setMyPosition(below.getMyPosition());
    std::shared_ptr<bElem> above = below.hasParent() ? below.getStandingOn().lock() : nullptr;
    st.setSteppingOn(onto);
    below.setStandingOn(elem);
    if (above) {
        // slide in between onto and what already stood on it
        above->getStats()->setSteppingOn(elem);
        st.setStandingOn(above);
    } else {
        st.setStandingOn(std::weak_ptr<bElem>());
        elem->getBoard()->setElement(st.getMyPosition(), elem);
    }
}

void chamber::lift(const std::shared_ptr<bElem> &elem)
{
    auto &st = *elem->getStats();
    auto board = elem->getBoard();
    const coords at = st.getMyPosition();
    auto below = st.getSteppingOn();
    if (st.hasParent()) {
        auto above = st.getStandingOn().lock();
        above->getStats()->setSteppingOn(below);
        if (below)
            below->getStats()->setStandingOn(above);
    } else if (below) {
        board->setElement(at, below);
        below->getStats()->setHasParent(false);
    } else {
        // the last element of a stack left; a cell is never empty
        auto floor = elementFactory::generateAnElement<floorElement>(board, 555);
        floor->getStats()->setMyPosition(at);
        board->setElement(at, floor);
    }
    // a lifted element is on no stack, so it keeps no links into one
    st.setSteppingOn(nullptr);
    st.setStandingOn(std::weak_ptr<bElem>());
    elem->setBoard(nullptr);
    st.setMyPosition(NOCOORDS);
}
