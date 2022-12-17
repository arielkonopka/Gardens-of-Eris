
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

    randomLevelGenerator* rndl=new randomLevelGenerator(150,150);
    presenter::presenter *myPresenter=new presenter::presenter(rndl->mychamber);
    myPresenter->initializeDisplay();
    myPresenter->loadCofiguredData();
    myPresenter->showSplash();
    rndl->generateLevel(10);
    for (int cnt=5;cnt>0;cnt--)
    {
        rndl=new randomLevelGenerator(100+((5-cnt)*50)+(rndl->gen()%10)*5,100+((5-cnt)*50)+(rndl->gen()%10)*5);
        rndl->generateLevel(cnt);
    }


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


    return 0;
}




