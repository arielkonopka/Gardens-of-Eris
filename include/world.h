#ifndef WORLD_H
#define WORLD_H
#include "commons.h"
#include "chamber.h"
#include "randomLevelGenerator.h"
#include "elements.h"

class world
{
    public:
        static world* getInstanceId();
        virtual ~world();
        chamber* getActiveChamber();
        void createAChamber();
    private:
        std::vector<chamber*> theWorldContainer;
        int chambers;
        chamber* activeChamber;
        int secChamb;
        chamber* findActiveChamber();
        std::vector<randomLevelGenerator*> rGens;
        static world* theInstance;
        world(); // we will have a singleton class this time.

};

#endif // WORLD_H
