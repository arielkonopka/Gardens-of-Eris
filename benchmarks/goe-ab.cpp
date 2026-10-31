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
 * goe-ab: an A/B test of four-way against eight-way movement, played by the demo's autopilot.
 *
 * Each game is headless (no window, no sound) and starts from one world seed. The seed builds
 * the world and seeds the game's random streams, the autopilot's included, so the same seed
 * plays the same game in the same arm. A game ends when the last avatar is lost, or at the tick
 * cap. An arm is a movement mode (motion::setMode) and the autopilot's ways
 * (autopilot::setWays); "8x4" is eight-way movement with an autopilot that walks the four sides.
 *
 * Usage, from GoEoOL:
 *   goe-ab --arm 8x8 --seeds 200 --first 1 --cap 20000 [--drop] [--out file.csv]
 * --drop starts the way the title screen demo does, far out in the maze, instead of the start.
 * One CSV row per game: the score (all avatars), ticks, avatars used, golden apples, keys and
 * doors, monsters and drones killed, cells visited, chunks reached, and the energy lost and
 * avatars lost by cause. tools/ab/analyze.py compares the arms.
 */
#include "autopilot.h"
#include "chamber.h"
#include "elements.h"
#include "gameClock.h"
#include "gameEvents.h"
#include "gameSerializer.h"
#include "inputManager.h"
#include "motion.h"
#include "randomStreams.h"
#include "worldBuilder.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {
/// what hurt the player
enum class cause : int { monster, patrol, collector, hunter, wallFollower, guardian, hound, drone, bunker,
                         landmine, explosion, beam, door, self, other, count };
constexpr std::array<const char *, (std::size_t) cause::count> causeNames = {
    "monster", "patrol", "collector", "hunter", "wall_follower", "guardian", "hound", "drone", "bunker",
    "landmine", "explosion", "beam", "door", "self", "other"};

cause classify(const bElem *who)
{
    if (!who)
        return cause::other;
    switch (who->getType()) {
    case bElemTypes::_monster:
        return cause::monster;
    case bElemTypes::_patrollingDrone: {
        const auto drone = dynamic_cast<const patrollingDrone *>(who);
        const auto brain = drone ? drone->getBrainModule() : nullptr;
        if (!brain)
            return cause::drone;
        switch (brain->getAttrs()->getSubtype()) {
        case puppetMasterFR::patrol: return cause::patrol;
        case puppetMasterFR::collector: return cause::collector;
        case puppetMasterFR::hunter: return cause::hunter;
        case puppetMasterFR::wallFollower: return cause::wallFollower;
        case puppetMasterFR::guardian: return cause::guardian;
        case puppetMasterFR::hound: return cause::hound;
        default: return cause::drone;
        }
    }
    case bElemTypes::_bunker:
        return cause::bunker;
    case bElemTypes::_landmineType:
        return cause::landmine;
    case bElemTypes::_simpleBombType:
    case bElemTypes::_bazookaMissileType:
    case bElemTypes::_plainMissile:
        return cause::explosion;
    case bElemTypes::_boubaType:
    case bElemTypes::_kikiType:
        return cause::beam;
    case bElemTypes::_door:
        return cause::door;
    case bElemTypes::_player:
        return cause::self;
    default:
        return cause::other;
    }
}

struct result
{
    long score = 0;
    long ticks = 0;
    int avatars = 1;
    int apples = 0, keys = 0, doors = 0, kills = 0;
    std::size_t cells = 0, chunks = 0;
    bool capped = false;
    std::array<long, (std::size_t) cause::count> damage{};
    std::array<int, (std::size_t) cause::count> deaths{};
};

/// the player acts in the next tick (as the agent library decides it, agentGame.cpp): the
/// autopilot is asked only then. The title screen asks it every tick, and a door it decides to
/// open while the player is still moving is never tried again; from the start room, whose doors
/// need the key lying inside, that would keep it in there for good.
bool readyNextTick(const bElem &e)
{
    const bElemStats &st = *e.getStats();
    const auto soon = [](int left) { return left <= 1; };
    return soon(st.getWaiting()) && soon(st.getTelInProgress()) && soon(st.getKilled()) && soon(st.getDestroyed())
           && soon(st.getMoved()) && soon(st.getFadingIn()) && soon(st.getFadingOut())
           && (!e.getAttrs()->isInteractive() || soon(st.getInteracted()));
}

/// as the title screen demo picks it (main.cpp farChunk): 23 to 63 chunks out, any direction
coords farChunk()
{
    constexpr int nearest = 23, furthest = 63;
    auto &rng = goe::rng::cosmetic();
    const int along = nearest + (int) goe::rng::below(rng, furthest - nearest + 1);
    const int across = (int) goe::rng::below(rng, 2 * along + 1) - along;
    const int sign = goe::rng::below(rng, 2) == 0 ? -1 : 1;
    return goe::rng::below(rng, 2) == 0 ? coords(sign * along, across) : coords(across, sign * along);
}

