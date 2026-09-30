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

#include "commons.h"
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
 * What is saved: every board registered in chamber::allChambers (normally just the endless world:
 * each chunk built so far with its cells and fog of war, and the board's name, colour and live
 * elements), every element reachable from them (stacked, collected, held weapons...),
 * the global clock, the instance id counter, the random generator state, and the static registries
 * (active and visited players, golden apples, teleporters, view points, elements being disposed).
 *
 * Elements reference each other by instance id; ids are kept across a save and load.
 * Plain floor and wall tiles, which are the vast majority of cells, are written in a compact
 * form (type, subtype, facing, direction) and get fresh instance ids on load.
 *
 * Chunks of the endless world that are far from the player go to disk (swapOutChunk) and come
 * back when the player nears them (swapInChunk); a save carries those chunks too.
 *
 * Every call must run on the game thread, between ticks (swapInChunk may also run inside a tick).
 * They take chamber::worldMutex themselves.
 * A save from before version 4 (separate bounded levels) still loads.
 */
class gameSerializer
{
public:
    static constexpr uint32_t formatVersion = 5;
    static bool saveGame(const std::string &fileName);
    static bool loadGame(const std::string &fileName);
    /// empties the world (chambers, players, apples, teleporters...), for a load or a new game
    static void clearWorld();

    /**
     * Writes a chunk of the endless world to a file in the world's swap folder, then drops it from
     * memory: its cells, and its elements from the live list and the apple and teleporter lists.
     * An element belongs to the chunk when it stands in it, or is carried by one that does.
     * References to elements outside the chunk are kept as ids and found again by swapInChunk.
     * Must run between ticks. False (and the chunk stays) when it is not there, cannot be written,
     * or holds an avatar or an element still being disposed (or the view's owner).
     */
    static bool swapOutChunk(const std::shared_ptr<chamber> &world, coords chunk);
    /// reads a chunk written by swapOutChunk back; false when it is not on disk or cannot be read
    static bool swapInChunk(const std::shared_ptr<chamber> &world, coords chunk);

private:
    class writer;
    class reader;
    struct loadContext;
    /// one element of a stack being read: a compact tile, or the id of a record
    struct cellEntry
    {
        std::shared_ptr<bElem> compact;
        uint64_t id = 0;
    };
    using cellStacks = std::vector<std::pair<coords, std::vector<cellEntry>>>;

    /// a chunk's fog of war and cell stacks
    static void writeChunk(writer &w, const chamber &board, std::size_t index);
    /// reads what writeChunk wrote into the board's chunk; the stacks are put together later
    static void readChunk(reader &r, const std::shared_ptr<chamber> &board, coords key, cellStacks &stacks);
    static std::vector<cellEntry> readStack(reader &r);
    /// fog of war, run-length encoded, for cells in the given order
    static void readFog(reader &r, chamber &board, const std::vector<coords> &order);
    /// every queued element and whatever those reference in turn, then the end marker
    static void writeRecords(writer &w);
    /// puts the read stacks on the board, bottom element first
    static void rebuildStacks(loadContext &ctx, const std::shared_ptr<chamber> &board, cellStacks &stacks);
    /// whether an avatar, an element being disposed or the view's owner is in the chunk
    static bool isPinned(coords chunk);
    /// the music of global teleporters among the elements
    static void restartMusic(const std::vector<std::shared_ptr<bElem>> &elements);

    static void writeElement(writer &w, const std::shared_ptr<bElem> &e);
    static std::shared_ptr<bElem> readElement(reader &r, loadContext &ctx);
    static bool isCompact(const std::shared_ptr<bElem> &e);
    static std::shared_ptr<bElem> createByType(int type, int subtype);
};

#endif // GAMESERIALIZER_H
