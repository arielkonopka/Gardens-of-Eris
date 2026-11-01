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

/*
 * goe-audit: do the generator's locks still lock with eight-way movement?
 *
 * For each world seed it builds the start of the world as a new game does (the start chunk and
 * the 5 x 5 chunks around it) and floods the maze without playing:
 *
 *  - walls, closed doors, brick clusters, bunkers and cameras stop the flood;
 *  - floor, landmines, teleporters (a step across takes 8 ticks, they send you away after 55)
 *    and what the player walks into (collectibles, monsters, drones, spare avatars) let it
 *    through;
 *  - once 4-connected, once 8-connected with exactly the game's corner rule (motion::cornerOpen).
 *
 * A room is a 4-connected part of the maze between those walls; a locked room has a door or a
 * brick cluster in its walls. Per seed it reports:
 *  - from the start, opening nothing: cells, rooms, golden apples, keys, spare avatars and global
 *    teleporters reached, 4-way and 8-way;
 *  - the same from the start with only the start room's own doors open (its key lies inside);
 *  - every locked room whose inside an 8-way walk leaves without opening its walls (a leak);
 *  - rooms sealed by brick clusters that a push from outside opens: straight (4-way too) or only
 *    diagonally (8-way only; motion::push with both corners passed).
 * Every bypass found is drawn as ASCII.
 *
 * Usage, from GoEoOL: goe-audit [seeds=200] [first seed=1] [output folder=.]
 * Writes goe-audit.csv and goe-audit-bypasses.txt into the output folder.
 */
#include "elements.h"
#include "chamber.h"
#include "gameSerializer.h"
#include "inputManager.h"
#include "motion.h"
#include "randomStreams.h"
#include "worldBuilder.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
constexpr std::array<dir::direction, 4> fourSides = {dir::direction::UP, dir::direction::LEFT, dir::direction::DOWN,
                                                     dir::direction::RIGHT};

std::uint64_t keyOf(coords c)
{
    return ((std::uint64_t) (std::uint32_t) c.x << 32) | (std::uint32_t) c.y;
}
coords cellOf(std::uint64_t k)
{
    return coords((int) (std::int32_t) (k >> 32), (int) (std::int32_t) (k & 0xffffffffu));
}

/// what the flood sees in a cell
enum class cell : char { outside, wall, open, door, brick };

struct board
{
    std::shared_ptr<chamber> world;
    coords lo, hi; ///< the built square of chunks

    std::shared_ptr<bElem> at(coords c) const
    {
        if (c.x < lo.x || c.y < lo.y || c.x > hi.x || c.y > hi.y)
            return nullptr;
        return world->getElement(c);
    }

    cell kind(coords c) const
    {
        const auto e = this->at(c);
        if (!e)
            return cell::outside;
        const int t = e->getType();
        if (t == bElemTypes::_door)
            return e->getAttrs()->isSteppable() ? cell::open : cell::door;
        if (t == bElemTypes::_brickClusterType)
            return cell::brick;
        // what the player walks onto, or into: it collects it, wakes it, or it moves away. A
        // teleporter can be stood on, and walked across: it sends you away only after 55 ticks.
        if (e->getAttrs()->isSteppable() || e->getAttrs()->isCollectible() || t == bElemTypes::_monster
            || t == bElemTypes::_patrollingDrone || t == bElemTypes::_player)
            return cell::open;
        return cell::wall;
    }

    /// whether a move from c in d passes, 4-way or 8-way with the game's corner rule
    bool passes(coords c, dir::direction d, bool eightWay) const
    {
        return !dir::isDiagonal(d) || (eightWay && motion::cornerOpen(*this->world, c, d));
    }
};

using cellSet = std::unordered_set<std::uint64_t>;

/// the cells reached from the starts without opening anything but the cells in extraOpen
cellSet flood(const board &b, const std::vector<coords> &starts, bool eightWay, const cellSet &extraOpen = {})
{
    cellSet seen;
    std::deque<coords> todo;
    for (const coords s : starts)
        if (seen.insert(keyOf(s)).second)
            todo.push_back(s);
    while (!todo.empty()) {
        const coords c = todo.front();
        todo.pop_front();
        for (const auto d : dir::ways) {
            if (!b.passes(c, d, eightWay))
                continue;
            const coords n = c + dir::dirToCoords(d);
            if ((b.kind(n) != cell::open && !extraOpen.contains(keyOf(n))) || !seen.insert(keyOf(n)).second)
                continue;
            todo.push_back(n);
        }
    }
    return seen;
}