result play(unsigned seed, int movement, int pilotWays, long cap, bool drop)
{
    constexpr controlItem nothing(-1, dir::direction::NODIRECTION);
    motion::setMode(motion::modeOf(movement));
    gameSerializer::clearWorld();
    goe::rng::setWorldSeed(seed);
    goe::rng::saved().seed(seed);
    goe::rng::cosmetic().seed(seed);
    gameClock::ticks = 5;
    inputManager::getInstance(true).setControlItem(nothing);
    const auto world = worldBuilder::startNew();
    if (drop)
        worldBuilder::movePlayerTo(world, farChunk());
    goe::autopilot pilot;
    pilot.setWays(pilotWays);
    pilot.reset();

    result r;
    auto plr = player::getActivePlayer();
    unsigned long current = plr ? plr->getStats()->getInstanceId() : 0;
    long banked = 0, lastScore = plr ? plr->getStats()->getPoints(TOTAL) : 0;
    std::set<unsigned long> applesTaken, keysTaken, doorsOpened;
    std::set<std::pair<int, int>> cells, chunks;
    std::vector<cause> hurtNow;   // what hurt the active avatar in this tick
    cause lastHarm = cause::other; // what hurt it last
    goe::events::observe([&](goe::events::kind k, const bElem &subject, const bElem *actor) {
        if (k == goe::events::kind::harm) {
            if (subject.getType() == bElemTypes::_player && subject.getStats()->getInstanceId() == current)
                hurtNow.push_back(classify(actor));
            return;
        }
        if (!actor || actor->getType() != bElemTypes::_player)
            return;
        const auto id = subject.getStats()->getInstanceId();
        const int t = subject.getType();
        if (k == goe::events::kind::collect && t == bElemTypes::_goldenAppleType && applesTaken.insert(id).second)
            r.apples++;
        if (k == goe::events::kind::collect && t == bElemTypes::_key && keysTaken.insert(id).second)
            r.keys++;
        if (k == goe::events::kind::open && doorsOpened.insert(id).second)
            r.doors++;
        if (k == goe::events::kind::kill && (t == bElemTypes::_monster || t == bElemTypes::_patrollingDrone))
            r.kills++;
    });

    for (; r.ticks < cap && plr; r.ticks++) {
        const coords at = plr->getStats()->getMyPosition();
        cells.insert({at.x, at.y});
        const coords ch = chamber::chunkOf(at);
        chunks.insert({ch.x, ch.y});
        const int energy = plr->getAttrs()->getEnergy();
        hurtNow.clear();
        inputManager::getInstance(true).setControlItem(readyNextTick(*plr) ? pilot.decide(plr) : nothing);
        if (!worldBuilder::growAround(plr->getBoard(), at))
            worldBuilder::shrinkAround(plr->getBoard(), at);
        bElem::runLiveElements();
        inputManager::getInstance(true).setControlItem(nothing);

        const auto now = player::getActivePlayer();
        const bool same = now && now->getStats()->getInstanceId() == current;
        // energy lost by this avatar in this tick, put down to what hurt it; an avatar is lost
        // when it is gone from the board, some ticks after the blow that killed it, so its loss
        // goes to the last thing that harmed it
        const int lost = same ? energy - now->getAttrs()->getEnergy() : 0;
        if (lost > 0 && hurtNow.empty())
            hurtNow.push_back(cause::other);
        if (lost > 0)
            for (const cause c : hurtNow)
                r.damage[(std::size_t) c] += lost / (long) hurtNow.size();
        if (!hurtNow.empty())
            lastHarm = hurtNow.back();
        if (!same) {
            r.deaths[(std::size_t) lastHarm]++;
            lastHarm = cause::other;
        }
        if (same) {
            lastScore = now->getStats()->getPoints(TOTAL);
        } else {
            banked += lastScore;
            lastScore = 0;
            if (now) {
                r.avatars++;
                current = now->getStats()->getInstanceId();
                lastScore = now->getStats()->getPoints(TOTAL);
                // a spare avatar brings its own score from before it was woken
                banked -= lastScore;
            }
        }
        plr = now;
    }
    r.capped = plr != nullptr;
    r.score = banked + lastScore;
    r.cells = cells.size();
    r.chunks = chunks.size();
    goe::events::observe({});
    gameSerializer::clearWorld();
    return r;
}
} // namespace

int main(int argc, char **argv)
{
    std::string arm = "8x8", out;
    int seeds = 200;
    unsigned first = 1;
    long cap = 20000;
    bool drop = false;
    for (int c = 1; c < argc; c++) {
        auto next = [&]() { return c + 1 < argc ? argv[++c] : ""; };
        if (!std::strcmp(argv[c], "--arm"))
            arm = next();
        else if (!std::strcmp(argv[c], "--seeds"))
            seeds = std::atoi(next());
        else if (!std::strcmp(argv[c], "--first"))
            first = (unsigned) std::strtoul(next(), nullptr, 10);
        else if (!std::strcmp(argv[c], "--cap"))
            cap = std::atol(next());
        else if (!std::strcmp(argv[c], "--drop"))
            drop = true;
        else if (!std::strcmp(argv[c], "--out"))
            out = next();
    }
    const int movement = arm.size() == 3 && arm[0] == '4' ? 4 : 8;
    const int pilotWays = arm.size() == 3 && arm[2] == '4' ? 4 : 8;
    std::ofstream file;
    if (!out.empty())
        file.open(out);
    std::ostream &csv = out.empty() ? std::cout : file;
    csv << "arm,movement,pilot,drop,seed,score,ticks,capped,avatars,apples,keys,doors,kills,cells,chunks";
    for (auto n : causeNames)
        csv << ",damage_" << n;
    for (auto n : causeNames)
        csv << ",deaths_" << n;
    csv << "\n";
    for (int s = 0; s < seeds; s++) {
        const unsigned seed = first + (unsigned) s;
        const result r = play(seed, movement, pilotWays, cap, drop);
        csv << movement << 'x' << pilotWays << ',' << movement << ',' << pilotWays << ',' << (drop ? 1 : 0) << ','
            << seed << ',' << r.score << ',' << r.ticks << ',' << (r.capped ? 1 : 0) << ',' << r.avatars << ','
            << r.apples << ',' << r.keys << ',' << r.doors << ',' << r.kills << ',' << r.cells << ',' << r.chunks;
        for (auto d : r.damage)
            csv << ',' << d;
        for (auto d : r.deaths)
            csv << ',' << d;
        csv << "\n";
        csv.flush();
    }
    return 0;
}
