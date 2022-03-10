#ifndef RANDOMLEVELGENERATOR_H
#define RANDOMLEVELGENERATOR_H
#include <iostream>
#include <chrono>
#include <random>
#include "commons.h"
#include "elements.h"
#include "chamber.h"
#include "objectTypes.h"
#include <string>
#include <math.h>

#define Wmin 4
#define Hmin 4
typedef struct elementToPlace
{
    int eType;
    int eSubType;
    int number;
    int placing; // 0 - scatter, 1 - by walls, 2 - cavity, 3 - turret
    std::string location;
} elementToPlace;

typedef struct _rect
{
    int x0;
    int y0;
    int x1;
    int y1;
    std::string location;
} rectangle;


class randomLevelGenerator
{
    public:


        bool placeElement(elementToPlace element);
        bElem* createElement(elementToPlace element);

        std::vector<elementToPlace> elementsToPlace;
        chamber *mychamber;
        randomLevelGenerator(int w,int h,gCollect *gbin);
        void addElementToPlace(elementToPlace element);
        virtual ~randomLevelGenerator();
        bool generateLevel(int holes);
        int lvlGenerate(int x1,int y1,int x2,int y2,int depth,int holes,std::string loc);
    protected:

    private:
        int findSpotsToChoose(elementToPlace element);
        std::vector<rectangle> endChambers;
        bool qualifies(std::string itemLoc,std::string chamLoc);
        int checkWalls(int x, int y);
        gCollect *garbageCollector;
        int width;
        int height;
        std::mt19937 gen;
        std::vector<coords> spotsToChoose;

};

#endif // RANDOMLEVELGENERATOR_H
