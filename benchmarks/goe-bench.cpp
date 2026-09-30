/*
 * Headless benchmark: builds the endless world the way the game does and times the parts that matter.
 * Run it from GoEoOL/ (it reads data/skins.json):  ../build/goe-bench [chunks] [ticks]
 * 61 chunks of 64 x 64 cells are about as many cells as one of the old 500 x 500 levels.
 */
#include "elements.h"
#include "chamber.h"
#include "gameSerializer.h"
#include "inputManager.h"
#include "worldBuilder.h"
#include <chrono>
#include <cstdio>
#include <algorithm>
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
    const int chunks = argc > 1 ? std::max(1, std::atoi(argv[1])) : 61;
    const int ticks = argc > 2 ? std::atoi(argv[2]) : 500;
    inputManager::getInstance(true);
    long rss0 = rssKb();

    auto t = clk::now();
    auto world = worldBuilder::startNew();
    double startMs = msSince(t);
    const std::size_t startChunks = world->chunkKeys().size();
    // walk east, building what the game would build around the player
    t = clk::now();
    for (coords p = world->origin; world->chunkKeys().size() < (std::size_t) chunks; p.x += chamber::chunkSize)
        while (world->chunkKeys().size() < (std::size_t) chunks && worldBuilder::growAround(world, p))
            ;
    const std::size_t grown = world->chunkKeys().size() - startChunks;
    double growMs = msSince(t);
    long rss1 = rssKb();

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

    const std::size_t total = world->chunkKeys().size();
    std::printf("world of %zu chunks\n", total);
    std::printf("start         %9.1f ms (%zu chunks)\n", startMs, startChunks);
    std::printf("one chunk     %9.1f ms (average of %zu)\n", grown ? growMs / grown : 0.0, grown);
    std::printf("memory        %9.1f MB (%.1f MB per chunk)\n", (rss1 - rss0) / 1024.0,
                (rss1 - rss0) / 1024.0 / total);
    std::printf("tick          %9.3f ms (average of %d)\n", tickMs, ticks);
    std::printf("save          %9.1f ms%s\n", saveMs, saved ? "" : " FAILED");
    std::printf("load          %9.1f ms%s\n", loadMs, loaded ? "" : " FAILED");
    return saved && loaded ? 0 : 1;
}
