#include "autopilot.h"
#include "bElem.h"
#include "chamber.h"
#include "inventory.h"
#include "lineOfSight.h"
#include "randomStreams.h"
#include <algorithm>
#include <array>
#include <vector>

namespace goe {
namespace {
constexpr std::array<dir::direction, 4> ways = {dir::direction::UP, dir::direction::LEFT, dir::direction::DOWN,
                                                dir::direction::RIGHT};

bool isPrey(const bElem &e)
{
    const int t = e.getType();
    return (t == bElemTypes::_monster || t == bElemTypes::_patrollingDrone) && e.getAttrs()->isKillable();
}
} // namespace

std::uint64_t autopilot::keyOf(coords c)
{
    return ((std::uint64_t) (std::uint32_t) c.x << 32) | (std::uint32_t) c.y;
}

void autopilot::reset()
{
    this->visits.clear();
    this->triedDoors.clear();
    this->last = NOCOORDS;
    this->heading = dir::direction::UP;
}

std::optional<dir::direction> autopilot::target(const std::shared_ptr<bElem> &plr) const
{
    const auto board = plr->getBoard();
    const coords me = plr->getStats()->getMyPosition();
    for (auto d : ways)
        for (int step = 1; step <= shootingRange; step++) {
            const auto e = board->getElement(me + dir::dirToCoords(d) * step);
            if (!e)
                break;
            if (isPrey(*e))
                return d;
            // a missile stops at the first thing that is not floor
            if (e->getType() != bElemTypes::_floorType || goe::sight::opaque(e))
                break;
        }
    return std::nullopt;
}

controlItem autopilot::decide(const std::shared_ptr<bElem> &plr)
{
    constexpr controlItem nothing = controlItem(-1, dir::direction::NODIRECTION);
    if (!plr || !plr->getBoard())
        return nothing;
    const coords me = plr->getStats()->getMyPosition();
    if (!(me == this->last)) {
        this->visits[keyOf(me)]++;
        this->last = me;
    }
    const auto inv = plr->getAttrs()->getInventory();
    if (inv && inv->getActiveWeapon())
        if (const auto d = this->target(plr))
            return controlItem(1, *d);

    std::vector<dir::direction> open;
    for (auto d : ways) {
        const auto e = plr->getElementInDirection(d);
        if (!e)
            continue;
        if (e->getAttrs()->isCollectible())
            return controlItem(0, d);
        if (e->getType() == bElemTypes::_door && !e->getAttrs()->isOpen()
            && this->triedDoors.insert(e->getStats()->getInstanceId()).second)
            return controlItem(2, d);
        if (e->getAttrs()->isSteppable() && e->getType() != bElemTypes::_landmineType)
            open.push_back(d);
    }
    if (open.empty())
        return nothing;
    // the least trodden way; on a tie, straight on, else any of them
    int fewest = -1;
    std::vector<dir::direction> best;
    for (auto d : open) {
        const auto it = this->visits.find(keyOf(me + dir::dirToCoords(d)));
        const int n = it == this->visits.end() ? 0 : it->second;
        if (fewest < 0 || n < fewest) {
            fewest = n;
            best.clear();
        }
        if (n == fewest)
            best.push_back(d);
    }
    if (std::find(best.begin(), best.end(), this->heading) == best.end())
        this->heading = goe::rng::pick(goe::rng::cosmetic(), best);
    return controlItem(0, this->heading);
}

} // namespace goe
