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
#include "fnordNavigator.h"

namespace fnordController {
/**
 * @brief fnordNavigator::fnordNavigator - this is a stub kind of a constructor
 */
fnordNavigator::fnordNavigator()
    : fnordNavigator(fnordEcho())
{}

/**
 * @brief fnordEcho::makeFnord - make a fnordEcho of any element. this is a part of the fnordVision thing
 * @param element
 * @param echo
 * @return 
 */
fnordEcho &fnordNavigator::makeFnord(std::shared_ptr<bElem> element, fnordEcho &echo)
{
    if (!element || element->getStats()->isDisposed()) {
        echo.disposed = true;
        return echo;
    }
    if (echo.fId != 0 && echo.fType == -1 && element->getStats()->getInstanceId() != echo.fId)
        return echo;
    echo.fId = element->getStats()->getInstanceId();
    echo.fType = element->getType();
    echo.previousPos = echo.currentPos;
    echo.previousDir = echo.currentDir;
    echo.previousFace = echo.currentFace;
    echo.hasGun = (bool) (element->getAttrs()->canCollect()
                          && element->getAttrs()->getInventory()->getActiveWeapon());
    echo.currentPos = element->getStats()->getMyPosition();
    echo.currentDir = element->getStats()->getMyDirection();
    echo.currentFace = element->getStats()->getFacing();
    echo.fTime = bElem::getCntr();
    echo.isSteppable = element->getAttrs()->isSteppable();
    echo.isCollectible = element->getAttrs()->isCollectible();
    echo.isWeapon = element->getAttrs()->isWeapon();
    echo.fScore = this->rescore(echo);
    return echo; /// We return the echo
}

fnordNavigator::fnordNavigator(const fnordEcho &myFnord)
    : myFnord(myFnord)
{
    this->fFunction = static_cast<fnordFunc>(
        bElem::randomNumberGenerator()
        % 3); // We choose a random function of the monster. Which means different navigators
    this->fMode = fnordMode::Wandering;
}

/**
 * @brief fnordNavigator::addFnord add fnord to fnordNavigatorStructures
 * @param fnord
 */

void fnordNavigator::addFnord(fnordEcho &fnord)
{
    if (!fnord.fnordMapped) {
        this->fnordMap.push(fnord);
        fnord.fnordMapped = true;
    }
    this->fnordStore[fnord.fId] = fnord;
}
/**
 * @brief fnordNavigator::rescore scoring function
 * @param fnord
 * @return int
 *
 * This function assigns a score to fnords based on their type and the monster's current mode.
 * Scores are determined by the priority rules set by the mighty Eris herself.
 */

int fnordNavigator::rescore(const fnordEcho &fnord)
{
    int score = 0;
    if (fnord.disposed || fnord.fType == bElemTypes::_wallType
        || fnord.fType == bElemTypes::_boubaType)
        return GoEConstants::_avoidPriority;
    switch (this->fMode) {
    case fnordMode::Fighting:
        // When in Fighting mode, prioritize the player and weapons, avoid shooting points if unarmed.
        if (myFnord.hasInventory) {
            if (myFnord.hasGun) {
                if (fnord.fType == bElemTypes::_player) {
                    score = GoEConstants::_topPriority; // Player is top priority
                } else if (fnord.fType == bElemTypes::_plainGun
                           || fnord.fType == bElemTypes::_bazookaType) {
                    score = GoEConstants::_midPriority; // Weapons have middle priority
                } else if (fnord.fType == bElemTypes::_key
                           || fnord.fType == bElemTypes::_goldenAppleType) {
                    score = GoEConstants::_midPriority; // Keys and apples have middle priority
                }
            } else {
                if (fnord.currentPos.getX() == myFnord.currentPos.getX()
                    || fnord.currentPos.getY() == myFnord.currentPos.getY()) {
                    score = GoEConstants::_avoidPriority; // Avoid shooting points
                } else if (fnord.fType == bElemTypes::_plainGun
                           || fnord.fType == bElemTypes::_bazookaType) {
                    score = GoEConstants::_topPriority; // Weapons are top priority
                } else if (fnord.fType == bElemTypes::_key
                           || fnord.fType == bElemTypes::_goldenAppleType) {
                    score = GoEConstants::_midPriority; // Keys and apples have middle priority
                }
            }
        } else {
            if (fnord.fType == bElemTypes::_player) {
                score = GoEConstants::_topPriority; // Player is top priority
            }
        }
        break;
    case fnordMode::Wandering:
        // When wandering, prioritize non-shooting points and mid-priority items.
        if (fnord.currentPos.getX() != myFnord.currentPos.getX()
            && fnord.currentPos.getY() != myFnord.currentPos.getY()) {
            if (fnord.fType == bElemTypes::_plainGun || fnord.fType == bElemTypes::_bazookaType) {
                score = GoEConstants::_topPriority; // Weapons are top priority
            } else if (fnord.fType == bElemTypes::_key
                       || fnord.fType == bElemTypes::_goldenAppleType) {
                score = GoEConstants::_midPriority; // Keys and apples have middle priority
            }
        }
        break;
    default:
        if (fnord.currentPos == myFnord.currentPos + dir::dirToCoords(myFnord.currentDir)
            && fnord.isSteppable) {
            score = GoEConstants::_midPriority;
            break;
        }
        if (fnord.isWeapon && myFnord.hasInventory) {
            score = GoEConstants::_topPriority;
            break;
        }
        if (fnord.isCollectible && myFnord.hasInventory) {
            score = GoEConstants::_midPriority;
            break;
        }
        if (fnord.isSteppable) {
            score = GoEConstants::_lowPriority;
            break;
        }
        score = GoEConstants::_lowestPriority;
        break;
    }

    // Points visited longest ago have the lowest priority
    if (fnord.fTime > GoEConstants::_monsterMemoryTime) {
        score = GoEConstants::_lowestPriority;
    }
    auto dist = myFnord.currentPos.distance(fnord.currentPos);
    if (score >= GoEConstants::_avoidPriority)
        score = (score * 5 * 5)
                / ((dist * dist)
                   + 1); /// the scoring heavily depends on the distance of the thisFnord;
    return score;
}

/**
 * @brief fnordNavigator::cleanupMap this is removing obsolete, or avoided elements.   
 * @return void
 * the overall logic depends on the recent mode change. 
 * When there was a mode chane, the logic is we rescore all the points in the fnordStore, then reconstruct the priority queue, so we can find an appropriate candidate for locking on the target
 */
void fnordNavigator::cleanupMap()
{
    // Now clean up fnordMap by rebuilding it
    std::priority_queue<fnordEcho, std::vector<fnordEcho>, std::greater<>> newQueue;
    // Rebuild scoring if the mode was changed
    if (this->modeChange) {
        auto currentTime = bElem::getCntr();
        for (auto it = fnordStore.begin(); it != fnordStore.end();) {
            it->second.fScore = this->rescore(it->second);
            if (currentTime - it->second.fTime > GoEConstants::_monsterMemoryTime
                || it->second.fScore == GoEConstants::_avoidPriority) {
                it = fnordStore.erase(it); // remove the fnordEcho
            } else {
                it++;
            }
        }
        for (const auto &[fId, fnord] : fnordStore) {
            newQueue.push(fnord);
        }
        /// Swap the old and new priority queues
        fnordMap = std::move(newQueue);
        this->modeChange = false;
    } else {
        auto currentTime = bElem::getCntr();
        for (auto it = 0; fnordStore.size();) {
            if (currentTime - fnordStore[it].fTime > GoEConstants::_monsterMemoryTime) {
                it = fnordStore.erase(it); // remove the fnordEcho
            } else {
                ++it;
            }
        }
    }
}

/**
 * @brief lockedOnTheTargetLogic the fnordLogic, when it is locked on the target
 * @return 
 */
myUtility::Coords fnordNavigator::lockedOnTheTargetLogic()
{
    if (hasPath) {
        if (fnordPath.empty())
            return myUtility::NOCOORDS;
        myUtility::Coords mc = this->fnordPath.back();
        this->fnordPath.pop_back();
        return mc;
    }
    /// search for the path
    return myUtility::NOCOORDS;
    /// Create the path seeking part With A* algorithm
}

/**
 * @brief Finds a path from the start point to the end point on the board using the A* search algorithm.
 * 
 * This method calculates a path between two given coordinates on the game board, 
 * considering obstacles and movement costs. It employs the A* search algorithm to 
 * efficiently find the shortest path, taking into account both the distance from 
 * the start and an estimated heuristic to the end. The search is performed within 
 * a limited area around the start point, defined by the 'radius' parameter. 
 * The method returns a 'Path' object, which is a sequence of coordinates and 
 * directions representing the steps to follow from the start to the end.
 *
 * @param start The coordinates of the starting point.
 * @param end The coordinates of the ending point.
 * @return A 'Path' object representing the found path, or an empty path if no 
 *         path could be found.
 */
Path fnordNavigator::findPath(const myUtility::Coords &start, const myUtility::Coords &end)
{
    // 1. Data structure definitions
    struct Node
    {
        myUtility::Coords coords;
        dir::direction direction; // Direction from which we came to this node
        int g;                    // Cost from start
        int h;                    // Heuristic to the end
        Node *parent;             // Pointer to the parent (previous node on the path)

        int f() const { return g + h; } // Total cost (f = g + h)

        // Comparison operator for the priority queue (reversed, because we want the smallest f)
        auto operator<=>(const Node &other) const { return f() <=> other.f(); };
    };

    // Function calculating the heuristic (Manhattan distance)
    auto calculateHeuristic = [&](const myUtility::Coords &a, const myUtility::Coords &b) { return std::abs(a.getX() - b.getX()) + std::abs(a.getY() - b.getY()); };

    // 2. Initialization
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList; // Open list of nodes to consider
    int searchAreaSize = radius * 2;
    std::vector<std::vector<bool>> closedMap(searchAreaSize); // Create the outer vector (rows)

    for (int i = 0; i < searchAreaSize; ++i) {
        closedMap[i].resize(searchAreaSize, false); // Resize each row (columns), initialized to false
    } // Map marking visited nodes (limited to searchAreaSize x searchAreaSize)

    // Calculate the boundaries of the search area, using cutOff()
    myUtility::Coords searchAreaStart = start - myUtility::Coords(radius, radius);
    searchAreaStart.cutOff(fnordBoard->getSizeCrd()); // Limit to the size of the board

    myUtility::Coords searchAreaEnd = start + myUtility::Coords(radius, radius);
    searchAreaEnd.cutOff(fnordBoard->getSizeCrd()); // Limit to the size of the board

    // Shift start and end to the local coordinate system (0,0) - (searchAreaSize-1, searchAreaSize-1)
    myUtility::Coords localStart = start - searchAreaStart;
    myUtility::Coords localEnd = end - searchAreaStart;

    Node startNode{localStart, dir::direction::NODIRECTION, 0, calculateHeuristic(localStart, localEnd), nullptr}; // Starting node
    openList.push(startNode);

    // 3. A* loop
    while (!openList.empty()) {
        Node current = openList.top(); // Get the node with the lowest cost f
        openList.pop();

        if (closedMap[current.coords.getX()][current.coords.getY()])
            continue;                                                   // If the node has already been considered, skip it
        closedMap[current.coords.getX()][current.coords.getY()] = true; // Mark the node as visited

        if (current.coords == localEnd) { // Target found! (in the local coordinate system)
            // Reconstruct the path from end to beginning
            Path path;
            while (current.parent != nullptr) {
                path.push_back({current.coords, current.direction});
                current = *current.parent;
            }
            std::reverse(path.begin(), path.end()); // Reverse the path, because it was from end to beginning
            return path;
        }

        // 4. Generating neighbors - USING dir::directionToCoordsMap
        for (int i = 0; i < 4; ++i) { // Iterate over possible directions (UP, LEFT, DOWN, RIGHT)
            dir::direction neighborDir = dir::allDirections[i];
            myUtility::Coords neighborCoords = (current.coords + dir::directionToCoordsMap[i]).validate(myUtility::Coords(searchAreaSize, searchAreaSize));
            std::shared_ptr<bElem> elem = this->fnordBoard->getElement(neighborCoords);
            if (neighborCoords == myUtility::NOCOORDS || (!(fnordBoard->getElement(searchAreaStart + neighborCoords)->getAttrs()->isSteppable() && !fnordBoard->getElement(searchAreaStart + neighborCoords)->getAttrs()->isCollectible()))
                || fnordBoard->getElement(searchAreaStart + neighborCoords)->getType() == bElemTypes::_boubaType) {
                continue;
            }

            Node neighbor{neighborCoords, neighborDir, current.g + 1, calculateHeuristic(neighborCoords, localEnd), &current};
            if (!closedMap[neighborCoords.getX()][neighborCoords.getY()]) {
                openList.push(neighbor);
            }
        }
    }

    // No path found
    return {};
}
/**
 * @brief getFnordEcho get reference to fnordEcho, needed for the scanner logic.
 * @param id
 * @return 
 */
fnordEcho &fnordNavigator::getFnordEcho(unsigned long id)
{
    return this->fnordStore[id];
}
} // namespace fnordController
