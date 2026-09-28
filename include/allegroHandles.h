/*
 * Owning smart pointers for Allegro and OpenAL objects, so nothing has to remember to free them.
 * Pass handle.get() to C functions that take the raw object; the handle keeps ownership.
 */
#ifndef ALLEGROHANDLES_H
#define ALLEGROHANDLES_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <memory>

namespace goe {
/// calls Destroy(p) when the owning pointer lets go of a non-null object
template <auto Destroy>
struct destroyWith
{
    template <typename T>
    void operator()(T *p) const noexcept
    {
        if (p)
            Destroy(p);
    }
};

using bitmapHandle = std::unique_ptr<ALLEGRO_BITMAP, destroyWith<al_destroy_bitmap>>;
using fontHandle = std::unique_ptr<ALLEGRO_FONT, destroyWith<al_destroy_font>>;
using timerHandle = std::unique_ptr<ALLEGRO_TIMER, destroyWith<al_destroy_timer>>;
using eventQueueHandle = std::unique_ptr<ALLEGRO_EVENT_QUEUE, destroyWith<al_destroy_event_queue>>;
using displayHandle = std::unique_ptr<ALLEGRO_DISPLAY, destroyWith<al_destroy_display>>;
using shaderHandle = std::unique_ptr<ALLEGRO_SHADER, destroyWith<al_destroy_shader>>;
} // namespace goe

#endif // ALLEGROHANDLES_H
