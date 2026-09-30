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
#include <memory>

/**
 * The endless world: one board that grows chunk by chunk around the player, so the maze never
 * ends. There are no separate levels; the whole game happens on this board.
 *
 * Chunks are built on the game thread, at most one per tick (a chunk takes a few milliseconds),
 * the nearest missing one first. Every chunk within buildRadius of the player exists, so the
 * player always walks into a built maze; beyond that, an unbuilt chunk is solid rock.
 */
namespace worldBuilder {
    /// chunks up to this many chunks from the player are built: the 5 x 5 around them
    constexpr int buildRadius = 2;
    static_assert(buildRadius >= chamber::activeChunks, "every chunk where elements run is built");

    /// a new game: the world with its start chunk (where the player is) and every chunk around it
    std::shared_ptr<chamber> startNew();
    /// builds the nearest missing chunk within buildRadius of the cell; false when none is missing
    bool growAround(const std::shared_ptr<chamber> &world, coords cell);
}

#endif // WORLDBUILDER_H
