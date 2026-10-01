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
#include <memory>
#include "elements.h"
#include "presenter.h"
#include "worldBuilder.h"
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
/// what the player reads when a game ends; the lost game's save goes, so it cannot be continued
void showEnd(titleScreen &title, int bestScore, bool &windowOpen)
{
    // the maze never ends, so a game only ends when the last avatar is lost
    const std::string saveFile = gameSettings::getInstance().getSaveFile();
    if (!gameSerializer::removeSave(saveFile))
        std::cout << "The save of the lost game " << saveFile << " could not be deleted\n";
    const std::vector<std::string> lines = {"Best score: " + std::to_string(bestScore)};
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

    {
        auto myPresenter=std::make_unique<presenter::presenter>();
        myPresenter->initializeDisplay();
        myPresenter->loadCofiguredData();
        myPresenter->showSplash();
        // the music follows the difficulty: the higher D, the further down the music list
        soundManager::getInstance().setupDifficultyMusic();

        titleMenu menu(gameSettings::getInstance(), gameSettings::settingsFile, [] {
            return gameSerializer::canLoad(gameSettings::getInstance().getSaveFile());
        });
        titleScreen title(menu);
        const auto firstSeed = goe::rng::worldSeed();
        bool windowOpen = true;
        bool played = false;    // a world was built or loaded before, so a new game starts over
        bool backToGame = false; // saving failed, so the player goes back to the game they were leaving
        for (bool first = true; windowOpen; first = false) {
            if (backToGame) {
                backToGame = false;
            } else if (first && !saveToLoad.empty() && gameSerializer::loadGame(saveToLoad)) {
                std::cout << "Loaded " << saveToLoad << "\n";
                goe::crashLog::setDetail("Loaded save", saveToLoad);
            } else {
                const auto choice = title.run();
                if (choice == titleMenu::action::EXIT)
                    break;
                // the save folder may have changed on the config screen
                goe::crashLog::setFolder(gameSettings::getInstance().getSaveDirectory());
                if (choice == titleMenu::action::CONTINUE) {
                    const std::string saveFile = gameSettings::getInstance().getSaveFile();
                    title.showBusy("Loading the maze...");
                    if (!gameSerializer::loadGame(saveFile)) {
                        windowOpen = title.showMessage("Cannot continue", {"The save could not be read:", saveFile});
                        continue;
                    }
                    std::cout << "Loaded " << saveFile << "\n";
                    goe::crashLog::setDetail("Loaded save", saveFile);
                } else {
                    title.showBusy("Building the maze...");
                    if (played) {
                        // a new game: the old world goes, and "--seed" builds the same world again
                        gameSerializer::clearWorld();
                        goe::rng::setWorldSeed(seedGiven ? firstSeed : goe::rng::freshSeed());
                    }
                    std::cout << "World seed: " << goe::rng::worldSeed() << "\n";
                    goe::crashLog::setDetail("World seed", std::to_string(goe::rng::worldSeed()));
                    // the start of the endless maze; the rest is built around the player while they play
                    worldBuilder::startNew();
                    // "New game" next to Continue: the new game overwrites the old save
                    if (const std::string saveFile = gameSettings::getInstance().getSaveFile();
                        gameSerializer::canLoad(saveFile) && !gameSerializer::replaceSave(saveFile))
                        std::cout << "The old save " << saveFile << " could not be replaced\n";
                }
            }
            played = true;
            soundManager::getInstance().enableSound();
            const auto end = myPresenter->presentEverything();
            if (end == presenter::gameEnd::QUIT)
                break;
            if (end == presenter::gameEnd::LOST)
                showEnd(title, myPresenter->getLastScore(), windowOpen);
            else if (end == presenter::gameEnd::SAVE_FAILED) {
                windowOpen = title.showMessage("The game could not be saved",
                                               {gameSettings::getInstance().getSaveFile(),
                                                "Your game goes on. Check that the save folder can be written."});
                backToGame = true;
            }
            // SAVED: back to the title screen, where Continue picks the game up again
        }
    }
    videoManager::getInstance().shutdown();
    return 0;
}
