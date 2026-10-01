#include "agentFeatures.h"
#include "difficulty.h"
#include "elements.h"
#include <array>
#include <stdexcept>

namespace goe::agent {
namespace {
float flag(bool b)
{
    return b ? 1.0f : 0.0f;
}
/// a timed state as ticks left: the stats getters give -1 when the state is over
float ticksLeft(int v)
{
    return v > 0 ? (float) v : 0.0f;
}
int itemsHeld(bElem &e)
{
    const auto inv = e.getAttrs()->getInventory();
    if (!inv)
        return 0;
    return (int) (inv->weapons.size() + inv->usables.size() + inv->keys.size() + inv->mods.size()
                  + inv->tokens.size());
}
/// how many elements are stacked under this one on its cell
int stackBelow(bElem &e)
{
    int n = 0;
    for (auto under = e.getStats()->getSteppingOn(); under; under = under->getStats()->getSteppingOn())
        n++;
    return n;
}

// clang-format off
const std::array elements = {
    feature{"type", "element type (bElemTypes); -1 for none", [](bElem &e) { return (float) e.getType(); }, -1.0f},
    feature{"subtype", "element subtype; -1 for none", [](bElem &e) { return (float) e.getAttrs()->getSubtype(); }, -1.0f},
    feature{"energy", "energy (health)", [](bElem &e) { return (float) e.getAttrs()->getEnergy(); }},
    feature{"max_energy", "maximum energy", [](bElem &e) { return (float) e.getAttrs()->getMaxEnergy(); }},
    feature{"ammo", "ammunition (guns)", [](bElem &e) { return (float) e.getAttrs()->getAmmo(); }},
    feature{"max_ammo", "maximum ammunition", [](bElem &e) { return (float) e.getAttrs()->getMaxAmmo(); }},
    feature{"killable", "can be killed", [](bElem &e) { return flag(e.getAttrs()->isKillable()); }},
    feature{"destroyable", "can be destroyed", [](bElem &e) { return flag(e.getAttrs()->isDestroyable()); }},
    feature{"steppable", "can be stepped on now", [](bElem &e) { return flag(e.getAttrs()->isSteppable()); }},
    feature{"movable", "can move", [](bElem &e) { return flag(e.getAttrs()->isMovable()); }},
    feature{"interactive", "reacts to the interact action", [](bElem &e) { return flag(e.getAttrs()->isInteractive()); }},
    feature{"collectible", "can be collected", [](bElem &e) { return flag(e.getAttrs()->isCollectible()); }},
    feature{"can_push", "pushes what it walks into", [](bElem &e) { return flag(e.getAttrs()->canPush()); }},
    feature{"can_be_pushed", "can be pushed or dragged", [](bElem &e) { return flag(e.getAttrs()->canBePushed()); }},
    feature{"can_collect", "collects things (has an inventory)", [](bElem &e) { return flag(e.getAttrs()->canCollect()); }},
    feature{"weapon", "is a weapon", [](bElem &e) { return flag(e.getAttrs()->isWeapon()); }},
    feature{"open", "is open (doors)", [](bElem &e) { return flag(e.getAttrs()->isOpen()); }},
    feature{"locked", "is locked (doors)", [](bElem &e) { return flag(e.getAttrs()->isLocked()); }},
    feature{"mod", "is a modifier", [](bElem &e) { return flag(e.getAttrs()->isMod()); }},
    feature{"active", "is the active player, or an activated element", [](bElem &e) { return flag(e.getStats()->isActive()); }},
    feature{"busy", "in a timed state, so it cannot act", [](bElem &e) { return flag(e.getStats()->busy()); }},
    feature{"moving", "ticks left of a move", [](bElem &e) { return ticksLeft(e.getStats()->getMoved()); }},
    feature{"waiting", "ticks left of waiting", [](bElem &e) { return ticksLeft(e.getStats()->getWaiting()); }},
    feature{"dying", "ticks left of dying", [](bElem &e) { return ticksLeft(e.getStats()->getKilled()); }},
    feature{"destroying", "ticks left of being destroyed", [](bElem &e) { return ticksLeft(e.getStats()->getDestroyed()); }},
    feature{"teleporting", "ticks left of teleporting", [](bElem &e) { return ticksLeft(e.getStats()->getTelInProgress()); }},
    feature{"fading_in", "ticks left of fading in", [](bElem &e) { return ticksLeft(e.getStats()->getFadingIn()); }},
    feature{"fading_out", "ticks left of fading out", [](bElem &e) { return ticksLeft(e.getStats()->getFadingOut()); }},
    feature{"interacting", "ticks left of an interaction", [](bElem &e) { return ticksLeft(e.getStats()->getInteracted()); }},
    feature{"facing", "where it faces (0 up, 1 left, 2 down, 3 right, 4 none)", [](bElem &e) { return (float) e.getStats()->getFacing(); }, 4.0f},
    feature{"direction", "where it moves (0 up, 1 left, 2 down, 3 right, 4 none)", [](bElem &e) { return (float) e.getStats()->getMyDirection(); }, 4.0f},
    feature{"items", "how many things it carries", [](bElem &e) { return (float) itemsHeld(e); }},
    feature{"stack", "how many elements lie under it on its cell", [](bElem &e) { return (float) stackBelow(e); }},
    feature{"below_type", "type of the element right under it; -1 for none", [](bElem &e) {
        const auto under = e.getStats()->getSteppingOn();
        return under ? (float) under->getType() : -1.0f;
    }, -1.0f},
};

coords positionFromOrigin(bElem &e)
{
    const auto board = e.getBoard();
    const coords at = e.getStats()->getMyPosition();
    return board && board->origin != NOCOORDS ? at - board->origin : at;
}

const std::array players = {
    feature{"x", "column, counted from the middle of the start area", [](bElem &e) { return (float) positionFromOrigin(e).x; }},
    feature{"y", "row, counted from the middle of the start area (grows downwards)", [](bElem &e) { return (float) positionFromOrigin(e).y; }},
    feature{"score", "total points", [](bElem &e) { return (float) e.getStats()->getPoints(TOTAL); }},
    feature{"shots", "shooting points (hits landed)", [](bElem &e) { return (float) e.getStats()->getPoints(SHOOT); }},
    feature{"steps", "steps statistic", [](bElem &e) { return (float) e.getStats()->getStats(STEPS); }},
    feature{"collects", "collecting points", [](bElem &e) { return (float) e.getStats()->getPoints(COLLECTS); }},
    feature{"view_radius", "how far the player sees; grows with steps", [](bElem &e) { return e.getViewRadius(); }},
    feature{"dex", "player level: floor(log5(shots + 1))", [](bElem &e) { return (float) difficulty::playerLevel(e.shared_from_this()); }},
    feature{"difficulty", "D = Dex + distance from the start", [](bElem &e) { return (float) difficulty::of(e.shared_from_this()); }},
    feature{"avatars", "spare avatars collected", [](bElem &) { return (float) player::countVisitedPlayers(); }},
};
// clang-format on
} // namespace

std::span<const feature> elementFeatures()
{
    return elements;
}

std::span<const feature> playerFeatures()
{
    return players;
}

std::vector<std::string> namesOf(std::span<const feature> table)
{
    std::vector<std::string> res;
    for (const auto &f : table)
        res.emplace_back(f.name);
    return res;
}

std::vector<feature> select(const std::vector<std::string> &names,
                            std::initializer_list<std::span<const feature>> tables)
{
    std::vector<feature> res;
    for (const auto &name : names) {
        bool found = false;
        for (const auto &table : tables) {
            for (const auto &f : table)
                if (f.name == name) {
                    res.push_back(f);
                    found = true;
                    break;
                }
            if (found)
                break;
        }
        if (!found)
            throw std::invalid_argument("unknown feature: " + name);
    }
    return res;
}

} // namespace goe::agent
