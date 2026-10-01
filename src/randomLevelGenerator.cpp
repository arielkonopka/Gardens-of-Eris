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

#include "randomLevelGenerator.h"
#include "difficulty.h"
#include "gameSerializer.h"
#include "player.h"

namespace {
/// gaps in a chunk wall are picked from these offsets, so a gap is never at a wall's corner
constexpr int firstGap = 1, lastGap = chamber::chunkSize - 2;
/// one global teleporter room in about every fifth chunk
constexpr int globalTeleporterOdds = difficulty::five;
/// local teleporters pair within a region of five by five chunks
constexpr int regionChunks = difficulty::five;
} // namespace

randomLevelGenerator::randomLevelGenerator(int w, int h, goe::rng::seed levelSeed)
    : eng(levelSeed)
{
    // the new chamber's name and colour come from this level's seed too
    goe::rng::generationScope scope(this->eng);
    this->hi = coords(w - 1, h - 1);
    std::lock_guard<std::recursive_mutex> worldLock(chamber::worldMutex);
    this->mychamber = chamber::makeNewChamber(myUtility::Coords(w, h));
}

randomLevelGenerator::randomLevelGenerator(std::shared_ptr<chamber> world, coords at)
    : eng(goe::rng::placeSeed(at.x, at.y))
    , mychamber(std::move(world))
    , chunk(at)
{
    this->lo = chamber::chunkOrigin(at);
    this->hi = this->lo + (chamber::chunkSize - 1);
    this->eastGaps = wallGaps(coords(at.x + 1, at.y), true);
    this->southGaps = wallGaps(coords(at.x, at.y + 1), false);
    // the floor's looks come from this chunk's seed too
    goe::rng::generationScope scope(this->eng);
    this->mychamber->addChunk(at);
}

std::vector<int> randomLevelGenerator::wallGaps(coords chunk, bool west)
{
    goe::rng::engine wallEngine(goe::rng::placeSeed(chunk.x, chunk.y, west ? 1 : 2));
    // a wall has as many gaps as the maze walls of the chunk that owns it have holes
    const int gaps = difficulty::mazeHoles(difficulty::chunkDepth(chunk));
    std::vector<int> offsets;
    for (int g = 0; g < gaps; g++)
        offsets.push_back(firstGap + (int) goe::rng::below(wallEngine, lastGap - firstGap + 1));
    return offsets;
}

std::shared_ptr<bElem> randomLevelGenerator::at(int x, int y) const
{
    // a chunk is built inside a fence (generateChunk), so the board has no cells outside lo..hi
    return this->mychamber->getElement(x, y);
}

bool randomLevelGenerator::steppableAt(int x, int y) const
{
    const auto e = this->at(x, y);
    return e && e->getAttrs()->isSteppable();
}

/* 이것은 순환 분할 구현입니다 */

