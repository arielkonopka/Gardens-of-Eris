/*
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

#ifndef GAMESERIALIZER_H
#define GAMESERIALIZER_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

class bElem;
class chamber;

/**
 * @brief Saves the whole game world to a binary file and restores it.
 *
 * What is saved: every chamber registered in chamber::allChambers (its cells, fog of war, name,
 * colour, live elements), every element reachable from them (stacked, collected, held weapons...),
 * the global clock, the instance id counter, the random generator state, and the static registries
 * (active and visited players, golden apples, teleporters, view points, elements being disposed).
 *
 * Elements reference each other by instance id; ids are kept across a save and load.
 * Plain floor and wall tiles, which are the vast majority of cells, are written in a compact
 * form (type, subtype, facing, direction) and get fresh instance ids on load.
 *
 * Both calls must run on the game thread, between ticks.
 * They take chamber::worldMutex themselves, so they wait for a level that is still being
 * generated in the background. The in-game keys use try_lock instead, so the game never stalls.
 */
class gameSerializer
{
public:
    static constexpr uint32_t formatVersion = 3;
    static bool saveGame(const std::string &fileName);
    static bool loadGame(const std::string &fileName);
    /// empties the world (chambers, players, apples, teleporters...), for a load or a new game
    static void clearWorld();

private:
    class writer;
    class reader;
    struct loadContext;

    static void writeElement(writer &w, const std::shared_ptr<bElem> &e);
    static std::shared_ptr<bElem> readElement(reader &r, loadContext &ctx);
    static bool isCompact(const std::shared_ptr<bElem> &e);
    static std::shared_ptr<bElem> createByType(int type, int subtype);
};

#endif // GAMESERIALIZER_H
