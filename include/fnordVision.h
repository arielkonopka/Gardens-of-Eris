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

#ifndef FNORDVISION_H
#define FNORDVISION_H

#include "Coords.h" // Includes the header file for the Coords class
#include "fnordEcho.h"
//#include "fnordNavigator.h"
#include <map>    // Includes the map container
#include <vector> // Includes the vector container

// Forward declaration of fnordNavigator (important to avoid circular dependencies)
namespace fnordController {
class fnordNavigator;
}
namespace fnordVision {

/**
 * @class fnordVision
 * @brief A class for generating circle points and lines using Bresenham's algorithms.
 *        This class provides functionality to find points on a circle of a given radius
 *        and generate lines from the center of the circle to these points.  The points
 *        and lines are stored in static maps for efficient access.
 */
class fnordVision
{
public:
    /**
     * @brief Default constructor.
     */
    fnordVision() = default;

    /**
     * @brief findCirclePoints - Finds all points on a circle and generates lines.
     * @param r - The radius of the circle.
     *          This function calculates the points on the circle's circumference
     *          using Bresenham's circle algorithm and stores them in the 'circles' map.
     *          It then generates lines from the center of the circle to each of these
     *          points using Bresenham's line algorithm and stores them in the 'linePoints' map.
     */
    static void findCirclePoints(int r);

    /**
     * @brief registerBoard - register the "real wolrd" board
     * @param board
     */
    void registerBoard(std::shared_ptr<chamber> board);

    bool isInVew(myUtility::Coords start, myUtility::Coords stop);

    /**
     * @brief circles - A static map storing the points on circles.
     *        The keys of the map are the radii of the circles, and the values are vectors
     *        containing the Coords of the points on the circle.
     */
    static std::map<int, std::vector<myUtility::Coords>> circles;

    /**
     * @brief linePoints - A static map storing the points on lines.
     *        The keys of the map are related to the circles (e.g., radius), and the values
     *        are vectors of vectors of Coords, representing lines.
     */
    static std::map<int, std::vector<std::vector<myUtility::Coords>>> linePoints;

    /**
     * @brief translate - Translates a point by a given offset.
     * @param pos - The point to translate.
     * @param center - The offset to translate by.
     * @return The translated point.
     */
    myUtility::Coords translate(myUtility::Coords pos, myUtility::Coords center);

    /**
     * @brief chaosScan - scan myFnordNeighborhood and determine visibility, addd visible fnords.
     * @param fn - fnordNavigator
     */

    void chaosScan(std::shared_ptr<fnordController::fnordNavigator> fn);

    static void BresenhamLine(int r, const myUtility::Coords &cPoint);

private:
    std::shared_ptr<chamber> fnordBoard;
};

} // namespace fnordVision

#endif // FNORDVISION_H
