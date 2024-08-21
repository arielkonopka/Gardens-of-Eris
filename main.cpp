/*
 * Gardens of Eris.
 *
 * Copyright (c) 2023, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <exception>
#include <thread>
#include "elements.h"
#include "presenter.h"
#include "randomLevelGenerator.h"
#include "soundManager.h"

bool finish=false;
void createChambers()
{
    randomLevelGenerator* rndl;
    for (int cnt=5; cnt>0; cnt--)
    {
        for(int c2=0; c2<5; c2++)
        {
            rndl=new randomLevelGenerator(500,500);
            rndl->generateLevel(cnt);
            if(finish)
                return;
        }
    }
}




int main( int argc, char * argv[] )
{
    auto *myPresenter=new presenter::presenter();
    myPresenter->initializeDisplay();
    myPresenter->loadCofiguredData();
    myPresenter->showSplash();
    auto rndl=new randomLevelGenerator(500,500);
    soundManager::getInstance()->setupSong(0,0, {0.0f,0.0f,0.0f},0,true);
    soundManager::getInstance()->setupSong(2,2, {1.0f,130.0f,0.0f},-1,true);
    soundManager::getInstance()->setupSong(3,3, {0.0f,170.0f,0.0f},-1,true);
    soundManager::getInstance()->setupSong(10,5, {550.0f,0.0f,0.0f},-1,true);
    soundManager::getInstance()->setupSong(11,6, {550.0f,550.0f,0.0f},-1,true);
    soundManager::getInstance()->setupSong(12,7, {1.0f,550.0f,0.0f},-1,true);
    soundManager::getInstance()->setupSong(13,8, {250.0f,250.0f,0.0f},-1,true);
    soundManager::getInstance()->setupSong(4,4, {0.0f,0.0f,0.0f},1,true);
    soundManager::getInstance()->setupSong(5,3, {0.0f,0.0f,0.0f},2,true);
    soundManager::getInstance()->setupSong(6,2, {0.0f,0.0f,0.0f},3,true);
    soundManager::getInstance()->setupSong(8,0, {0.0f,0.0f,0.0f},5,true);
    soundManager::getInstance()->setupSong(9,0, {0.0f,0.0f,0.0f},6,true);
    rndl->generateLevel(5);
    /// generate the remaining leveldata in the background, so the user would not be greeted with a delay.
    std::thread nt=std::thread(&createChambers);
    nt.detach();
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




