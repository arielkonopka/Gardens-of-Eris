/*
 * Copyright (c) 2026, Ariel Konopka
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
#ifndef WORLDBUILDER_H
#define WORLDBUILDER_H
#include "commons.h"
#include "chamber.h"
#include "chunkPattern.h"
#include <cstddef>
#include <memory>

/**
 * The endless world: one board that grows chunk by chunk around the player, so the maze never
 * ends. There are no separate levels; the whole game happens on this board.
 *
 * Chunks are built on the game thread, at most one per tick (a chunk takes a few milliseconds),
 * the nearest missing one first. Every chunk within buildRadius of the player exists, so the
 * player always walks into a built maze; beyond that, an unbuilt chunk is solid rock.
 *
 * Chunks further than keepRadius from the player go to disk, one per tick, so memory stays
 * bounded however far the player walks; they come back, as they were left, when the player nears
 * them again (or a teleporter leads into one). The chunks of every avatar stay in memory.
 */
namespace worldBuilder {
    /// chunks up to this many chunks from the player are built: the 5 x 5 around them
    constexpr int buildRadius = 2;
    static_assert(buildRadius >= chamber::activeChunks, "every chunk where elements run is built");

    /// a new game: the world with its start chunk (where the player is) and every chunk around it
    std::shared_ptr<chamber> startNew();
    /// chunks further than this many chunks from the player go to disk: 7 x 7 stay in memory
    constexpr int keepRadius = 3;
    static_assert(keepRadius > buildRadius, "a chunk is not dropped right after it is built");

    /// builds, or reads back from disk, the nearest missing chunk within buildRadius of the cell;
    /// false when none is missing
    bool growAround(const std::shared_ptr<chamber> &world, coords cell);
    /// puts the furthest chunk beyond keepRadius of the cell on disk; false when there is none
    bool shrinkAround(const std::shared_ptr<chamber> &world, coords cell);
    /// makes sure the cell's chunk is in memory, reading it back from disk if it went there
    void bringIn(const std::shared_ptr<chamber> &world, coords cell);
    /**
     * Moves the active player into the chunk, as if they had walked there: the chunk and every
     * chunk within buildRadius of it are built (or read back), and the player stands on the free
     * floor nearest the chunk's middle. The start of the world stays where it was, so a far chunk
     * plays with the difficulty of its distance. False when there is no player or no free floor.
     */
    bool movePlayerTo(const std::shared_ptr<chamber> &world, coords chunk);
    /// how many chunks were made new so far (read back from disk does not count); it only grows,
    /// so a caller that remembers the last value sees when the maze grew (see storyScroller)
    std::size_t chunksGenerated();

    /**
     * Chunks built from a fixed pattern instead of a random maze (see goe::chunkPattern), for
     * agents that train on a world they choose. A chunk made new from now on takes its own
     * pattern, else the default one, else a random maze; chunks already built, or on disk, stay
     * as they are. The patterns stay until changed or cleared, across new games.
     */
    /// the chunk's own pattern; nullptr removes it
    void setPattern(coords chunk, std::shared_ptr<const goe::chunkPattern> pattern);
    /// the pattern of every chunk without its own; nullptr: a random maze
    void setDefaultPattern(std::shared_ptr<const goe::chunkPattern> pattern);
    /// every chunk a random maze again
    void clearPatterns();
    /// what the chunk is built from: nullptr for a random maze
    std::shared_ptr<const goe::chunkPattern> patternFor(coords chunk);
}

#endif // WORLDBUILDER_H
