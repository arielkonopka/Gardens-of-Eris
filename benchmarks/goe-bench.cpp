/*
 * Headless benchmark: builds levels the way the game does and times the parts that matter.
 * Run it from GoEoOL/ (it reads data/skins.json):  ../build/goe-bench [size] [ticks]
 */
#include "elements.h"
#include "chamber.h"
#include "gameSerializer.h"
#include "inputManager.h"
#include "randomLevelGenerator.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
using clk = std::chrono::steady_clock;

double msSince(clk::time_point t)
{
    return std::chrono::duration<double, std::milli>(clk::now() - t).count();
}

long rssKb()
{
    std::ifstream f("/proc/self/status");
    std::string line;
    while (std::getline(f, line))
        if (line.rfind("VmRSS:", 0) == 0)
            return std::atol(line.c_str() + 6);
    return -1; // not on Linux
}
} // namespace

int main(int argc, char **argv)
{
    const int size = argc > 1 ? std::atoi(argv[1]) : 500;
    const int ticks = argc > 2 ? std::atoi(argv[2]) : 500;
    inputManager::getInstance(true);
    long rss0 = rssKb();

    auto t = clk::now();
    {
        randomLevelGenerator gen(size, size);
        gen.generateLevel(5);
    }
    double genMs = msSince(t);
    long rss1 = rssKb();

    auto board = player::getActivePlayer()->getBoard();
    t = clk::now();
    for (int c = 0; c < ticks; c++)
        bElem::runLiveElements();
    double tickMs = msSince(t) / ticks;

    const std::string saveFile = (std::filesystem::temp_directory_path() / "goe-bench.goe").string();
    t = clk::now();
    bool saved = gameSerializer::saveGame(saveFile);
    double saveMs = msSince(t);
    t = clk::now();
    bool loaded = gameSerializer::loadGame(saveFile);
    double loadMs = msSince(t);
    std::filesystem::remove(saveFile);

    std::printf("level %dx%d\n", size, size);
    std::printf("generate      %9.1f ms\n", genMs);
    std::printf("memory        %9.1f MB (%.0f bytes per cell)\n", (rss1 - rss0) / 1024.0,
                (rss1 - rss0) * 1024.0 / (size * size));
    std::printf("tick          %9.3f ms (average of %d)\n", tickMs, ticks);
    std::printf("save          %9.1f ms%s\n", saveMs, saved ? "" : " FAILED");
    std::printf("load          %9.1f ms%s\n", loadMs, loaded ? "" : " FAILED");
    return saved && loaded ? 0 : 1;
}
