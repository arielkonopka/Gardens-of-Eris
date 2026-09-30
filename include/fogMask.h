#ifndef FOGMASK_H
#define FOGMASK_H

#include "commons.h"
#include <cstdint>
#include <span>
#include <vector>

class vpPoint;

/**
 * How much of the scene is seen, at one value per few pixels.
 *
 * The fog used to be worked out per pixel on the GPU, looping over every view point for every
 * pixel. That is the heaviest thing a phone GPU would do all frame, so the CPU now paints this
 * small mask instead (each view point only touches the cells inside its radius) and the shader
 * reads it back with one filtered texture fetch.
 */
class fogMask
{
public:
    /// scene pixels per mask cell; the GPU's linear filter smooths the steps between cells
    static constexpr int cellPx = 8;

    /// a mask that covers a scene of the given size in pixels
    fogMask(int scenePxWidth, int scenePxHeight);

    int width() const { return cellsWide; }
    int height() const { return cellsHigh; }
    /// row by row, 0 hidden to 255 fully seen
    std::span<const std::uint8_t> texels() const { return current; }
    std::uint8_t at(int cx, int cy) const { return current[cy * cellsWide + cx]; }

    /**
     * Paints what the view points see: full sight at a point, fading smoothly to nothing at its
     * radius. Points are in scene pixels; centreShift moves them to the middle of their tile.
     * @return true when the mask changed, so an unchanged mask needs no upload
     */
    bool paint(std::span<const vpPoint> points, coords centreShift);

private:
    int cellsWide;
    int cellsHigh;
    std::vector<std::uint8_t> current;
    std::vector<std::uint8_t> next;
};

#endif // FOGMASK_H
