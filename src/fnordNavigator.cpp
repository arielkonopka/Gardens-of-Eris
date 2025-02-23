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
#include "fnordVision.h"

namespace fnordController {
/**
 * @brief fnordNavigator::fnordNavigator - this is a stub kind of a constructor
 */
fnordNavigator::fnordNavigator()
    : fnordNavigator(fnordEcho())
{
    this->fv = std::make_shared<fnordVision::fnordVision>();
}

bool fnordNavigator::isValid(myUtility::Coords pos, myUtility::Coords upLeft)
{
    if (upLeft.validate(fnordBoard->getSizeCrd()) == myUtility::NOCOORDS)
        return false;
    auto bEle = this->fnordBoard->getElement(pos + upLeft);
    if (!bEle || !bEle->getAttrs()->isSteppable() || bEle->getType() == bElemTypes::_boubaType)
        return false;
    return true;
}

std::vector<Node> fnordNavigator::makePath(std::array<std::array<Node, 50>, 50> map, Node dest)
{
    try {
        int x = dest.coords.getX();
        int y = dest.coords.getY();
        std::stack<Node> path;
        std::vector<Node> usablePath;

        while (!(map[x][y].parent == myUtility::Coords(x, y)) && !(map[x][y].coords == myUtility::NOCOORDS)) {
            path.push(map[x][y]);
            x = map[x][y].parent.getX();
            y = map[x][y].parent.getY();
        }
        path.push(map[x][y]);

        while (!path.empty()) {
            Node top = path.top();
            path.pop();
            usablePath.emplace_back(top);
        }
        return usablePath;
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}

void fnordNavigator::attachBoard(const std::shared_ptr<chamber> chmbr)
{
    this->fnordBoard = chmbr;
    this->fv->registerBoard(chmbr);
}

/*
moveInfo fnordNavigator::makeUpMind() {
    moveInfo mi;
    mi.action = moveInfo::actionType::HOLD;
    mi.direction = myFnord.previousDir;
    mi.nextCoord = myFnord.currentPos;
    
    if (!this->fnordBoard) {
        return mi; // No board, no movement, just chillin'
    }
    
    if (!this->locked) { // Not locked on a target, time to explore the chaos
        this->fMode = (this->myFnord.hasGun) ? fnordMode::Fighting : fnordMode::Collecting;
        this->modeChange = true;
        this->cleanupMap(); // Gotta clean up the mess before making more
        this->fv->chaosScan(shared_from_this()); // Scan for new opportunities in the chaos
        
        if (this->fnordMap.empty()) { // Nothing interesting in sight, time to wander
            mi.action = moveInfo::actionType::MOVE;
            mi.nextCoord = mi.nextCoord + dir::dirToCoords(mi.direction);
            return mi;
        }
        
        // Pick the most intriguing point from the chaos (1.2)
        fnordEcho bestFnord;
        int bestScore = std::numeric_limits<int>::min(); // Lowest score to start with
        
        for (const auto& [id, fnord] : fnordStore) { // Check every fnord in the store
            if (fnord.fScore > bestScore) { // Found a more interesting fnord
                bestScore = fnord.fScore; // Update the best score
                bestFnord = fnord; // Remember the best fnord
            }
        }
        
        this->lockedFnord = bestFnord; // Lock onto the chosen fnord
        this->fnordPath = this->findPath(this->myFnord.currentPos, bestFnord.currentPos); // Calculate the path to it
        
        if (this->fnordPath.empty()) { // No path to the chosen fnord, maybe it's unreachable
            return mi; // Give up and wait for new opportunities
        }
        
        this->locked = true; // We're locked on a target now
    }
    
    // 2. We have a path, let's follow it
    
    // 3. Are we there yet?
    auto targetDistance = this->myFnord.currentPos.distance(this->lockedFnord.currentPos);
    if (targetDistance <= 1) { // Close enough to the target
        // Take action based on the target type
        if (this->lockedFnord.isWeapon) {
            // ... (action: pick up weapon)
        } else if (this->lockedFnord.fType == bElemTypes::_player) {
            // ... (action: attack)
        } else if (this->lockedFnord.isCollectible) {
            // ... (action: collect item)
        }
        this->locked = false; // Unlock after taking action
        return mi; // No movement needed, action taken
    }
    
    // 4. Move to the next point on the path
    auto fnordStep = this->fnordPath.back();
    if (this->myFnord.currentDir != fnordStep.second) { // Wrong direction, gotta rotate
        mi.action = moveInfo::actionType::ROTATE;
        mi.shouldRotate = true;
        mi.direction = fnordStep.second;
        mi.shouldWait = true;
        mi.nextCoord = myFnord.currentPos + dir::dirToCoords(fnordStep.second);
        return mi;
    }
    
    mi.action = moveInfo::actionType::MOVE; // Move to the next point
    mi.direction = fnordStep.second;
    mi.nextCoord = fnordStep.first;
    
    if (myFnord.currentPos == fnordStep.first) { // Reached the current point on the path
        this->fnordPath.pop_back(); // Remove it from the path
        if (this->fnordPath.empty()) { // Path finished, we're at the target
            this->locked = false; // Unlock and look for new targets
        }
    }
    return mi;
}
*/

moveInfo fnordNavigator::makeUpMind(std::shared_ptr<fnordController::fnordNavigator> fnC)
{
    moveInfo mi;
    mi.action = moveInfo::actionType::HOLD;
    mi.direction = myFnord.previousDir;
    mi.nextCoord = myFnord.currentPos;
    if (locked && lockedFnord.currentPos == myFnord.currentPos)
        locked = false;
    if (!this->fnordBoard)
        return mi;
    if (!this->locked) {
        this->fMode = (this->myFnord.hasGun) ? fnordMode::Fighting : fnordMode::Collecting;
        this->modeChange = true;
        this->cleanupMap();
        this->fv->chaosScan(fnC);
        if (this->fnordMap.empty()) {
            mi.action = moveInfo::actionType::MOVE;
            mi.nextCoord = mi.nextCoord + dir::dirToCoords(mi.direction);
            return mi;
        }
        this->fnordPath.clear();
        while (this->fnordPath.empty() && !this->fnordMap.empty()) {
            auto fnord = this->fnordMap.top();
            this->fnordMap.pop(); // this is not really good way to handle it, as we will have to perform cleanUp to recover removed Elements, this includes the already collected one...
            this->lockedFnord = fnord;
            this->fnordPath = this->findPath(this->myFnord.currentPos, fnord.currentPos);
        }
        if (!this->fnordPath.empty()) {
            this->locked = true;
        }
    }
    if (!locked)
        return mi;
    auto fnordStep = this->fnordPath.back();
    auto targetDistance = this->myFnord.currentPos.distance(this->lockedFnord.currentPos);

    if (targetDistance <= 1) { // Close enough to the target
        if (this->lockedFnord.fType == bElemTypes::_player) {
            mi.action = moveInfo::actionType::ATTACK;
            mi.direction = fnordStep.second;
            this->locked = false; // Unlock after taking action
            return mi;            // No movement needed, action taken
        }
    }
    if (targetDistance == 0) {
        /// it seems we reached the point in means either we collected it or it had already run away, either way, we should remove it, set the state to unlocked, and return whatever mi object we already have, it will rescan the perimetr next time.
        this->fnordStore.erase(this->lockedFnord.fId);
        locked = false;
        return mi;
    }

    if (this->myFnord.currentDir != fnordStep.second) {
        mi.action = moveInfo::actionType::ROTATE;
        mi.shouldRotate = true;
        mi.direction = fnordStep.second;
        mi.shouldWait = true;
        return mi;
    }
    mi.action = moveInfo::actionType::MOVE;
    mi.nextCoord = fnordStep.first;
    mi.direction = fnordStep.second;
    this->fnordPath.pop_back();
    return mi;
}

/*** FINISH me ***
Path fnordNavigator::makePath(std::array<std::array<Node, GoEConstants::_fnordNavigatorPathSearchSize>, GoEConstants::_fnordNavigatorPathSearchSize> &allMap, myUtility::Coords end, myUtility::Coords upLeft)
{
    Path r;
    myUtility::Coords spoint = end;
    r.push_back(allMap[spoint.getX()][spoint.getY()]);
}

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
    /*  this->fFunction = static_cast<fnordFunc>(
        bElem::randomNumberGenerator()
        % 3); // We choose a random function of the monster. Which means different navigators
  */
    this->fMode = fnordMode::Wandering;
}

/**
 * @brief fnordNavigator::addFnord add fnord to fnordNavigatorStructures
 * @param fnord
 */

void fnordNavigator::addFnord(fnordEcho fnord)
{
    if (fnord.fScore == -6502)
        return;
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

int fnordNavigator::rescore(fnordEcho fnord)
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
        if (fnord.currentPos == myFnord.currentPos + myUtility::Coords::dir2coords(myFnord.currentDir) && fnord.isSteppable) {
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
    double dist = std::max(1.0, static_cast<double>(myFnord.currentPos.distance(fnord.currentPos)));
    int tdist = std::max(1, static_cast<int>(myFnord.fTime - fnord.fTime));

    if (score >= GoEConstants::_avoidPriority)
        score = (score * 5 * 5) / ((dist * tdist)); /// the scoring heavily depends on the time and the distance of the thisFnord;
    return std::floor(score * dist);
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
Path fnordNavigator::findPath(const myUtility::Coords _start, const myUtility::Coords _end)
{
    std::vector<Node> empty;

    if (_start.validate(fnordBoard->getSizeCrd()) == myUtility::NOCOORDS || _end.validate(fnordBoard->getSizeCrd()) == myUtility::NOCOORDS || _start.distance(_end) > GoEConstants::_fnordNavigatorPathSearchSize)
        return {};

    myUtility::Coords upLeft = (myUtility::Coords::min(_start, _end) - myUtility::Coords(5, 5)).cutOff(fnordBoard->getSizeCrd());
    auto start = _start - upLeft;
    auto end = _end - upLeft;
    if (isValid(end, upLeft) == false || isValid(start, upLeft) || start == end) {
        return {};
    }

    bool closedList[GoEConstants::_fnordNavigatorPathSearchSize + 10][GoEConstants::_fnordNavigatorPathSearchSize + 10];

    //Initialize whole map
    std::array<std::array<Node, GoEConstants::_fnordNavigatorPathSearchSize>, GoEConstants::_fnordNavigatorPathSearchSize> allMap;
    for (int x = 0; x < allMap.size(); x++) {
        for (int y = 0; y < allMap[x].size(); y++) {
            allMap[x][y].fCost = FLT_MAX;
            allMap[x][y].gCost = FLT_MAX;
            allMap[x][y].hCost = FLT_MAX;
            allMap[x][y].parent = myUtility::NOCOORDS;
            allMap[x][y].coords = myUtility::Coords(x, y);
            closedList[x][y] = false;
        }
    }

    //Initialize our starting list
    int x = start.getX();
    int y = start.getY();
    allMap[x][y].fCost = 0.0;
    allMap[x][y].gCost = 0.0;
    allMap[x][y].hCost = 0.0;
    allMap[x][y].parent = myUtility::Coords(x, y);
    std::vector<Node> openList;
    openList.emplace_back(allMap[x][y]);
    bool destinationFound = false;

    while (!openList.empty() && openList.size() < 50 * 50) {
        Node node;
        do {
            //This do-while loop could be replaced with extracting the first
            //element from a set, but you'd have to make the openList a set.
            //To be completely honest, I don't remember the reason why I do
            //it with a vector, but for now it's still an option, although
            //not as good as a set performance wise.
            float temp = FLT_MAX;
            std::vector<Node>::iterator itNode;
            for (std::vector<Node>::iterator it = openList.begin(); it != openList.end(); it = next(it)) {
                Node n = *it;
                if (n.fCost < temp) {
                    temp = n.fCost;
                    itNode = it;
                }
            }
            node = *itNode;
            openList.erase(itNode);
        } while (isValid(node.coords, upLeft) == false);

        x = node.coords.getX();
        y = node.coords.getY();
        closedList[x][y] = true;

        //For each neighbour starting from North-West to South-East
        for (auto direct : myUtility::directionToCoordsMap) {
            double gNew, hNew, fNew;
            myUtility::Coords newC = node.coords + direct;
            int newx = newC.getX(), newy = newC.getY();
            if (isValid(newC, upLeft)) {
                if (newC == end) {
                    //Destination found - make path
                    allMap[newx][newy].parent = node.coords;
                    destinationFound = true;
                    return makePath(allMap, end, upLeft);
                } else if (closedList[newx][newy] == false) {
                    gNew = node.gCost + 1.0;
                    hNew = end.manhattan(newC);
                    fNew = gNew + hNew;
                    // Check if this path is better than the one already present
                    if (allMap[newx][newy].fCost == FLT_MAX || allMap[newx][newy].fCost > fNew) {
                        // Update the details of this neighbour node
                        allMap[newx][newy].fCost = fNew;
                        allMap[newx][newy].gCost = gNew;
                        allMap[newx][newy].hCost = hNew;
                        allMap[newx][newy].parent = node.coords;
                        openList.emplace_back(allMap[newx][newy]);
                    }
                }
            }
        }
    }
    if (destinationFound == false) {
        return {};
    }

    return {}; // Brak ścieżki
}

/*Path fnordNavigator::findPath(const myUtility::Coords &start, const myUtility::Coords &end)
{
    /**
     * @brief The Node struct 
     * coords - position in search area
     * direction - direction to get you there
     * g - cost from start
     * h - heuristic to the end
     * parent - parent pointer to reconstruct the full path from end to the beging
     * f() - cost function
     * it also contains <=> operator and cost functions are used
     *
    struct Node
    {
        myUtility::Coords coords;
        dir::direction direction;
        int g;
        int h;
        Node *parent;
        int f() const { return g + h; } /// Total cost (f = g + h)
        auto operator<=>(const Node &other) const { return f() <=> other.f(); };
    };

    /// calculateHeuristic Lambda Function calculating the heuristic (Manhattan distance)
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

*/
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
