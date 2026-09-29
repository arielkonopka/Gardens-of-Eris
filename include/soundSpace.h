#ifndef SOUNDSPACE_H
#define SOUNDSPACE_H
#include <array>
#include "commons.h"

/**
 * @brief Where OpenAL hears a sound on the board.
 *
 * All sources are relative to the listener, so the listener stays at the origin with the
 * default orientation: facing -z, with +y up. The board is seen from above, so a board
 * offset (dx, dy) in cells becomes (dx, -earHeight, dy): right is right, up on the screen
 * is in front of the listener, and down on the screen is behind. The listener hovers
 * earHeight cells above the board, so a sound on the player's own cell has a distance
 * and a sound one cell away is not panned all the way to one ear.
 */
namespace soundSpace {
/// cells between the listener and the board
constexpr float earHeight = 1.0f;
/// distance, in cells, at which a sound plays at its own gain; it halves at twice that
constexpr float referenceDistance = 1.0f;

/// the source position in the listener's space, in cells
inline std::array<float, 3> relative(const coords3d &source, const coords3d &listener)
{
    return {source.x - listener.x, -earHeight, source.y - listener.y};
}
} // namespace soundSpace

#endif // SOUNDSPACE_H
