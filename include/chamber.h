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

#ifndef CHAMBER_H
#define CHAMBER_H
#include "Coords.h"
#include "commons.h"
#include "randomWordGen.h"
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <array>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <allegro5/allegro5.h>
typedef struct color
{
    int r;
    int g;
    int b;
    int a;
} colour;
class bElem;
//using boost::multi_array;



/**
 * A board of cells. The game plays on one endless board, the world (makeWorld), whose cells are
 * made chunk by chunk as the player comes near (see worldBuilder). A bounded board
 * (makeNewChamber) has a fixed size and all its cells from the start; tests, the benchmark and
 * saves from before the endless world use those.
 *
 * Either way the cells are stored in square chunks of chunkSize x chunkSize, so coordinates may
 * be negative. A cell that is outside a bounded board, or in a chunk that was not made yet,
 * holds nothing (getElement returns nullptr), and nothing can step there.
 *
 * The board belongs to the game thread.
 */
class chamber: public std::enable_shared_from_this<chamber>
{
    friend class gameSerializer;
public:
    chamber(const chamber&) = delete;
    chamber& operator=(const chamber&) = delete;

    /// the side of a chunk, in cells; also the unit of distance in the difficulty (difficulty.h)
    static constexpr int chunkSize = 64;
    /// on the endless world, only elements up to this many chunks from the active player run;
    /// the rest wait until the player comes back (the 5 x 5 chunks around the player run)
    static constexpr int activeChunks = 2;
    /// whether a cell is close enough to the given one to run (always true on a bounded board)
    bool isActiveNear(coords cell, coords player) const
    {
        if (this->bounded || cell == NOCOORDS || player == NOCOORDS)
            return true;
        const coords a = chunkOf(cell), b = chunkOf(player);
        return std::abs(a.x - b.x) <= activeChunks && std::abs(a.y - b.y) <= activeChunks;
    }
    /// which chunk a cell is in, by floor division, so negative cells land in the right chunk
    static constexpr coords chunkOf(coords cell) { return coords(cell.x >> chunkShift, cell.y >> chunkShift); }
    /// the first cell (the one with the lowest x and y) of a chunk
    static constexpr coords chunkOrigin(coords chunk) { return coords(chunk.x * chunkSize, chunk.y * chunkSize); }

    int calculateLine(myUtility::Coords position,dir::direction Odir);
    std::shared_ptr<bElem> getLastInLine(myUtility::Coords pos,dir::direction mydir);

    /// every board in the world; owns them, so a board lives until the world is cleared
    static std::vector<std::shared_ptr<chamber>> allChambers;
    /// guards allChambers, and is held while a level is generated or the game is saved or loaded
    static std::recursive_mutex worldMutex;
    /// a bounded board of csize cells, covered with floor
    static std::shared_ptr<chamber> makeNewChamber(coords csize);
    static std::shared_ptr<chamber> makeNewChamber(myUtility::Coords csize);
    /// the endless board, with no chunks yet
    static std::shared_ptr<chamber> makeWorld();
    /// false for the endless world
    bool isBounded() const { return this->bounded; }
    /// whether the chunk exists (its cells may be read and written)
    bool hasChunk(coords chunk) const;
    /// makes the chunk if it is missing, and covers it with floor; the endless world only
    void addChunk(coords chunk);
    /// the chunks in memory now (made, or read back from disk)
    const std::vector<coords> &chunkKeys() const { return this->keys; }
    /// whether the chunk was written to disk and dropped from memory (see gameSerializer::swapOutChunk)
    bool isSwapped(coords chunk) const { return this->swapped.contains(keyOf(chunk)); }
    /// how many chunks are on disk
    std::size_t swappedCount() const { return this->swapped.size(); }
    /**
     * While alive, the board has no cells outside lo..hi (both included). A chunk is built inside
     * such a fence, so nothing placed while building it (a kiki's beam, say) reaches into the
     * chunks next to it, and a chunk comes out the same whichever chunks were built before it.
     */
    class fence
    {
    public:
        fence(chamber &board, coords lo, coords hi);
        ~fence();
        fence(const fence &) = delete;
        fence &operator=(const fence &) = delete;

    private:
        chamber &board;
        coords savedLo, savedHi;
    };

    bool visitPosition(int x, int y)
    {
        return this->visitPosition(coords(x,y));
    };
    unsigned int applesCount=0;
    /// where the player starts; the distance part of the difficulty is measured from here
    coords origin = NOCOORDS;
    bool visitPosition(coords point);
    int isVisible(int x, int y) ;
    int isVisible(coords point);
    void setVisible(coords point,int v);

