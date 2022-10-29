#ifndef CHAMBERAREA_H
#define CHAMBERAREA_H
#include "commons.h"
#include "chamber.h"
#include "bElem.h"
#include <vector>
class chamberArea
{

public:
    static std::vector<chamberArea*> foundAreas;
    static std::vector<bElem* > foundElements;
    chamberArea(int xu, int yu, int xd, int yd);
    virtual ~chamberArea();
    bool addChildNode(chamberArea *child);
    coords upLeft;
    coords downRight;
    int surface;
    std::vector<chamberArea* > children;
    chamberArea* parent;
    int calculateInitialSurface();
    int calculateSurface(chamber *mychamber);
    bool findElementsToStepOn(chamber *myChamber);
    void findElementsRec(chamber* mychamber);
    void findChambersCloseToSurface(int s,int tolerance);
    bool checkIfElementIsFree(int x,int y, chamber *mychamber);
    bool childrenLock;
    void removeEmptyNodes();
};

#endif // CHAMBERAREA_H
