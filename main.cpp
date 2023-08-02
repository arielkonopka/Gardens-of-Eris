
#include <exception>
#include <string>
#include <iostream>
#include <vector>
#include <string>

#include "commons.h"
#include "elements.h"
#include "presenter.h"
#include "randomLevelGenerator.h"
#include "soundManager.h"
int main( int argc, char * argv[] )
{
    bool finish=false;

    randomLevelGenerator* rndl=new randomLevelGenerator(200,200);
    presenter::presenter *myPresenter=new presenter::presenter(rndl->mychamber);
    myPresenter->initializeDisplay();
    myPresenter->loadCofiguredData();
    myPresenter->showSplash();
    soundManager::getInstance()->setupSong(0,0,{0,0,0},0,true);
    soundManager::getInstance()->setupSong(1,1,{rndl->mychamber->width*32,0,0},0,true);
    soundManager::getInstance()->setupSong(2,2,{rndl->mychamber->width*32,0,rndl->mychamber->height*32},0,true);
    soundManager::getInstance()->setupSong(3,3,{0,0,rndl->mychamber->height*32},0,true);
    soundManager::getInstance()->setupSong(4,4,{0,0,0},1,false);
    soundManager::getInstance()->setupSong(5,3,{0,0,0},2,false);
    soundManager::getInstance()->setupSong(6,2,{0,0,0},3,false);
    soundManager::getInstance()->setupSong(7,1,{0,0,0},4,false);
    soundManager::getInstance()->setupSong(8,0,{0,0,0},5,false);



    rndl->generateLevel(10);
  /*  for (int cnt=5;cnt>0;cnt--)
    {
        rndl=new randomLevelGenerator(200+((5-cnt)*50)+(rndl->gen()%10)*5,200+((5-cnt)*50)+(rndl->gen()%10)*5);
        rndl->generateLevel(cnt);
    }
*/
    soundManager::getInstance()->enableSound();


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




