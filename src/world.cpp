#include "world.h"

world* world::theInstance=nullptr;

world::world()
{


}

world::~world()
{
    world::theInstance=nullptr;
    this->chambers=-1;
    this->activeChamber=nullptr;
    for(auto rls:this->rGens)
    {
        delete rls;
    };
    for(auto chmbr:this->theWorldContainer)
    {
        delete chmbr;
    };
}
world* world::getInstanceId()
{
    if (world::theInstance==nullptr)
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
    return this->findActiveChamber();
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
    return nullptr;
}


void world::createAChamber()
{
    randomLevelGenerator *rndl=new randomLevelGenerator(150,150);
    rndl->generateLevel(4);
    this->rGens.push_back(rndl);
    this->theWorldContainer.push_back(rndl->mychamber);
    this->chambers=this->theWorldContainer.size();

}



