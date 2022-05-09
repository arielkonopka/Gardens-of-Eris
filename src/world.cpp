#include "world.h"

world* world::theInstance=NULL;

world::world()
{
    this->garbageBin=gCollect::getInstance();

}

world::~world()
{
    this->theInstance=NULL;
    this->chambers=-1;
    this->activeChamber=-1;

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
    if(this->activeChamber>=0 && (int)this->theWorldContainer.size()>this->activeChamber)
    {
        if (this->theWorldContainer[this->activeChamber]->player!=NOCOORDS)
        {
            return this->theWorldContainer[this->activeChamber];
        }
        else
        {
            return this->findActiveChamber();
        }
    }
    return NULL;

}
chamber* world::findActiveChamber()
{
    for(int chmbr=0; chmbr<this->theWorldContainer.size();chmbr++)
    {
        if(this->theWorldContainer[chmbr]->player!=NOCOORDS)
        {
            this->activeChamber=chmbr;
            return this->theWorldContainer[chmbr];
        }
    }
    if (player::findAndActivatePlayer())
        return this->findActiveChamber();

    //None was voted, we should look for a spare player, before we announce that we did not found it
    //TBC
    return NULL;
}


void world::createAChamber()
{
    randomLevelGenerator *rndl=new randomLevelGenerator(150,150,garbageBin);
    rndl->generateLevel(4);

    this->theWorldContainer.push_back(rndl->mychamber);
    this->chambers=this->theWorldContainer.size();

}



