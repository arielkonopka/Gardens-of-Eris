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

#include <atomic>
#include <exception>
#include <memory>
#include <stop_token>
#include <thread>
#include "elements.h"
#include "presenter.h"
#include "randomLevelGenerator.h"
#include "soundManager.h"
#include "gameSerializer.h"
#include "gameSettings.h"
#include "titleScreen.h"
#include "crashLog.h"
#include "videoManager.h"
#include <cstring>
#include <cstdlib>
#include <string>
#include "randomStreams.h"

namespace {
/// builds the remaining levels in the background, until they are all built or the game ends
void createChambers(std::stop_token stop)
{
    // players in these levels wait to be activated; they never take over the game
    player::backgroundScope background;
    for (int cnt=5; cnt>0; cnt--)
    {
        for(int c2=0; c2<5; c2++)
        {
            // let a pending save or load go first
            while(chamber::worldLockWanted && !stop.stop_requested())
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(stop.stop_requested())
                return;
            // the world (chamber::allChambers) keeps the chamber after the generator is gone
            randomLevelGenerator(500,500).generateLevel(cnt);
        }
    }
}

/// what the player reads when a game ends
void showEnd(titleScreen &title, presenter::gameEnd end, int score, bool &windowOpen)
{
    const std::vector<std::string> lines = {"Score: " + std::to_string(score)};
    if (end == presenter::gameEnd::ALL_APPLES)
        windowOpen = title.showMessage("All the golden apples are yours", lines);
    else
        windowOpen = title.showMessage("Game over", lines);
}
} // namespace

int main( int argc, char * argv[] )
{
    // "--load <file>" starts from a saved game instead of a freshly generated world
    // "--seed <number>" builds the same world again; the seed is printed at every start
    std::string saveToLoad;
    bool seedGiven = false;
    for (int c = 1; c + 1 < argc; c++) {
        if (std::strcmp(argv[c], "--load") == 0)
            saveToLoad = argv[c + 1];
        else if (std::strcmp(argv[c], "--seed") == 0) {
            goe::rng::setWorldSeed((goe::rng::seed) std::strtoul(argv[c + 1], nullptr, 10));
            seedGiven = true;
        }
    }
    gameSettings::getInstance().load();
    // a crash leaves a report in the save folder, for players without a debugger
    goe::crashLog::install(gameSettings::getInstance().getSaveDirectory());

    // builds the remaining levels; declared first so it is joined last, after the window is gone
    std::jthread levelBuilder;
    {
        auto myPresenter=std::make_unique<presenter::presenter>();
        myPresenter->initializeDisplay();
        myPresenter->loadCofiguredData();
        myPresenter->showSplash();
        soundManager::getInstance().setupSong(0,0, {0.0f,0.0f,0.0f},0,true);
        soundManager::getInstance().setupSong(2,2, {1.0f,130.0f,0.0f},-1,true);
        soundManager::getInstance().setupSong(3,3, {0.0f,170.0f,0.0f},-1,true);
        soundManager::getInstance().setupSong(10,5, {550.0f,0.0f,0.0f},-1,true);
        soundManager::getInstance().setupSong(11,6, {550.0f,550.0f,0.0f},-1,true);
        soundManager::getInstance().setupSong(12,7, {1.0f,550.0f,0.0f},-1,true);
        soundManager::getInstance().setupSong(13,8, {250.0f,250.0f,0.0f},-1,true);
        soundManager::getInstance().setupSong(4,4, {0.0f,0.0f,0.0f},1,true);
        soundManager::getInstance().setupSong(5,3, {0.0f,0.0f,0.0f},2,true);
        soundManager::getInstance().setupSong(6,2, {0.0f,0.0f,0.0f},3,true);
        soundManager::getInstance().setupSong(8,0, {0.0f,0.0f,0.0f},5,true);
        soundManager::getInstance().setupSong(9,0, {0.0f,0.0f,0.0f},6,true);

        titleMenu menu(gameSettings::getInstance());
        titleScreen title(menu);
        const auto firstSeed = goe::rng::worldSeed();
        bool windowOpen = true;
        for (int game = 0; windowOpen; game++) {
            if (game == 0 && !saveToLoad.empty() && gameSerializer::loadGame(saveToLoad)) {
                std::cout << "Loaded " << saveToLoad << "\n";
                goe::crashLog::setDetail("Loaded save", saveToLoad);
            } else {
                if (title.run() == titleMenu::action::EXIT)
                    break;
                // the save folder may have changed on the config screen
                goe::crashLog::setFolder(gameSettings::getInstance().getSaveDirectory());
                title.showBusy("Building the maze...");
                if (game > 0) {
                    // a new game: the old world goes, and "--seed" builds the same world again
                    levelBuilder = {};
                    gameSerializer::clearWorld();
                    goe::rng::setWorldSeed(seedGiven ? firstSeed : goe::rng::freshSeed());
                }
                std::cout << "World seed: " << goe::rng::worldSeed() << "\n";
                goe::crashLog::setDetail("World seed", std::to_string(goe::rng::worldSeed()));
                randomLevelGenerator(500,500).generateLevel(5);
                /// generate the remaining leveldata in the background, so the user would not be greeted with a delay.
                levelBuilder=std::jthread(&createChambers);
            }
            soundManager::getInstance().enableSound();
            const auto end = myPresenter->presentEverything();
            // stop building levels now; the builder finishes the one it is on while the end screen shows
            levelBuilder.request_stop();
            if (end == presenter::gameEnd::QUIT)
                break;
            showEnd(title, end, myPresenter->getLastScore(), windowOpen);
        }
        levelBuilder.request_stop();
    }
    // close the window before waiting for the level builder, so leaving never looks like a freeze
    videoManager::getInstance().shutdown();
    return 0;
}
