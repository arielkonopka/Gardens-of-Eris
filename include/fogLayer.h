#ifndef FOGLAYER_H
#define FOGLAYER_H

#include "allegroHandles.h"
#include "commons.h"
#include "fogMask.h"
#include <allegro5/allegro.h>
#include <optional>
#include <span>
#include <string>

/**
 * Draws the game field under the fog of view.
 *
 * What nobody sees is covered by the fog pattern, a bitmap set by "FogBitmap" in skins.json and
 * tiled over the world so it stays put while the view scrolls. Without that entry, or when the
 * file cannot be loaded, the fog is the plain dark colour the game always had.
 */
class fogLayer
{
public:
    /// the fog when no pattern is configured, also the colour behind the game field
    static ALLEGRO_COLOR plainColour() { return al_map_rgb(15, 25, 45); }
    /// how much of a hidden spot still shows through the fog
    static constexpr float minVisibility = 0.05f;

    /**
     * Needs the display. scenePxWidth and scenePxHeight are the size of the scene bitmap,
     * tileSize its tiles in pixels, patternFile the fog bitmap (empty for the plain fog).
     */
    void setup(int scenePxWidth, int scenePxHeight, coords tileSize, const std::string &patternFile);

    /**
     * Paints the mask for these view points (in scene pixels) and uploads it when it changed.
     * draw does this itself; a view with a shader of its own calls it and reads the textures.
     */
    void refresh(std::span<const vpPoint> points);
    /// the mask, one texel per fogMask::cellPx scene pixels, kept upside down like any bitmap
    ALLEGRO_BITMAP *maskTexture() const { return this->maskBitmap.get(); }
    /// texels across and down the mask
    coords maskSize() const { return this->mask ? coords(this->mask->width(), this->mask->height()) : coords(0, 0); }
    /// the fog bitmap (or one texel of the plain colour), tiled over the world
    ALLEGRO_BITMAP *patternTexture() const { return this->pattern.get(); }
    /// a bitmap's texture size; larger than the bitmap when the GPU pads it
    static coords textureSize(ALLEGRO_BITMAP *bmp);

    /**
     * Draws the region (sx, sy, w, h) of the scene at (dx, dy), fogged where no view point sees.
     * points are the view points in scene pixels, worldOrigin is the world position in pixels of
     * the scene's top left corner.
     */
    void draw(ALLEGRO_BITMAP *scene,
              std::span<const vpPoint> points,
              coords worldOrigin,
              int sx, int sy, int w, int h, int dx, int dy);

private:
    void upload();
    int shaderId = -1;
    coords tileSize{0, 0};
    std::optional<fogMask> mask;
    goe::bitmapHandle maskBitmap;
    goe::bitmapHandle pattern;
};

#endif // FOGLAYER_H
