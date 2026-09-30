#include "fogMask.h"
#include "viewPoint.h"
#include <algorithm>
#include <cmath>

fogMask::fogMask(int scenePxWidth, int scenePxHeight)
    : cellsWide(std::max(1, (scenePxWidth + cellPx - 1) / cellPx))
    , cellsHigh(std::max(1, (scenePxHeight + cellPx - 1) / cellPx))
    , current(static_cast<std::size_t>(cellsWide * cellsHigh), 0)
    , next(current.size(), 0)
{}

bool fogMask::paint(std::span<const vpPoint> points, coords centreShift)
{
    std::ranges::fill(this->next, std::uint8_t{0});
    for (const vpPoint &p : points) {
        const float r = p.radius;
        if (r <= 0.0f)
            continue; // a view point that sees nothing, e.g. carried by a vanished collector
        const float px = static_cast<float>(p.x + centreShift.x);
        const float py = static_cast<float>(p.y + centreShift.y);
        const float cell = static_cast<float>(cellPx);
        // only the cells inside the radius; the fog stays cheap however many points there are
        const int x0 = std::max(0, static_cast<int>(std::floor((px - r) / cell)));
        const int x1 = std::min(this->cellsWide - 1, static_cast<int>(std::floor((px + r) / cell)));
        const int y0 = std::max(0, static_cast<int>(std::floor((py - r) / cell)));
        const int y1 = std::min(this->cellsHigh - 1, static_cast<int>(std::floor((py + r) / cell)));
        for (int cy = y0; cy <= y1; cy++) {
            const float dy = (static_cast<float>(cy) + 0.5f) * cell - py;
            std::uint8_t *row = this->next.data() + cy * this->cellsWide;
            for (int cx = x0; cx <= x1; cx++) {
                const float dx = (static_cast<float>(cx) + 0.5f) * cell - px;
                const float t = std::min(1.0f, std::sqrt(dx * dx + dy * dy) / r);
                // 1 - smoothstep(0, r, d), as the old per-pixel shader had it
                const float seen = 1.0f - t * t * (3.0f - 2.0f * t);
                const auto v = static_cast<std::uint8_t>(std::lround(seen * 255.0f));
                row[cx] = std::max(row[cx], v);
            }
        }
    }
    if (this->next == this->current)
        return false;
    std::swap(this->next, this->current);
    return true;
}