std::unique_ptr<chamberArea> randomLevelGenerator::lvlGenerate(
    int x1, int y1, int x2, int y2, int depth, int holes)
{
    auto mychamberArea = std::make_unique<chamberArea>(x1, y1, x2, y2);
#ifdef _VerbousMode_
    std::cout << "create Chamber " << x1 << "," << y1 << " " << x2 << "," << y2 << "\n";
#endif
    std::vector<int> doorPlaces1, doorPlaces2;
    int Hmin_ = ((y2 - y1) / 2);
    int Wmin_ = ((x2 - x1) / 2);

    if (Hmin_ < Hmin)
        Hmin_ = Hmin;
    if (Wmin_ < Wmin)
        Wmin_ = Wmin;

    int c = x2, dc1 = -1, dc2 = -1;
    int d = y2, dd1 = -1, dd2 = -1;
    int dx, dy;
    dx = x2 - x1;
    dc1 = dx + 1;
    dy = y2 - y1;
    dd1 = dy + 1;
    c = x2;
    d = y2;

    if (depth < 0 || (dx < Wmin_ && dy < Hmin_)) {
#ifdef _VerbousMode_
        std::cout << "Depth deplated or size too small: (" << x1 << "," << y1 << ")->(" << x2 << ","
                  << y2 << ")\n";
#endif
        return mychamberArea;
    }

    // 수평 길이는 세로 길이의 절반 이상이어야 합니다.
    if (dx > Wmin_ && dx * 2 > dy) {
        c = (this->eng() % (dx - (Wmin_))) + x1 + (Wmin_ / 2); //find vertical divider location
        dc1 = c - x1;
        dc2 = x2 - c + 2;
    }
    if (dy > Hmin_ && dy * 2 > dx) {
        d = (this->eng() % (dy - (Hmin_))) + y1 + (Hmin_ / 2); // horizontal divider
        dd1 = d - y1;
        dd2 = y2 - d + 2;
    }
    // add children nodes first, then we will draw the overlay
    if (dc1 > Wmin && dd1 > Hmin) {
        mychamberArea->addChildNode(this->lvlGenerate(x1, y1, c, d, depth - 1, holes));
    }
    if (dc2 > Wmin && dd1 > Hmin) {
        mychamberArea->addChildNode(this->lvlGenerate(c + 2, y1, x2, d, depth - 1, holes));
    }

    if (dd2 > Hmin && dc1 > Wmin) {
        mychamberArea->addChildNode(this->lvlGenerate(x1, d + 2, c, y2, depth - 1, holes));
    }

    if (dd2 > Hmin && dc2 > Wmin) {
        mychamberArea->addChildNode(this->lvlGenerate(c + 2, d + 2, x2, y2, depth - 1, holes));
    }
    doorPlaces1.clear();
    doorPlaces2.clear();
    if (c != x2) // d>0 dvider exists
    {
        //we draw vertical line
        for (int a = y1; a <= y2; a++) {
            if (a != d && this->steppableAt(c, a) && this->steppableAt(c + 2, a)) {
                if (a < d + 2) {
                    doorPlaces1.push_back(a);
                } else {
                    doorPlaces2.push_back(a);
                }
            }
            if (this->steppableAt(c + 1, a)) {
                std::shared_ptr<bElem> newElement
                    = elementFactory::generateAnElement<wall>(this->mychamber, 0);
                newElement->stepOnElement(this->at(c + 1, a));
            } else {
                break;
            }
        }

        //now pick the door gap randomly, and make as many holes, as requested
        for (int cnt = 0; cnt < holes; cnt++) {
            if (doorPlaces1.size() > 0) {
                int rnd = this->eng() % (doorPlaces1.size());
                this->at(c + 1, doorPlaces1[rnd])->disposeElement();
                doorPlaces1[rnd] = doorPlaces1[doorPlaces1.size() - 1];
                doorPlaces1.pop_back();
            }
            if (doorPlaces2.size() > 0) {
                int rnd = this->eng() % (doorPlaces2.size());
                this->at(c + 1, doorPlaces2[rnd])->disposeElement();
                doorPlaces2[rnd] = doorPlaces2[doorPlaces2.size() - 1];
                doorPlaces2.pop_back();
            }
        }
    }
    //we draw horizontal line

    if (d != y2) // divider exists
    {
        doorPlaces1.clear();
        doorPlaces2.clear();
        for (int a = x1; a <= x2; a++) {
            if (a != c && this->steppableAt(a, d) && this->steppableAt(a, d + 2)) {
                if (a < c + 2) {
                    doorPlaces1.push_back(a);
                } else {
                    doorPlaces2.push_back(a);
                }
            }
            if (this->steppableAt(a, d + 1)) {
                std::shared_ptr<bElem> newElement
                    = elementFactory::generateAnElement<wall>(this->mychamber, 0);
                newElement->stepOnElement(this->at(a, d + 1));
            }
        }
        for (int cnt = 0; cnt < holes; cnt++) {
            if (!doorPlaces1.empty()) {
                int rnd = this->eng() % (doorPlaces1.size());
                this->at(doorPlaces1[rnd], d + 1)->disposeElement();
                doorPlaces1[rnd] = doorPlaces1[doorPlaces1.size() - 1];
                doorPlaces1.pop_back();
            }
        }
        for (int cnt = 0; cnt < holes; cnt++) {
            if (!doorPlaces2.empty()) {
                int rnd = this->eng() % (doorPlaces2.size());
                this->at(doorPlaces2[rnd], d + 1)->disposeElement();
                doorPlaces2[rnd] = doorPlaces2[doorPlaces2.size() - 1];
                doorPlaces2.pop_back();
            }
        }
    }

    return mychamberArea;
}

