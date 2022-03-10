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

    struct timespec delay;

    delay.tv_sec=0;
    delay.tv_nsec=2;
    gCollect *garbageBin=new gCollect();
    randomLevelGenerator *rndl=new randomLevelGenerator(50,50,garbageBin);
    presenter::presenter *myPresenter=new presenter::presenter(rndl->mychamber);
    chamber *myFirstChamber=rndl->mychamber;

    myPresenter->initializeDisplay();
    myPresenter->loadCofiguredData();
    rndl->addElementToPlace({_collectible,5,25,0,"B"});
    rndl->addElementToPlace({_collectible,0,20,0,"Bc"});
    rndl->addElementToPlace({_key,0,20,0,"Ba"});
    rndl->addElementToPlace({_key,1,1,0,"Bb"});
    rndl->addElementToPlace({_door,0,20,0,"Ba"});
    rndl->addElementToPlace({_door,1,20,0,"Bc"});

    rndl->addElementToPlace({_player,0,1,0,"Ba"});
    bElem *mojElement2=new monster(myFirstChamber,garbageBin);



    mojElement2->stepOnElement(myFirstChamber->chamberArray[2][2]);
    mojElement2=new door(myFirstChamber,garbageBin,0);
    mojElement2->stepOnElement(myFirstChamber->chamberArray[2][3]);
    mojElement2=new key(myFirstChamber,garbageBin,0);
    mojElement2->stepOnElement(myFirstChamber->chamberArray[2][4]);
    //rndl->lvlGenerate(1,1,48,48,8,4,"B");
    rndl->generateLevel(4);


    //new chamber(100,100,garbageBin);
    //






    myPresenter->showGameField(1,1);

    int blue=0;

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
    for (int y=0; y<myFirstChamber->height; y++)
    {
        for (int x=0; x<myFirstChamber->width; x++)
        {
            std::cout<<myFirstChamber->chamberArray[x][y]->getType()<<","<<myFirstChamber->chamberArray[x][y]->getSubtype()<<"|";

        }
        std::cout<<">\n";
    }

    bElem* n=new key(myFirstChamber,garbageBin,10);
    n->setSubtype(10);
    std::cout<<"\n\n"<<n->getSubtype();

    delete myFirstChamber;
    return 1;
}