/// a room: a 4-connected part of the maze between walls, doors and bricks
struct room
{
    coords any = NOCOORDS;
    std::size_t size = 0;
    std::vector<coords> doors, bricks; ///< in its walls
    bool globalTeleporter = false;     ///< holds a global (subtype 0) teleporter
};

struct rooms
{
    std::unordered_map<std::uint64_t, int> of; ///< cell -> room
    std::vector<room> all;

    explicit rooms(const board &b)
    {
        for (int x = b.lo.x; x <= b.hi.x; x++)
            for (int y = b.lo.y; y <= b.hi.y; y++) {
                const coords c(x, y);
                if (b.kind(c) != cell::open || of.contains(keyOf(c)))
                    continue;
                const int id = (int) all.size();
                room r;
                r.any = c;
                std::set<std::uint64_t> doors, bricks;
                std::deque<coords> todo{c};
                of[keyOf(c)] = id;
                while (!todo.empty()) {
                    const coords p = todo.front();
                    todo.pop_front();
                    r.size++;
                    if (const auto e = b.at(p); e->getType() == bElemTypes::_teleporter && e->getAttrs()->getSubtype() == 0)
                        r.globalTeleporter = true;
                    for (const auto d : fourSides) {
                        const coords n = p + dir::dirToCoords(d);
                        const cell k = b.kind(n);
                        if (k == cell::door)
                            doors.insert(keyOf(n));
                        if (k == cell::brick)
                            bricks.insert(keyOf(n));
                        if (k != cell::open || of.contains(keyOf(n)))
                            continue;
                        of[keyOf(n)] = id;
                        todo.push_back(n);
                    }
                }
                for (auto k : doors)
                    r.doors.push_back(cellOf(k));
                for (auto k : bricks)
                    r.bricks.push_back(cellOf(k));
                all.push_back(std::move(r));
            }
    }

    bool locked(int id) const { return !all[(std::size_t) id].doors.empty() || !all[(std::size_t) id].bricks.empty(); }

    std::set<int> touched(const cellSet &reach) const
    {
        std::set<int> r;
        for (auto k : reach)
            if (auto it = of.find(k); it != of.end())
                r.insert(it->second);
        return r;
    }
};

/// what a flood reaches: golden apples, keys, spare avatars and global teleporters on reached cells
struct finds
{
    std::size_t cells = 0;
    int rooms = 0, lockedRooms = 0, apples = 0, keys = 0, avatars = 0, globalTeleporters = 0;
};

finds count(const board &b, const rooms &rm, const cellSet &reach, coords start)
{
    finds f;
    f.cells = reach.size();
    for (int id : rm.touched(reach)) {
        f.rooms++;
        f.lockedRooms += rm.locked(id);
    }
    for (auto k : reach) {
        const coords c = cellOf(k);
        if (const auto e = b.at(c)) {
            const int t = e->getType();
            f.apples += t == bElemTypes::_goldenAppleType;
            f.keys += t == bElemTypes::_key;
            f.avatars += t == bElemTypes::_player && !(c == start);
            f.globalTeleporters += t == bElemTypes::_teleporter && e->getAttrs()->getSubtype() == 0;
        }
    }
    return f;
}