bool randomLevelGenerator::placeElementCollection(const chamberArea &chmbrArea,
                                                  const std::vector<elementToPlace> &elements)
{
    for (const auto &element : elements) {
        auto freeCells = chmbrArea.findElementsToStepOn(mychamber);
        // We sometimes must create more than one element
        for (int cnt = 0; cnt < element.number && !freeCells.empty(); cnt++) {
            const std::size_t selectedEl = goe::rng::below(this->eng, freeCells.size());
            std::shared_ptr<bElem> newElem = createElement(element);
            newElem->stepOnElement(freeCells[selectedEl]);
            newElem->selfAlign();
            freeCells[selectedEl] = std::move(freeCells.back());
            freeCells.pop_back();
        }
    }
    return true;
}

std::optional<chamberArea::areaRef> randomLevelGenerator::pickArea(int demandedSurface,
                                                                   int tolerance,
                                                                   bool lockable)
{
    auto found = this->headNode->findChambersCloseToSurface(demandedSurface, tolerance);
    if (lockable) {
        // prefer an area whose doors can all stand in its own walls
        std::vector<chamberArea::areaRef> sealable;
        for (const auto &area : found)
            if (!this->reachesNextChunkGap(area))
                sealable.push_back(area);
        if (!sealable.empty())
            found = std::move(sealable);
    }
    if (found.empty())
        return std::nullopt;
    return goe::rng::pick(this->eng, found);
}

bool randomLevelGenerator::frontsNextChunkGap(int x, int y) const
{
    auto listed = [](const std::vector<int> &gaps, int offset) {
        return std::find(gaps.begin(), gaps.end(), offset) != gaps.end();
    };
    return (x == this->hi.x && listed(this->eastGaps, y - this->lo.y))
           || (y == this->hi.y && listed(this->southGaps, x - this->lo.x));
}

bool randomLevelGenerator::reachesNextChunkGap(const chamberArea &area) const
{
    // the cell in front of such a gap is cleared, so it may lie in the area or in its walls
    for (int x = area.upLeft.x - 1; x <= area.downRight.x + 1; x++)
        for (int y = area.upLeft.y - 1; y <= area.downRight.y + 1; y++)
            if (this->frontsNextChunkGap(x, y))
                return true;
    return false;
}

void randomLevelGenerator::retireArea(const chamberArea &area)
{
    this->headNode->removeArea(area);
    this->headNode->removeEmptyNodes();
}

void randomLevelGenerator::buildMaze(int holes)
{
    // a bounded level has walls on all four sides; a chunk only on its west and north side,
    // the other two are the walls of the next chunks
    const bool bounded = this->chunk == NOCOORDS;
    this->headNode = this->lvlGenerate(this->lo.x + 1,
                                       this->lo.y + 1,
                                       bounded ? this->hi.x - 1 : this->hi.x,
                                       bounded ? this->hi.y - 1 : this->hi.y,
                                       _iterations,
                                       holes);
    this->headNode->calculateInitialSurface();
    if (!bounded) {
        this->buildChunkWalls();
        return;
    }
    for (int x = this->lo.x; x <= this->hi.x; x++) {
        elementFactory::generateAnElement<wall>(this->mychamber, 0)->stepOnElement(this->at(x, this->lo.y));
        elementFactory::generateAnElement<wall>(this->mychamber, 0)->stepOnElement(this->at(x, this->hi.y));
    }
    for (int y = this->lo.y; y <= this->hi.y; y++) {
        elementFactory::generateAnElement<wall>(this->mychamber, 0)->stepOnElement(this->at(this->lo.x, y));
        elementFactory::generateAnElement<wall>(this->mychamber, 0)->stepOnElement(this->at(this->hi.x, y));
    }
}

