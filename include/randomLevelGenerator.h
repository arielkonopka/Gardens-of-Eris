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

#include <istream>
#ifndef RANDOMLEVELGENERATOR_H
#define RANDOMLEVELGENERATOR_H
#include <iostream>
#include <chrono>
#include <random>
#include "door.h"
#include "commons.h"
#include "elements.h"
#include "chamber.h"
#include "objectTypes.h"
#include <string>
#include <math.h>
#include "teleport.h"
#include "chamberArea.h"
#include <vector>

#define _debugRandomGenerator true
#define Wmin 3
#define Hmin 4
#define _iterations 35
typedef struct elementToPlace
{
    int eType;
    int eSubType;
    int number;
    int placing; // 0 - scatter, 1 - by walls, 2 - cavity, 3 - turret
    int surface;
    auto operator<=>(const elementToPlace& it) const
    {
        return this->surface<=>it.surface;
    };

} elementToPlace;

enum closingType {doorTypeA=1,doorTypeB=2,none=0};



typedef struct _rect
{
    int x0;
    int y0;
    int x1;
    int y1;
    int surface;
    auto operator<=>(const _rect& it) const
    {
        return this->surface<=>it.surface;
    };
    bool banned;
    std::string location;
} rectangle;


/**
 * Builds a maze and fills it with elements. It builds either a bounded level (tests and the
 * benchmark) or one chunk of the endless world (see worldBuilder).
 *
 * A chunk owns the wall along its west and north edges. Each of those walls has a few gaps,
 * picked from the world seed and the wall's place only (wallGaps), so both chunks beside a wall
 * know where it is open without the other one being built. The cell inside the chunk next to
 * every gap, on all four sides, is cleared, so every gap leads into the maze.
 */
class randomLevelGenerator
{
public:
    std::unique_ptr<chamberArea> headNode;
    /// this level's own randomness: layout, and the starting stats of the elements placed in it
    goe::rng::engine eng;
    std::shared_ptr<bElem> createElement(elementToPlace element);


    std::shared_ptr<chamber> mychamber;
    /// a new bounded level of w x h cells; levelSeed defaults to the next seed derived from the world seed
    randomLevelGenerator(int w,int h,goe::rng::seed levelSeed=goe::rng::nextLevelSeed());
    /// one chunk of the endless world, which must not have that chunk yet
    randomLevelGenerator(std::shared_ptr<chamber> world, coords chunk);

    /// fills the bounded level: walls all around, the player's start and everything else
    bool generateLevel(int holes);
    /// fills the chunk: its west and north walls with their gaps, and everything else.
    /// The start chunk also gets the player and the world's origin.
    bool generateChunk(bool start);
    /// where the west (or north) wall of a chunk is open: offsets from the chunk's first cell
    static std::vector<int> wallGaps(coords chunk, bool west);

private:
    /// the cells this generator may change: from lo to hi, both included
    coords lo = coords(0, 0);
    coords hi = coords(0, 0);
    /// the chunk being built; NOCOORDS for a bounded level
    coords chunk = NOCOORDS;
    /// cells just past hi where the next chunk's wall is open (rows on the east, columns on the south)
    std::vector<int> eastGaps, southGaps;
    /// the top element of a cell; nothing outside this generator's area
    std::shared_ptr<bElem> at(int x, int y) const;
    bool steppableAt(int x, int y) const;
    /// the maze and its outer walls; the holes are the gaps in every maze wall
    void buildMaze(int holes);
    /// the chunk's west and north walls with their gaps, and the cells next to every gap cleared
    void buildChunkWalls();
    /// everything placed in the maze: the player's start (if asked), a global teleporter room, and the rest
    bool placeEverything(int holes, int depth, bool start, bool globalTeleporter);
    /// the subtype of this area's local teleporters, which pair only with each other
    int localTeleporterSubtype() const;
    bool placeElementCollection(const chamberArea& chmbrArea,const std::vector<elementToPlace>& elements);

    std::unique_ptr<chamberArea> lvlGenerate(int x1,int y1,int x2,int y2,int depth,int holes);
    bool placeDoors(elementToPlace element,const chamberArea& location);
    /// puts one element of the door kind on the cell, if it is free
    void placeDoorAt(const elementToPlace &element, int x, int y);
    /// a random area with room for demandedSurface cells, if any is left; a lockable one avoids
    /// the next chunk's wall gaps where it can
    std::optional<chamberArea::areaRef> pickArea(int demandedSurface,int tolerance,bool lockable=false);
    /// the cell is the one in front of a gap in the east or south wall, which belongs to the next chunk
    bool frontsNextChunkGap(int x, int y) const;
    /// the area or its walls reach a gap in the east or south wall, so no door of this chunk can close it
    bool reachesNextChunkGap(const chamberArea& area) const;
    /// removes a filled area from the tree, so nothing else is placed there
    void retireArea(const chamberArea& area);

};

#endif // RANDOMLEVELGENERATOR_H
