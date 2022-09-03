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
#define Hmin 3
#define _iterations 26
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
    bElem* createElement(elementToPlace element);


    chamber *mychamber;
    randomLevelGenerator(int w,int h);

    virtual ~randomLevelGenerator();
    bool generateLevel(int holes);

    //      bool addSpaceToCreate(spaceToCreate spc);
protected:

private:
    int doorTypes;
    bool placeElementCollection(chamberArea* chmbrArea,std::vector<elementToPlace>* elements);

    chamberArea* lvlGenerate(int x1,int y1,int x2,int y2,int depth,int holes);
    bool placeDoors(elementToPlace element,chamberArea* location);
    void addElementToPlace(elementToPlace element);
    int checkWalls(int x, int y);
    gCollect *garbageCollector;
    int width;
    int height;

    std::vector<coords> spotsToChoose;

};

#endif // RANDOMLEVELGENERATOR_H
