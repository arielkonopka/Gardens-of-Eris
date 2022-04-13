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
    gCollect *garbageBin=new gCollect();
    randomLevelGenerator *rndl=new randomLevelGenerator(150,150,garbageBin);

    chamber* myFirstChamber=rndl->mychamber;
    presenter::presenter *myPresenter=new presenter::presenter(myFirstChamber);
    //chamber *myFirstChamber=rndl->mychamber;

    myPresenter->initializeDisplay();
    myPresenter->loadCofiguredData();
   /* rndl->addElementToPlace({_monster,5,5,0,"Bba"});
    rndl->addElementToPlace({_collectible,0,20,0,"Bc"});
    rndl->addElementToPlace({_key,1,20,0,"Baa"});
    rndl->addElementToPlace({_key,0,1,0,"Bbb"});
    rndl->addElementToPlace({_door,0,20,0,"Bba"});
    rndl->addElementToPlace({_door,1,20,0,"Bac"});

    */
     rndl->generateLevel(1);
    rndl->placeElement({_player,0,1,0,5},"Ba");
    rndl->placeElement({_bunker,0,100,0,5},"B");
    rndl->placeElement({_plainGun,0,400,0,5},"B");
    rndl->placeElement({_monster,0,40,0,5},"B");
    bElem *mojElement2=new monster(myFirstChamber,garbageBin);



    mojElement2->stepOnElement(myFirstChamber->chamberArray[2][2]);
    mojElement2=new door(myFirstChamber,garbageBin,0);
    mojElement2->stepOnElement(myFirstChamber->chamberArray[2][3]);
    mojElement2=new key(myFirstChamber,garbageBin,0);
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
        case 2: // player died, time to pick another one.
            finish=true;
            break;
        }
    }


    /*
        mojElement->stepOnElement(myFirstChamber->chamberArray[5][5]);
    //   bool r=mojElement2->stepOnElement(myFirstChamber->chamberArray[3][3]);
    //   if (r==false)
        //  {
        mojElement2->stepOnElement((myFirstChamber->chamberArray[4][5]));
        mojElement2->moveInDirection(RIGHT);
        mojElement2->moveInDirection(LEFT);
        //  mojElement2->stepOnElement((myFirstChamber->chamberArray[4][6]));
        //  }

    */
 /*   for (int y=0; y<myFirstChamber->height; y++)
    {
        for (int x=0; x<myFirstChamber->width; x++)
        {
            std::cout<<myFirstChamber->chamberArray[x][y]->getType()<<","<<myFirstChamber->chamberArray[x][y]->getSubtype()<<"|";

        }
        std::cout<<">\n";
    }
*/
    bElem* n=new key(myFirstChamber,garbageBin,10);
    n->setSubtype(10);
    std::cout<<"\n\n"<<n->getSubtype();

    delete myFirstChamber;
    return 1;
}
