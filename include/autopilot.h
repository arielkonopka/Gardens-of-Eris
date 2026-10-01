#ifndef AUTOPILOT_H
#define AUTOPILOT_H
#include "commons.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>

class bElem;

namespace goe {

/**
 * Plays the active player by itself, for the title screen's demo. It is fair like the chasers:
 * it only reads the cells next to the player and the lines it could shoot along.
 *
 * Each tick it picks one control, in this order: shoot a monster or drone in a straight line
 * within reach (when it holds a gun), collect an item next to it, try a closed door next to it
 * (each door once), or walk to the neighbouring cell it has stood on least, keeping its
 * direction on a tie. Landmines are walked around.
 */
class autopilot
{
public:
    /// how far along a line it looks for something to shoot
    static constexpr int shootingRange = 5;

    /// the control for this tick; no command when there is nothing to do
    controlItem decide(const std::shared_ptr<bElem> &plr);
    /// forgets where it has been, for a new world
    void reset();

private:
    static std::uint64_t keyOf(coords c);
    /// the first direction along which a monster or drone stands in reach, with nothing between
    std::optional<dir::direction> target(const std::shared_ptr<bElem> &plr) const;

    std::unordered_map<std::uint64_t, int> visits;
    std::set<unsigned long> triedDoors;
    coords last = NOCOORDS;
    dir::direction heading = dir::direction::UP;
};

} // namespace goe

#endif // AUTOPILOT_H