void randomLevelGenerator::buildChunkWalls()
{
    const auto westGaps = wallGaps(this->chunk, true);
    const auto northGaps = wallGaps(this->chunk, false);
    auto isGap = [](const std::vector<int> &gaps, int offset) {
        return std::find(gaps.begin(), gaps.end(), offset) != gaps.end();
    };
    for (int o = 0; o < chamber::chunkSize; o++) {
        if (!isGap(westGaps, o))
            elementFactory::generateAnElement<wall>(this->mychamber, 0)->stepOnElement(this->at(this->lo.x, this->lo.y + o));
        if (!isGap(northGaps, o))
            elementFactory::generateAnElement<wall>(this->mychamber, 0)->stepOnElement(this->at(this->lo.x + o, this->lo.y));
    }
    // a maze wall may run right past a gap; open the cell next to every gap, so each one leads in
    auto clear = [this](int x, int y) {
        if (!this->steppableAt(x, y))
            this->at(x, y)->disposeElement();
    };
    for (int g : westGaps)
        clear(this->lo.x + 1, this->lo.y + g);
    for (int g : northGaps)
        clear(this->lo.x + g, this->lo.y + 1);
    for (int g : this->eastGaps)
        clear(this->hi.x, this->lo.y + g);
    for (int g : this->southGaps)
        clear(this->lo.x + g, this->hi.y);
}

int randomLevelGenerator::localTeleporterSubtype() const
{
    if (this->chunk == NOCOORDS)
        return this->mychamber->getInstanceId() + 1;
    // one number per region, above 0 (0 is the global kind); regions up to 2^14 away stay apart
    const int rx = floorDiv(this->chunk.x, regionChunks) & 0x3FFF;
    const int ry = floorDiv(this->chunk.y, regionChunks) & 0x3FFF;
    return 1 + ((rx << 14) | ry);
}

bool randomLevelGenerator::generateLevel(int holes)
{
    // elements made while building this level draw their starting stats from its seed
    goe::rng::generationScope scope(this->eng);
    // keep saving and loading out while this level is being built
    std::lock_guard<std::recursive_mutex> worldLock(chamber::worldMutex);
    // publish this level's teleporters only once the level is complete
    teleport::registrationBatch teleporterBatch;
    this->buildMaze(holes);
    // fewer holes make a harder level
    return this->placeEverything(holes, std::max(0, difficulty::five - holes), true, true);
}

bool randomLevelGenerator::generateChunk(bool start, std::shared_ptr<const goe::chunkPattern> pattern)
{
    goe::rng::generationScope scope(this->eng);
    std::lock_guard<std::recursive_mutex> worldLock(chamber::worldMutex);
    teleport::registrationBatch teleporterBatch;
    // players found in a new chunk are spare avatars; only the start chunk's player takes over
    std::optional<player::backgroundScope> spareAvatars;
    if (!start)
        spareAvatars.emplace();
    // nothing built here may reach into the chunks next to this one
    chamber::fence onlyThisChunk(*this->mychamber, this->lo, this->hi);
    if (pattern)
        return this->fillChunk(*pattern, start);
    const int depth = difficulty::chunkDepth(this->chunk);
    const int holes = difficulty::mazeHoles(depth);
    this->buildMaze(holes);
    const bool globalTeleporter = start || goe::rng::below(this->eng, globalTeleporterOdds) == 0;
    return this->placeEverything(holes, depth, start, globalTeleporter);
}

