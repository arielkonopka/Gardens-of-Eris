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

enum closingType {doorTypeA=1,doorTypeB=2,teleport=10,none=0};


typedef struct _spaceToCreate
{
    /*
    We will combine the objects to be thrown to the board into spaces, each space might be locked with:
     door - the key is one to all doors of the type
     oneTimeDoor - for each instance a separate key instance is needed (these are consumed)
     teleport - all walls are blind, in the area there is a teleport and one is located outside
    */
    int surface;
    closingType closing;
    std::vector<elementToPlace> elementsToBePlaced;

} spaceToCreate;


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


        bool placeElement(elementToPlace element,std::string location);
        bElem* createElement(elementToPlace element);

        std::vector<elementToPlace> elementsToPlace;
        chamber *mychamber;
        randomLevelGenerator(int w,int h,gCollect *gbin);
        void addElementToPlace(elementToPlace element);
        virtual ~randomLevelGenerator();
        bool generateLevel(int holes);
        int lvlGenerate(int x1,int y1,int x2,int y2,int depth,int holes,std::string loc);
        bool placeDoors(elementToPlace element,std::string location);
        bool claimSpace(spaceToCreate theClaim);
        int recalculateLocations();
        bool banLocation(std::string loc);
  //      bool addSpaceToCreate(spaceToCreate spc);
    protected:

    private:
    //    std::vector<spaceToCreate> spacesToCreate;
        bool isLocationAllowed(int x,int y);
        int findSpotsToChoose(std::string location);
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
