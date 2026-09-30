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
#ifndef RANDOMSTREAMS_H
#define RANDOMSTREAMS_H
#include <cstddef>
#include <iterator>
#include <random>

/**
 * The game's sources of randomness, kept apart so that drawing from one never shifts another.
 *
 * | stream     | used for                                   | thread                     | saved |
 * |------------|--------------------------------------------|----------------------------|-------|
 * | gameplay() | anything that shapes the world: element    | the game thread, or the    | the game's own engine is |
 * |            | behaviour, starting stats, level layout    | thread building a level    | saved; levels are rebuilt from the world seed |
 * | audio()    | music choices                              | any, drawn under a lock    | no    |
 * | cosmetic() | visual effects                             | game thread only           | no    |
 *
 * While a level is built, gameplay() on that thread draws from the level's own engine (see
 * generationScope), so building levels in the background never touches the running game's
 * randomness, and the same level seed always builds the same level. Every seed is derived from
 * the world seed (a chunk's from its place, see placeSeed), so one world seed rebuilds the whole world.
 */
namespace goe::rng {
using engine = std::mt19937;
using seed = engine::result_type;

/// a seed from the system's entropy source
seed freshSeed();

/// the level being built on this thread (innermost generationScope), otherwise saved()
engine &gameplay() noexcept;
/// the running game's own engine, the one saved with the game; game thread only
engine &saved() noexcept;
/// visual effects; game thread only; never saved, and never affects the game
engine &cosmetic() noexcept;
/// one number for audio decisions; safe from any thread
seed audio();

/// the world seed, picked by freshSeed() on first use unless set before
seed worldSeed();
/// sets the world seed and restarts level numbering, so the next level is level 0 again
void setWorldSeed(seed s);
/// the seed of the next level to be generated: derived from the world seed and the level's number
seed nextLevelSeed();

/// a seed for one place of the endless world (a chunk, or one of its walls): derived from the
/// world seed, the place and the salt only, so it does not depend on the order places are built in
seed placeSeed(int x, int y, seed salt = 0);

/// a number from 0 to n - 1; n must be above 0
inline std::size_t below(engine &e, std::size_t n)
{
    return e() % n;
}

/// a random item of a non-empty container
template<class Items>
decltype(auto) pick(engine &e, Items &items)
{
    return items[below(e, std::size(items))];
}

/// While alive, gameplay() on this thread draws from the given engine. Create it as a local
/// variable, so scopes always end in the reverse order they started.
class generationScope
{
public:
    explicit generationScope(engine &levelEngine);
    ~generationScope();
    generationScope(const generationScope &) = delete;
    generationScope &operator=(const generationScope &) = delete;
};
} // namespace goe::rng

#endif // RANDOMSTREAMS_H
