/*
 * Copyright (c) 2025, Ariel Konopka
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
#ifndef FNORDNAVIGATOR_H
#define FNORDNAVIGATOR_H
#include "Coords.h"
#include "bElem.h"
#include "commons.h"
#include "fnordEcho.h"
#include <algorithm>
#include <queue>
#include <stack>

namespace fnordVision {
class fnordVision;
} // namespace fnordVision
namespace fnordController {

struct Node
{
    myUtility::Coords coords;
    dir::direction direction;
    float gCost;
    float hCost;
    float fCost;
    myUtility::Coords parent;
    int f() const { return gCost + hCost; }
    auto operator<=>(const Node &other) const { return f() <=> other.f(); }
};

/**
  *  @brief The moveInfo struct - the bElem controlling api
  * a bit different from the one for the player, where turns are without penalty
  */
struct moveInfo
{
    dir::direction direction;
    myUtility::Coords nextCoord;
    bool shouldRotate;
    bool shouldWait; // Dodajemy informację o czekaniu
    enum class actionType { MOVE = 1, HOLD = 2, ATTACK = 3, INTERACT = 4, ROTATE = 5 };
    actionType action;
    bool attackWithGun;
};

enum class fnordMode {
    Wandering = 0,  // Patroling -> Wandering through chaos
    Collecting = 1, // Collecting ephemeral things
    Locking = 2,    // ClosingDoors -> Locking gates
    Escaping = 3,   // RunningAway -> Escaping into confusion
    Fighting = 4,   // Fighting -> Fighting the absurd
    Resting = 5     // Resting -> slacking around
};
using Path = std::vector<std::pair<myUtility::Coords, dir::direction>>;
//enum class fnordFunc { Janitor = 0, Guard = 1, Critter = 2 };

/**
     * @brief The fnordNavigator class
     * This class is a main navigation engine for the monster. It will navigate through fnords.
     * Fnords are anomalies, elements that are not floors nor walls.
     * 
     */
class fnordNavigator : public std::enable_shared_from_this<fnordNavigator>
{
public:
    std::shared_ptr<fnordVision::fnordVision> fv;
    int radius = 10;
    bool isValid(myUtility::Coords pos, myUtility::Coords upLeft);
    std::vector<Node> makePath(std::array<std::array<Node, 50>, 50> map, Node dest);
    Path findPath(const myUtility::Coords _start, const myUtility::Coords _end);
    bool modeChange = false;
    bool hasPath = false;
    bool locked = false;
    fnordEcho lockedFnord;
    void attachBoard(const std::shared_ptr<chamber> chmbr);
    fnordEcho myFnord;
    fnordMode fMode = fnordMode::Wandering;
    //  fnordFunc fFunction = fnordFunc::Guard;
    std::priority_queue<fnordEcho, std::vector<fnordEcho>, std::greater<>>
        fnordMap; /// Priority queue of fnords of interest
    std::map<unsigned long int, fnordEcho> fnordStore;
    Path fnordPath;
    void addFnord(fnordEcho fnord);
    fnordEcho &getFnordEcho(unsigned long int id);
    int rescore(fnordEcho fnord);
    void cleanupMap();
    fnordEcho &makeFnord(std::shared_ptr<bElem> element, fnordEcho &echo);
    fnordNavigator(const fnordEcho &myFnord);
    fnordNavigator();
    std::shared_ptr<chamber> fnordBoard;
    moveInfo makeUpMind(std::shared_ptr<fnordController::fnordNavigator> fnC);
    Path makePath(std::array<std::array<Node, GoEConstants::_fnordNavigatorPathSearchSize>, GoEConstants::_fnordNavigatorPathSearchSize> &allMap, myUtility::Coords end, myUtility::Coords upLeft);
};

} // namespace fnordController
#endif
