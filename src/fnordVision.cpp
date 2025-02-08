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
#include "fnordVision.h" // Includes the header file for fnordVision class

namespace fnordVision {

// Initialize the static maps outside the class definition
std::map<int, std::vector<myUtility::Coords>> fnordVision::circles;
std::map<int, std::vector<std::vector<myUtility::Coords>>> fnordVision::linePoints;

/**
 * @brief findCirclePoints - Finds all points on a circle using Bresenham's algorithm
 *        and generates lines from the center to each point.
 * @param r - The radius of the circle.
 */
void fnordVision::findCirclePoints(int r)
{
    if (circles.find(r) != circles.end())
        return;
    int x = 0, y = r;  // Initialize starting point on the circle
    int d = 3 - 2 * r; // Initialize decision parameter
    while (y >= x) {
        // Add points for all 8 octants using symmetry
        circles[r].emplace_back(x, y);
        circles[r].emplace_back(y, x);
        circles[r].emplace_back(-x, y);
        circles[r].emplace_back(-y, x);
        circles[r].emplace_back(x, -y);
        circles[r].emplace_back(y, -x);
        circles[r].emplace_back(-x, -y);
        circles[r].emplace_back(-y, -x);

        // Update decision parameter and move to the next point
        if (d < 0)
            d = d + 4 * x + 6;
        else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }

    // Generate lines from the center to each point on the circle
    for (const auto &point : circles[r]) {
        BresenhamLine(r, point);
    }
}
/**
 * @brief registerBoard - Register the "real world" chamber
 * @param board
 */

void fnordVision::registerBoard(std::shared_ptr<chamber> board)
{
    this->fnordBoard = board;
}

bool fnordVision::isInVew(myUtility::Coords start, myUtility::Coords stop)
{
    myUtility::Coords &center = start;
    myUtility::Coords &cPoint = stop;
    ; // Start point of the line
    int x0 = center.getX();
    int y0 = center.getY();
    int x1 = cPoint.getX(); // End point of the line
    int y1 = cPoint.getY();
    int dx = std::abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1; // Direction of x-axis movement
    int dy = -std::abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1; // Direction of y-axis movement
    int error = dx + dy;       // Initial error value

    // Bresenham's algorithm loop
    while (x0 != x1 || y0 != y1) {
        int e2 = 2 * error;
        if (e2 >= dy) {
            error = error + dy;
            x0 = x0 + sx;
        }
        if (e2 <= dx) {
            error = error + dx;
            y0 = y0 + sy;
        }
        if (this->fnordBoard->getElement(x0, y0)->getAttrs()->isSteppable())
            return false; // Add the next point on the line
    }
    return true;
}

/**
 * @brief translate - Translates a point by a given offset.
 * @param pos - The point to translate.
 * @param center - The offset to translate by.
 * @return The translated point.
 */
myUtility::Coords fnordVision::translate(myUtility::Coords pos, myUtility::Coords center)
{
    return pos + center; // Uses the overloaded + operator from the Coords class
}

/**
 * @brief chaosScan - scan rectangle, and add fnords that are in view
 * @param fn
 */

void fnordVision::chaosScan(std::shared_ptr<fnordController::fnordNavigator> fn)
{
    if (!this->fnordBoard)
        return;
    myUtility::Coords pos = fn->myFnord.currentPos;
    myUtility::Coords posUp = pos - myUtility::Coords(40, 40);
    myUtility::Coords posDown = pos + myUtility::Coords(40, 40);
    posUp.cutOff(this->fnordBoard->getSizeCrd());
    posDown.cutOff(this->fnordBoard->getSizeCrd());

    for (auto x = posUp.getX(); x < posDown.getX(); x++) {
        for (auto y = posUp.getY(); y < posDown.getY(); y++) {
            auto type = this->fnordBoard->getElement(x, y)->getType();
            if (type != bElemTypes::_floorType) {
                std::shared_ptr<bElem> el = this->fnordBoard->getElement(x, y);
                if (this->isInVew(el->getStats()->getMyPosition(), fn->myFnord.currentPos)) {
                    fnordEcho &fe = fn->getFnordEcho(el->getType());
                    fn->addFnord(fn->makeFnord(el, fe));
                }
            }
        }
    }
}

/**
 * @brief BresenhamLine - Draws a line using Bresenham's line algorithm.
 * @param r - The radius of the circle (used as a key for linePoints map).
 * @param cPoint - The end point of the line. The start point is assumed to be (0, 0).
 */

void fnordVision::BresenhamLine(int r, const myUtility::Coords &cPoint)
{
    myUtility::Coords center(0, 0); // Start point of the line
    int x0 = center.getX();
    int y0 = center.getY();
    int x1 = cPoint.getX(); // End point of the line
    int y1 = cPoint.getY();
    int dx = std::abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1; // Direction of x-axis movement
    int dy = -std::abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;              // Direction of y-axis movement
    int error = dx + dy;                    // Initial error value
    std::vector<myUtility::Coords> lCoords; // Vector to store line points

    lCoords.emplace_back(x0, y0); // Add the starting point

    // Bresenham's algorithm loop
    while (x0 != x1 || y0 != y1) {
        int e2 = 2 * error;
        if (e2 >= dy) {
            error = error + dy;
            x0 = x0 + sx;
        }
        if (e2 <= dx) {
            error = error + dx;
            y0 = y0 + sy;
        }
        lCoords.emplace_back(x0, y0); // Add the next point on the line
    }
    linePoints[r].push_back(lCoords); // Store the line points in the map
}

} // namespace fnordVision