bool randomLevelGenerator::fillChunk(const goe::chunkPattern &pattern, bool start)
{
    auto place = [this](int type, int subtype, int x, int y) {
        auto e = gameSerializer::createByType(type, subtype, this->mychamber);
        e->stepOnElement(this->at(x, y));
        e->selfAlign();
    };
    std::optional<coords> firstPlayer;
    // row by row, so the first player of the pattern is the one the game starts with
    for (int y = this->lo.y; y <= this->hi.y; y++)
        for (int x = this->lo.x; x <= this->hi.x; x++) {
            const goe::patternCell &cell = pattern.at(x - this->lo.x, y - this->lo.y);
            if (cell.type == bElemTypes::_belemType)
                continue;
            if (cell.type == bElemTypes::_floorType) {
                this->at(x, y)->getAttrs()->setSubtype(cell.subtype);
                continue;
            }
            place(cell.type, cell.subtype, x, y);
            if (cell.type == bElemTypes::_player && !firstPlayer)
                firstPlayer = coords(x, y);
        }
    if (!start)
        return true;
    if (!firstPlayer) {
        // no player in the pattern: on the free floor nearest the chunk's middle
        const coords middle = this->lo + chamber::chunkSize / 2;
        std::optional<coords> best;
        for (int y = this->lo.y; y <= this->hi.y; y++)
            for (int x = this->lo.x; x <= this->hi.x; x++) {
                const auto e = this->at(x, y);
                if (e->getType() != bElemTypes::_floorType || !e->getAttrs()->isSteppable())
                    continue;
                if (!best || coords(x, y).distance(middle) < best->distance(middle))
                    best = coords(x, y);
            }
        if (!best)
            throw std::runtime_error("the start chunk's pattern leaves no free floor for the player");
        place(bElemTypes::_player, 0, best->x, best->y);
        firstPlayer = best;
    }
    // the distance part of the difficulty is measured from where the player starts
    this->mychamber->origin = *firstPlayer;
    return true;
}

