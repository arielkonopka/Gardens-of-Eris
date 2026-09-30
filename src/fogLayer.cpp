#include "fogLayer.h"
#include "videoManager.h"
#include "viewPoint.h"
#include <allegro5/allegro_opengl.h>
#include <array>
#include <cstring>
#include <iostream>

namespace {

/// a bitmap's real texture size; Allegro may pad it, e.g. to a power of two on older GPUs
coords textureSize(ALLEGRO_BITMAP *bmp)
{
    int w = 0, h = 0;
    if (!al_get_opengl_texture_size(bmp, &w, &h) || w <= 0 || h <= 0)
        return {al_get_bitmap_width(bmp), al_get_bitmap_height(bmp)};
    return {w, h};
}

/// creates a bitmap with the given flags and format, leaving Allegro's defaults as they were
goe::bitmapHandle createBitmap(int w, int h, int flags, int format)
{
    const int oldFlags = al_get_new_bitmap_flags();
    const int oldFormat = al_get_new_bitmap_format();
    al_set_new_bitmap_flags(flags);
    al_set_new_bitmap_format(format);
    goe::bitmapHandle bmp(al_create_bitmap(w, h));
    al_set_new_bitmap_flags(oldFlags);
    al_set_new_bitmap_format(oldFormat);
    return bmp;
}

void setVector(const char *name, const std::array<float, 4> &v)
{
    al_set_shader_float_vector(name, 4, v.data(), 1);
}

} // namespace

void fogLayer::setup(int scenePxWidth, int scenePxHeight, coords tileSize, const std::string &patternFile)
{
    this->tileSize = tileSize;
    this->shaderId = videoManager::getInstance().setupShader("data/shaders/vertexShader.glvs",
                                                             "data/shaders/pixelShader.glps");
    this->mask.emplace(scenePxWidth, scenePxHeight);
    // one byte per cell is a quarter of the upload; a linear filter blends the cells
    constexpr int maskFlags = ALLEGRO_VIDEO_BITMAP | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR;
    this->maskBitmap = createBitmap(this->mask->width(),
                                    this->mask->height(),
                                    maskFlags,
                                    ALLEGRO_PIXEL_FORMAT_SINGLE_CHANNEL_8);
    if (!this->maskBitmap)
        this->maskBitmap = createBitmap(this->mask->width(),
                                        this->mask->height(),
                                        maskFlags,
                                        ALLEGRO_PIXEL_FORMAT_ANY);
    this->upload();

    if (!patternFile.empty()) {
        this->pattern.reset(al_load_bitmap(patternFile.c_str()));
        if (!this->pattern)
            std::cerr << "Cannot load the fog bitmap " << patternFile << ", using the plain fog\n";
    }
    if (!this->pattern) {
        this->pattern = createBitmap(1, 1, ALLEGRO_VIDEO_BITMAP, ALLEGRO_PIXEL_FORMAT_ANY);
        if (this->pattern) {
            ALLEGRO_BITMAP *target = al_get_target_bitmap();
            al_set_target_bitmap(this->pattern.get());
            al_clear_to_color(plainColour());
            al_set_target_bitmap(target);
        }
    }
}

void fogLayer::upload()
{
    if (!this->maskBitmap || !this->mask)
        return;
    ALLEGRO_BITMAP *bmp = this->maskBitmap.get();
    ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap(bmp, al_get_bitmap_format(bmp), ALLEGRO_LOCK_WRITEONLY);
    if (!lr)
        return;
    const auto texels = this->mask->texels();
    const int w = this->mask->width();
    for (int y = 0; y < this->mask->height(); y++) {
        auto *row = static_cast<std::uint8_t *>(lr->data) + static_cast<std::ptrdiff_t>(y) * lr->pitch;
        const std::uint8_t *src = texels.data() + y * w;
        if (lr->pixel_size == 1) {
            std::memcpy(row, src, static_cast<std::size_t>(w));
            continue;
        }
        // a fallback format: every channel, alpha too, carries the same value
        for (int x = 0; x < w; x++)
            std::memset(row + x * lr->pixel_size, src[x], static_cast<std::size_t>(lr->pixel_size));
    }
    al_unlock_bitmap(bmp);
}

void fogLayer::draw(ALLEGRO_BITMAP *scene,
                    std::span<const vpPoint> points,
                    coords worldOrigin,
                    int sx, int sy, int w, int h, int dx, int dy)
{
    ALLEGRO_SHADER *shader = videoManager::getInstance().getShader(this->shaderId);
    if (!shader || !this->mask || !this->maskBitmap || !this->pattern) {
        al_draw_bitmap_region(scene, sx, sy, w, h, dx, dy, 0);
        return;
    }
    // standing still leaves the mask as it was, and then nothing goes to the GPU
    if (this->mask->paint(points, coords(this->tileSize.x / 2, this->tileSize.y / 2)))
        this->upload();

    const coords sceneTex = textureSize(scene);
    const coords maskTex = textureSize(this->maskBitmap.get());
    const coords patternTex = textureSize(this->pattern.get());
    const float cell = static_cast<float>(fogMask::cellPx);
    const int pw = al_get_bitmap_width(this->pattern.get());
    const int ph = al_get_bitmap_height(this->pattern.get());

    al_use_shader(shader);
    al_set_shader_sampler("fogMask", this->maskBitmap.get(), 1);
    al_set_shader_sampler("fogPattern", this->pattern.get(), 2);
    setVector("sceneMap",
              {float(sceneTex.x), float(sceneTex.y), float(al_get_bitmap_height(scene)), 0.0f});
    setVector("maskMap",
              {1.0f / (cell * float(maskTex.x)),
               1.0f / (cell * float(maskTex.y)),
               float(this->mask->height()) / float(maskTex.y),
               0.0f});
    setVector("patternMap",
              {float(pw), float(ph), float(pw) / float(patternTex.x), float(ph) / float(patternTex.y)});
    // kept inside one pattern tile, so the shader never meets huge world coordinates
    const std::array<float, 2> offset{float(floorMod(worldOrigin.x, pw)),
                                      float(floorMod(worldOrigin.y, ph))};
    al_set_shader_float_vector("patternOffset", 2, offset.data(), 1);
    al_set_shader_float("minVisibility", minVisibility);
    al_draw_bitmap_region(scene, sx, sy, w, h, dx, dy, 0);
    al_use_shader(nullptr);
}