/// whether a push from outside the room moves the brick in its walls into the room: the pusher
/// stands on an open cell outside, the cell beyond the brick is free floor inside, and a diagonal
/// push passes both corners (the pusher's and the brick's), as motion::push does
bool pushedIn(const board &b, const rooms &rm, int id, coords brick, bool diagonal)
{
    const auto what = b.at(brick);
    if (!what || !what->getAttrs()->canBePushed() || !what->getAttrs()->isMovable())
        return false;
    for (const auto d : dir::ways) {
        if (dir::isDiagonal(d) != diagonal)
            continue;
        const coords from = brick - dir::dirToCoords(d), beyond = brick + dir::dirToCoords(d);
        const auto past = b.at(beyond);
        const auto inside = rm.of.find(keyOf(beyond));
        const auto outside = rm.of.find(keyOf(from));
        if (!past || !past->getAttrs()->isSteppable() || inside == rm.of.end() || inside->second != id
            || outside == rm.of.end() || outside->second == id)
            continue;
        if (diagonal && (!motion::cornerOpen(*b.world, from, d) || !motion::cornerOpen(*b.world, brick, d)))
            continue;
        return true;
    }
    return false;
}

/// a picture around c: # wall, D door, B brick, T teleporter, k key, a golden apple, p spare
/// avatar, . floor, o the room's inside, ! reached only in 8-way, * the cell in question
std::string draw(const board &b, coords c, const cellSet &inside, const cellSet &only8, int r = 8)
{
    std::string out;
    for (int y = c.y - r; y <= c.y + r; y++) {
        out += "    ";
        for (int x = c.x - r; x <= c.x + r; x++) {
            const coords p(x, y);
            const auto e = b.at(p);
            char ch = ' ';
            if (p == c)
                ch = '*';
            else if (only8.contains(keyOf(p)))
                ch = '!';
            else if (e)
                switch (b.kind(p)) {
                case cell::wall: ch = '#'; break;
                case cell::door: ch = 'D'; break;
                case cell::brick: ch = 'B'; break;
                default:
                    ch = e->getType() == bElemTypes::_teleporter     ? 'T'
                         : e->getType() == bElemTypes::_key             ? 'k'
                         : e->getType() == bElemTypes::_goldenAppleType ? 'a'
                         : e->getType() == bElemTypes::_player          ? 'p'
                         : inside.contains(keyOf(p))                    ? 'o'
                                                                        : '.';
                }
            out += ch;
        }
        out += '\n';
    }
    return out;
}

struct totals
{
    long seeds = 0, cells8not4 = 0, rooms8not4 = 0, locked8not4 = 0, apples = 0, keys = 0, avatars = 0, teleporters = 0;
    long mainCells8not4 = 0, mainLocked8not4 = 0, mainTeleporters8not4 = 0;
    long lockedRooms = 0, doorRooms = 0, brickRooms = 0, leaks4 = 0, leaks8 = 0, seedsWithLeak8 = 0;
    long brickOnlyRooms = 0, brickOpenStraight = 0, brickOpenDiagonalOnly = 0, seedsWithDiagonalOnly = 0;
};
} // namespace

