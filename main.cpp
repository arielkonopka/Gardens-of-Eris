#ifndef _UNIT_TEST_BUILD_
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
    randomLevelGenerator *rndl;
    rndl=new randomLevelGenerator(555,555);
    chamber* myFirstChamber=rndl->mychamber;
    presenter::presenter *myPresenter=new presenter::presenter(myFirstChamber);

    myPresenter->initializeDisplay();
    myPresenter->loadCofiguredData();
    myPresenter->showSplash();

    for (int cnt=5;cnt>0;cnt--)
    {
        rndl=new randomLevelGenerator(200+(rndl->gen()%10)*5,200+(rndl->gen()%10)*5);
        rndl->generateLevel(cnt);
    }



  //  myPresenter->showGameField(1,1);

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
    //std::cout<<"\n\n"<<n->getSubtype();

    delete myFirstChamber;
    return 0;
}

#endif


