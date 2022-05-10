#include <exception>
#include <string>
#include <iostream>
#include <vector>
#include <string>

#include "commons.h"
#include "elements.h"
#include "presenter.h"
#include "randomLevelGenerator.h"

int main( int argc, char * argv[] )
{
    bool finish=false;
    bElem::resetInstances();
  //  struct timespec delay;

   // delay.tv_sec=0;
   // delay.tv_nsec=2;
    gCollect *garbageBin=gCollect::getInstance();
    randomLevelGenerator *rndl=new randomLevelGenerator(200,200);

    chamber* myFirstChamber=rndl->mychamber;
    presenter::presenter *myPresenter=new presenter::presenter(myFirstChamber);
    //chamber *myFirstChamber=rndl->mychamber;

    myPresenter->initializeDisplay();
    myPresenter->loadCofiguredData();




    rndl->placeElement({_monster,5,5,0},"Bba");
    rndl->placeElement({_collectible,0,500,0},"B");
    rndl->placeElement({_key,1,200,0},"B");
    rndl->placeElement({_key,0,100,0},"B");
    //rndl->placeElement({_door,0,200,0},"Ba");
    //rndl->placeElement({_door,1,20,0},"Bb");


     rndl->generateLevel(1);
    rndl->placeElement({_player,0,100,0,5},"Ba");
    rndl->placeElement({_bunker,0,100,0,5},"B");
    rndl->placeElement({_plainGun,0,400,0,5},"B");
    rndl->placeElement({_monster,0,40,0,5},"B");
    rndl->placeElement({_teleporter,0,40,0,5},"B");
    bElem *mojElement2=new monster(myFirstChamber);



    mojElement2->stepOnElement(myFirstChamber->chamberArray[2][2]);
    mojElement2=new door(myFirstChamber,0);
    mojElement2->stepOnElement(myFirstChamber->chamberArray[2][3]);
    mojElement2=new key(myFirstChamber,0);
    mojElement2->stepOnElement(myFirstChamber->chamberArray[2][4]);
    //rndl->lvlGenerate(1,1,48,48,8,4,"B");

   // std::cout<<bElem::instances<<" instances created\n";
  //  exit(0);

    myPresenter->showGameField(1,1);

  //  int blue=0;

    while(!finish)
    {
        int reason=0;
        reason=myPresenter->presentEverything();
        switch(reason)
        {
        case 0:
            break; // No reaction, needed for reloading or something;
        case 1:

            finish=true;
            break;
        case 2:
            finish=true;
            break;

        }
    }

    bElem* n=new key(myFirstChamber,10);
    n->setSubtype(10);
    std::cout<<"\n\n"<<n->getSubtype();

    delete myFirstChamber;
    return 0;
}
