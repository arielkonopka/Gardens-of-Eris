#include "world.h"

world* world::theInstance=NULL;

world::world()
{
    this->garbageBin=gCollect::getInstance();

}

world::~world()
{
    world::theInstance=NULL;
    this->chambers=-1;
    this->activeChamber=NULL;

}
world* world::getInstanceId()
{
    if (world::theInstance==NULL)
    {
        world::theInstance=new world();
    }
    return world::theInstance;
}

chamber* world::getActiveChamber()
{
    if (this->activeChamber->player!=NOCOORDS)
    {
        return this->activeChamber;
    }
    else
    {
        return this->findActiveChamber();
    }
    return NULL;

}


chamber* world::findActiveChamber()
{
    for(auto chmbr:this->theWorldContainer)
    {
        if(chmbr->player!=NOCOORDS)
        {
            this->activeChamber=chmbr;
            return chmbr;
        }
    }

    //None was voted, we should look for a spare player, before we announce that we did not found it
    //TBC
    return NULL;
}


void world::createAChamber()
{
    randomLevelGenerator *rndl=new randomLevelGenerator(150,150);
    rndl->generateLevel(4);

    this->theWorldContainer.push_back(rndl->mychamber);
    this->chambers=this->theWorldContainer.size();

}