bool randomLevelGenerator::placeEverything(int holes, int depth, bool start, bool globalTeleporter)
{
    int tolerance = 10;
    std::vector<elementToPlace>
        elementCollection; // here we will store the elements to be placed on the board
    std::vector<elementToPlace> elementsToChooseFrom;

    // build probablility table - this way we can pick random objects with different probablilities
    for (int c = 1; c < (50 / holes); c++) {
        // dangerous elements here, the more holes, the less of them in the gamefield
        elementsToChooseFrom.push_back({bElemTypes::_monster, 0, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_bunker, 0, 1, 0, 3});
        // each camera brings its own guardian drones
        elementsToChooseFrom.push_back({bElemTypes::_securityCamera, 0, 1, 0, 3});
    }
    for (int c = 0; c < holes * 15; c++) {
        elementsToChooseFrom.push_back({bElemTypes::_kikiType, 0, 1, 0, 2});
        elementsToChooseFrom.push_back({bElemTypes::_bunker, 0, 1, 0, 3});

        elementsToChooseFrom.push_back({bElemTypes::_goldenAppleType, 0, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_simpleBombType, 0, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_puppetMasterType, 0, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_patrollingDrone, 0, 1, 0, 3});
    }
    for (int cnt = 0; cnt < holes * 5; cnt++) {
        elementsToChooseFrom.push_back({bElemTypes::_key, 1, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_key, 3, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_bazookaType, 0, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_plainGun, 0, 1, 0, 3});
    }

    for (int c = 0; c < difficulty::landmineCopies(depth); c++)
        elementsToChooseFrom.push_back({bElemTypes::_landmineType, 0, 1, 0, 3});

    const int localTeleporters = this->localTeleporterSubtype();
    for (int c = 0; c < 50; c++) {
        elementsToChooseFrom.push_back({bElemTypes::_brickClusterType, 0, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_key, 0, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_key, 2, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_key, 4, 1, 0, 3});
        elementsToChooseFrom.push_back({bElemTypes::_teleporter, localTeleporters, 1, 0, 3});
    }
    // a spare avatar now and then
    elementsToChooseFrom.push_back({bElemTypes::_player, 0, 1, 0, 3});

    //
    elementsToChooseFrom.push_back({bElemTypes::_patrollingDrone, 0, 1, 0, 3});
    elementsToChooseFrom.push_back({bElemTypes::_puppetMasterType, 0, 1, 0, 3});

    //first find area for the player and stuff for it

    elementCollection.push_back({bElemTypes::_player, 0, 2, 0, 3});
    elementCollection.push_back({bElemTypes::_key, 1, 2, 0, 3});
    elementCollection.push_back({bElemTypes::_plainGun, 0, 2, 0, 3});
    elementCollection.push_back({bElemTypes::_teleporter, 0, 1, 0, 3});

    /***************************refactor me******************************/
    int demandedSurface = 0;
    for (unsigned int cnt = 0; cnt < elementCollection.size(); cnt++)
        demandedSurface += elementCollection[cnt].surface * (elementCollection[cnt].number);
    if (start) {
        // the player's starting area, behind doors that need the key placed with the player
        auto playerArea = this->pickArea(demandedSurface, tolerance, true);
        if (!playerArea) {
            std::cout << "Found areas is empty!\n";
            return false;
        }
        this->placeElementCollection(*playerArea, elementCollection);
        // the distance part of the difficulty is measured from the player's starting room
        {
            const chamberArea &startArea = *playerArea;
            this->mychamber->origin = coords((startArea.upLeft.x + startArea.downRight.x) / 2,
                                             (startArea.upLeft.y + startArea.downRight.y) / 2);
        }
        this->placeDoors({bElemTypes::_door, 1, 1, 0, 9}, *playerArea);
        if (auto parent = this->headNode->parentOf(*playerArea))
            parent->get().childrenLock = true;
        this->retireArea(*playerArea);
    }
    elementCollection.clear();

    if (globalTeleporter) {
        elementCollection.push_back({bElemTypes::_teleporter, 0, 1, 0, 5});
        if (auto teleportArea = this->pickArea(demandedSurface, tolerance, true)) {
            this->placeElementCollection(*teleportArea, elementCollection);
            this->placeDoors({bElemTypes::_door, 0, 1, 0, 9}, *teleportArea);
            if (auto parent = this->headNode->parentOf(*teleportArea))
                parent->get().childrenLock = true;
            this->retireArea(*teleportArea);
        }
    }

    while (true) {
#ifdef _VerbousMode_
        std::cout << "Surface total: " << this->headNode->surface << "\n";
#endif
        int roomSurface = 0;
        int elementsToMake = ((this->eng() % 5) + 1) * 5;
        elementCollection.clear();
        for (int cnt = 0; cnt < elementsToMake; cnt++)
            elementCollection.push_back(goe::rng::pick(this->eng, elementsToChooseFrom));
        for (const auto &element : elementCollection)
            roomSurface += element.surface * element.number;
        auto area = this->pickArea(roomSurface, tolerance);
        if (!area)
            break;
        this->placeElementCollection(*area, elementCollection);
        elementCollection.clear();
        auto parent = this->headNode->parentOf(*area);
        if (!parent)
            break; // only the whole level is left, nothing more to fill
        // a door in front of the next chunk's wall gap would stand in the room, not in a wall
        if (!parent->get().childrenLock && !this->reachesNextChunkGap(*area)) {
            int dice = this->eng() % 100;
            int keyType = this->eng() % 10;
            if (dice < (75 / holes)) {
                if (keyType >= 5)
                    this->placeDoors({bElemTypes::_brickClusterType, 0, 1, 0, 9}, *area);
                else
                    this->placeDoors({bElemTypes::_door, keyType, 1, 0, 9}, *area);
                // the key for the door is placed with the next collection; the door placed at the end might never receive it
                elementCollection.push_back({bElemTypes::_key, keyType, 1, 0, 9});
                parent->get().childrenLock = true;
            }
        }
        this->retireArea(*area);
    }
    /****************************************************/

    return true;
}

