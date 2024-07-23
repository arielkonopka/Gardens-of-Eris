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


class randomLevelGenerator
{
public:
    chamberArea* headNode;
    std::mt19937 gen;
    std::shared_ptr<bElem> createElement(elementToPlace element);


    std::shared_ptr<chamber> mychamber;
    randomLevelGenerator(int w,int h);

    virtual ~randomLevelGenerator();
    bool generateLevel(int holes);

private:
    int doorTypes;
    bool placeElementCollection(chamberArea* chmbrArea,std::vector<elementToPlace>* elements);

    chamberArea* lvlGenerate(int x1,int y1,int x2,int y2,int depth,int holes);
    bool placeDoors(elementToPlace element,chamberArea* location);
    void addElementToPlace(elementToPlace element);
    int checkWalls(int x, int y);
    int width;
    int height;

    std::vector<coords> spotsToChoose;

};

#endif // RANDOMLEVELGENERATOR_H