    std::shared_ptr<bElem> getElement(int x, int y) const { return this->topAt(coords(x, y)); }
    std::shared_ptr<bElem> getElement(coords point) const { return this->topAt(point); }
    /// the top element at a cell, without copying the pointer; empty where there is no cell.
    /// The reference is only valid until that cell changes.
    const std::shared_ptr<bElem> &topAt(coords point) const
    {
        static const std::shared_ptr<bElem> none;
        const auto idx = this->chunkIndex(point);
        if (idx < 0)
            return none;
        return this->chunks[(std::size_t) idx]->cells[cellIndex(point)];
    }
    std::shared_ptr<bElem> getElement(myUtility::Coords point) const;
    void setElement(int x, int y, std::shared_ptr<bElem> elem);
    void setElement(coords point,std::shared_ptr<bElem> elem);
    /// the size of a bounded board; (0, 0) for the endless world
    coords getSize();
    myUtility::Coords getSizeCrd();

    explicit chamber(int x,int y);
    explicit chamber(coords csize);
    ~chamber();
    int getInstanceId();
    std::string getName();
    colour getChColour();
    coords getSizeOfChamber();

    /**
     * The stack of elements on a cell. Only these two change the stepping-on and standing-on
     * links, so a stack is never half updated. Neither checks game rules (steppable, collectible,
     * live registration, stepOnAction); bElem::stepOnElement and bElem::removeElement do that.
     */
    /// puts elem directly on top of onto, which must be on a board; elem must not be on one
    static void place(const std::shared_ptr<bElem> &elem, const std::shared_ptr<bElem> &onto);
    /// takes elem out of its cell's stack; a cell left empty gets a fresh floor
    static void lift(const std::shared_ptr<bElem> &elem);

    bool registerLiveElem(std::shared_ptr<bElem> in);
    bool deregisterLiveElem(std::shared_ptr<bElem>in);
    std::vector<std::shared_ptr<bElem>> liveElems;
    std::vector<unsigned long int> toDeregister;


private:
    static constexpr int chunkShift = 6;
    static_assert(chunkSize == 1 << chunkShift);
    static constexpr int chunkCells = chunkSize * chunkSize;
    struct chunk
    {
        /// the top element of every cell's stack
        std::array<std::shared_ptr<bElem>, chunkCells> cells;
        /// fog of war per cell; 0 once the player has seen it
        std::array<int, chunkCells> visited;
        chunk() { this->visited.fill(555); }
    };
    /// a cell's place inside its chunk
    static constexpr std::size_t cellIndex(coords cell)
    {
        return (std::size_t) (cell.x & (chunkSize - 1)) * chunkSize + (cell.y & (chunkSize - 1));
    }
    static constexpr std::uint64_t keyOf(coords chunk)
    {
        return ((std::uint64_t) (std::uint32_t) chunk.x << 32) | (std::uint32_t) chunk.y;
    }
    /// where the cell's chunk is in chunks, or -1 when there is no such cell
    long chunkIndex(coords cell) const
    {
        if (cell.x < this->limitLo.x || cell.y < this->limitLo.y || cell.x > this->limitHi.x
            || cell.y > this->limitHi.y)
            return -1;
        const std::uint64_t key = keyOf(chunkOf(cell));
        // almost every lookup is in the same chunk as the one before
        if (key == this->lastKey && this->lastIndex >= 0)
            return this->lastIndex;
        auto it = this->chunkByKey.find(key);
        if (it == this->chunkByKey.end())
            return -1;
        this->lastKey = key;
        this->lastIndex = (long) it->second;
        return this->lastIndex;
    }
    /// turns a new board into the endless world: no size, every cell may exist
    void makeEndless();
    /// forgets the chunk's cells; whatever holds on to its elements keeps them
    void removeChunk(coords chunk);
    /// the folder this board's swapped chunks are written to; made on first use
    const std::filesystem::path &swapFolder();
    /// the chunk, made empty (no elements, all in fog) if it is missing
    chunk &chunkAt(coords chunkKey);
    /// covers every cell of the chunk that belongs to this board with floor
    void createFloor(coords chunkKey);
    bool bounded = true;
    int width;
    int height;
    /// the cells the board has: a bounded board's own, everything on the endless world, or a fence
    coords limitLo = coords(0, 0);
    coords limitHi = coords(0, 0);
    std::vector<std::unique_ptr<chunk>> chunks;
    std::vector<coords> keys;
    std::unordered_map<std::uint64_t, std::size_t> chunkByKey;
    mutable std::uint64_t lastKey = 0;
    mutable long lastIndex = -1;
    /// chunks on disk: key -> file
    std::unordered_map<std::uint64_t, std::filesystem::path> swapped;
    /// removed with the board
    std::filesystem::path swapDir;
    colour chamberColour;
    std::string chamberName;
    void setInstanceId(int id);
    int instanceid;
    /// chambers are also created by tests on other threads
    static std::atomic<int> lastid;

};

#endif // CHAMBER_H