void randomLevelGenerator::placeDoorAt(const elementToPlace &element, int x, int y)
{
    // the cell in front of the next chunk's wall gap is cleared even where a maze wall ends there;
    // a door on it would stand beside that wall, a cell away from the gap
    if (!this->steppableAt(x, y) || this->frontsNextChunkGap(x, y))
        return;
    this->createElement(element)->stepOnElement(this->at(x, y));
}

bool randomLevelGenerator::placeDoors(elementToPlace element, const chamberArea &location)
{
#ifdef _VerbousMode_
    std::cout << "door " << element.eSubType << "\n";
#endif
    // every open cell around the area gets a door
    for (int c1 = location.upLeft.x - 1; c1 <= location.downRight.x + 1; c1++) {
        this->placeDoorAt(element, c1, location.upLeft.y - 1);
        this->placeDoorAt(element, c1, location.downRight.y + 1);
    }
    for (int c2 = location.upLeft.y; c2 <= location.downRight.y; c2++) {
        this->placeDoorAt(element, location.upLeft.x - 1, c2);
        this->placeDoorAt(element, location.downRight.x + 1, c2);
    }
    // a gap in the next chunk's east or south wall is not ours to change, and the cell in front of
    // it gets no door (placeDoorAt); areas that must be locked avoid those gaps (pickArea)
    return true;
}

std::shared_ptr<bElem> randomLevelGenerator::createElement(elementToPlace element)
{
    switch (element.eType) {
    case bElemTypes::_goldenAppleType:
        return elementFactory::generateAnElement<goldenApple>(this->mychamber, 0);
    case bElemTypes::_player:
        return elementFactory::generateAnElement<player>(this->mychamber, 0);
    case bElemTypes::_door:
        return elementFactory::generateAnElement<door>(this->mychamber, element.eSubType);
    case bElemTypes::_key:
        return elementFactory::generateAnElement<key>(this->mychamber, element.eSubType);
    case bElemTypes::_monster:
        return elementFactory::generateAnElement<monster>(this->mychamber, element.eSubType);
    case bElemTypes::_plainGun:
        return elementFactory::generateAnElement<plainGun>(this->mychamber, element.eSubType);
    case bElemTypes::_bunker:
        return elementFactory::generateAnElement<bunker>(this->mychamber, 0);
    case bElemTypes::_teleporter:
        return elementFactory::generateAnElement<teleport>(this->mychamber, element.eSubType);

    case bElemTypes::_simpleBombType:
        return elementFactory::generateAnElement<simpleBomb>(this->mychamber, 0);
    case bElemTypes::_patrollingDrone:
        return elementFactory::generateAnElement<patrollingDrone>(this->mychamber, 0);
    case bElemTypes::_brickClusterType:
        return elementFactory::generateAnElement<brickCluster>(this->mychamber, 0);
    case bElemTypes::_securityCamera:
        return elementFactory::generateAnElement<securityCamera>(this->mychamber, 0);
    case bElemTypes::_puppetMasterType:
        return puppetMasterFR::create(this->mychamber, (int) (this->eng() % puppetMasterFR::looseKinds));
    case bElemTypes::_bazookaType:
        return elementFactory::generateAnElement<bazooka>(this->mychamber, element.eSubType);
    case bElemTypes::_landmineType:
        return elementFactory::generateAnElement<landmine>(this->mychamber, 0);
    case bElemTypes::_kikiType:
        return elementFactory::generateAnElement<kiki>(this->mychamber, element.eSubType);
    }
    return nullptr;
}
