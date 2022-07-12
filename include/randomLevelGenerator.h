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

#define _debugRandomGenerator true
#define Wmin 3
#define Hmin 3
#define _iterations 14
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


    bool placeElement(elementToPlace element,rectangle location);
    bElem* createElement(elementToPlace element);


    chamber *mychamber;
    randomLevelGenerator(int w,int h);

    virtual ~randomLevelGenerator();
    bool generateLevel(int holes);

    //      bool addSpaceToCreate(spaceToCreate spc);
protected:

private:
    int doorTypes;
    int lvlGenerate(int x1,int y1,int x2,int y2,int depth,int holes,std::string loc);
    bool placeDoors(elementToPlace element,rectangle location);
    //    bool claimSpace(spaceToCreate theClaim);
    int recalculateLocations();
    bool banLocation(std::string loc);
    int steppableNeighs(int x, int y);
    void addElementToPlace(elementToPlace element);
    std::vector<elementToPlace> elementsToPlace;
    std::vector<rectangle> bannedPlaces;
    bool isLocationAllowed(int x,int y);
    int findSpotsToChoose(rectangle location);
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