int main(int argc, char **argv)
{
    const int seeds = argc > 1 ? std::max(1, std::atoi(argv[1])) : 200;
    const unsigned first = argc > 2 ? (unsigned) std::strtoul(argv[2], nullptr, 10) : 1u;
    const std::filesystem::path outDir = argc > 3 ? argv[3] : ".";
    std::filesystem::create_directories(outDir);
    inputManager::getInstance(true);

    std::ofstream csv(outDir / "goe-audit.csv");
    std::ofstream pics(outDir / "goe-audit-bypasses.txt");
    csv << "seed,chunks,rooms,lockedRooms,doorRooms,brickRooms,"
           "start_cells4,start_cells8,start_rooms4,start_rooms8,start_apples4,start_apples8,start_keys4,start_keys8,"
           "start_avatars4,start_avatars8,start_globalTp4,start_globalTp8,"
           "main_cells4,main_cells8,main_locked4,main_locked8,main_apples4,main_apples8,main_keys4,main_keys8,"
           "main_avatars4,main_avatars8,main_globalTp4,main_globalTp8,"
           "leaks4,leaks8,globalTpRooms,globalTpRoomLeaks8,brickOnlyRooms,brickOpenStraight,brickOpenDiagonalOnly\n";
    totals t;
    int pictures = 0;

    for (int s = 0; s < seeds; s++) {
        const unsigned seed = first + (unsigned) s;
        gameSerializer::clearWorld();
        goe::rng::setWorldSeed(seed);
        const auto world = worldBuilder::startNew();
        const auto plr = player::getActivePlayer();
        if (!world || !plr) {
            std::fprintf(stderr, "seed %u: no world\n", seed);
            continue;
        }
        t.seeds++;
        board b{world, coords(0, 0), coords(0, 0)};
        int chunks = 0;
        for (const auto ch : world->chunkKeys()) {
            const coords o = chamber::chunkOrigin(ch), e = o + (chamber::chunkSize - 1);
            b.lo = chunks == 0 ? o : coords(std::min(b.lo.x, o.x), std::min(b.lo.y, o.y));
            b.hi = chunks == 0 ? e : coords(std::max(b.hi.x, e.x), std::max(b.hi.y, e.y));
            chunks++;
        }
        const coords start = plr->getStats()->getMyPosition();
        const rooms rm(b);

        // from the start, opening nothing
        const cellSet s4 = flood(b, {start}, false), s8 = flood(b, {start}, true);
        const finds fs4 = count(b, rm, s4, start), fs8 = count(b, rm, s8, start);
        // from the start, with the start room's doors open (its key lies inside)
        cellSet startDoors;
        if (const auto it = rm.of.find(keyOf(start)); it != rm.of.end())
            for (const coords d : rm.all[(std::size_t) it->second].doors)
                startDoors.insert(keyOf(d));
        const cellSet m4 = flood(b, {start}, false, startDoors), m8 = flood(b, {start}, true, startDoors);
        const finds fm4 = count(b, rm, m4, start), fm8 = count(b, rm, m8, start);
        cellSet only8;
        for (auto k : m8)
            if (!m4.contains(k))
                only8.insert(k);

        // every locked room: does a walk from its inside get out without opening its walls?
        int lockedRooms = 0, doorRooms = 0, brickRooms = 0, leaks4 = 0, leaks8 = 0, tpRooms = 0, tpLeaks8 = 0;
        int brickOnly = 0, openStraight = 0, openDiagonalOnly = 0;
        for (int id = 0; id < (int) rm.all.size(); id++) {
            const room &r = rm.all[(std::size_t) id];
            if (!rm.locked(id))
                continue;
            lockedRooms++;
            doorRooms += !r.doors.empty();
            brickRooms += !r.bricks.empty();
            tpRooms += r.globalTeleporter;
            const cellSet in4 = flood(b, {r.any}, false), in8 = flood(b, {r.any}, true);
            const bool leak4 = in4.size() > r.size, leak8 = in8.size() > r.size;
            leaks4 += leak4;
            leaks8 += leak8;
            tpLeaks8 += leak8 && r.globalTeleporter;
            if (leak8 && pictures < 80) {
                cellSet inside, escaped;
                for (auto k : in8)
                    (rm.of.contains(k) && rm.of.at(k) == id ? inside : escaped).insert(k);
                const coords at = cellOf(*escaped.begin());
                pics << "seed " << seed << ": locked room at (" << r.any.x << "," << r.any.y << ") leaks to ("
                     << at.x << "," << at.y << ") in 8-way" << (leak4 ? " (and in 4-way)" : "") << "\n"
                     << draw(b, at, inside, escaped) << "\n";
                pictures++;
            }
            // sealed by bricks only: does a push from outside open it?
            if (r.doors.empty() && !r.bricks.empty()) {
                brickOnly++;
                bool straight = false, diagonal = false;
                coords diagonalBrick = NOCOORDS;
                for (const coords br : r.bricks) {
                    straight = straight || pushedIn(b, rm, id, br, false);
                    if (pushedIn(b, rm, id, br, true)) {
                        diagonal = true;
                        diagonalBrick = br;
                    }
                }
                openStraight += straight;
                if (diagonal && !straight) {
                    openDiagonalOnly++;
                    if (pictures < 80) {
                        cellSet inside;
                        for (auto k : flood(b, {r.any}, false))
                            inside.insert(k);
                        pics << "seed " << seed << ": brick-sealed room at (" << r.any.x << "," << r.any.y
                             << ") opens only by a diagonal push of the brick *\n"
                             << draw(b, diagonalBrick, inside, {}) << "\n";
                        pictures++;
                    }
                }
            }
        }

        csv << seed << ',' << chunks << ',' << rm.all.size() << ',' << lockedRooms << ',' << doorRooms << ','
            << brickRooms << ',' << fs4.cells << ',' << fs8.cells << ',' << fs4.rooms << ',' << fs8.rooms << ','
            << fs4.apples << ',' << fs8.apples << ',' << fs4.keys << ',' << fs8.keys << ',' << fs4.avatars << ','
            << fs8.avatars << ',' << fs4.globalTeleporters << ',' << fs8.globalTeleporters << ',' << fm4.cells << ','
            << fm8.cells << ',' << fm4.lockedRooms << ',' << fm8.lockedRooms << ',' << fm4.apples << ','
            << fm8.apples << ',' << fm4.keys << ',' << fm8.keys << ',' << fm4.avatars << ',' << fm8.avatars << ','
            << fm4.globalTeleporters << ',' << fm8.globalTeleporters << ',' << leaks4 << ',' << leaks8 << ','
            << tpRooms << ',' << tpLeaks8 << ',' << brickOnly << ',' << openStraight << ',' << openDiagonalOnly
            << '\n';

        t.cells8not4 += (long) fs8.cells - (long) fs4.cells;
        t.rooms8not4 += fs8.rooms - fs4.rooms;
        t.locked8not4 += fs8.lockedRooms - fs4.lockedRooms;
        t.apples += fs8.apples - fs4.apples;
        t.keys += fs8.keys - fs4.keys;
        t.avatars += fs8.avatars - fs4.avatars;
        t.teleporters += fs8.globalTeleporters - fs4.globalTeleporters;
        t.mainCells8not4 += (long) only8.size();
        t.mainLocked8not4 += fm8.lockedRooms - fm4.lockedRooms;
        t.mainTeleporters8not4 += fm8.globalTeleporters - fm4.globalTeleporters;
        t.lockedRooms += lockedRooms;
        t.doorRooms += doorRooms;
        t.brickRooms += brickRooms;
        t.leaks4 += leaks4;
        t.leaks8 += leaks8;
        t.seedsWithLeak8 += leaks8 > 0;
        t.brickOnlyRooms += brickOnly;
        t.brickOpenStraight += openStraight;
        t.brickOpenDiagonalOnly += openDiagonalOnly;
        t.seedsWithDiagonalOnly += openDiagonalOnly > 0;
        if ((s + 1) % 20 == 0)
            std::fprintf(stderr, "%d seeds done\n", s + 1);
    }
    gameSerializer::clearWorld();

    std::printf("seeds: %ld (from %u), the start chunk and the 5 x 5 chunks around it\n", t.seeds, first);
    std::printf("\nfrom the start, opening nothing (8-way minus 4-way, all seeds):\n");
    std::printf("  cells %ld, rooms %ld, locked rooms %ld, golden apples %ld, keys %ld, spare avatars %ld, global teleporters %ld\n",
                t.cells8not4, t.rooms8not4, t.locked8not4, t.apples, t.keys, t.avatars, t.teleporters);
    std::printf("from the start with the start room's doors open (8-way minus 4-way):\n");
    std::printf("  cells %ld, locked rooms %ld, global teleporters %ld\n", t.mainCells8not4, t.mainLocked8not4,
                t.mainTeleporters8not4);
    std::printf("\nlocked rooms: %ld (%ld with doors, %ld with bricks in their walls)\n", t.lockedRooms, t.doorRooms, t.brickRooms);
    std::printf("  left without opening their walls: 4-way %ld, 8-way %ld (seeds with an 8-way leak: %ld)\n", t.leaks4,
                t.leaks8, t.seedsWithLeak8);
    std::printf("rooms sealed only by bricks: %ld\n", t.brickOnlyRooms);
    std::printf("  opened by a straight push from outside: %ld\n", t.brickOpenStraight);
    std::printf("  opened only by a diagonal push:         %ld (seeds with any: %ld)\n", t.brickOpenDiagonalOnly,
                t.seedsWithDiagonalOnly);
    std::printf("\npictures: %d (goe-audit-bypasses.txt)\n", pictures);
    return 0;
}
