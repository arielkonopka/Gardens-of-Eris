/*
 * Copyright (c) 2023, Ariel Konopka
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
#include "inputManager.h"
#include "gameSettings.h"
std::once_flag inputManager::once;

inputManager::inputManager()
{
    for (int c = 0; c < ALLEGRO_KEY_MAX; c++) {
        this->pressed_keys[c] = false;
    }
}

inputManager::~inputManager()
{
    this->exit = true;
    if (this->nt.joinable())
        this->nt.join(); // the loop wakes up at least every 0.1 s
}
controlItem inputManager::translateEvent(ALLEGRO_EVENT *ev)
{
    switch (ev->type) {
    case ALLEGRO_EVENT_JOYSTICK_AXIS:
        // the stick walks: axis 0 is left and right, axis 1 is up and down
        if (ev->joystick.axis == 0) {
            this->held.stick[2] = ev->joystick.pos < -this->sesitivity;
            this->held.stick[3] = ev->joystick.pos > this->sesitivity;
        } else if (ev->joystick.axis == 1) {
            this->held.stick[0] = ev->joystick.pos < -this->sesitivity;
            this->held.stick[1] = ev->joystick.pos > this->sesitivity;
        }
        break;
    case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
    case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
        if (ev->joystick.button >= 0 && ev->joystick.button < goe::controls::padButtons)
            this->held.pad[ev->joystick.button] = ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
        break;
    case ALLEGRO_EVENT_KEY_DOWN:
    case ALLEGRO_EVENT_KEY_UP:
        this->pressed_keys[ev->keyboard.keycode] = ev->type == ALLEGRO_EVENT_KEY_DOWN;
        this->held.keys[ev->keyboard.keycode] = this->pressed_keys[ev->keyboard.keycode];
        break;
    }
    // the Config menu may have changed the layout since the last event
    this->lastItem = gameSettings::getInstance().getControls().translate(this->held);
    return this->lastItem;
}
controlItem inputManager::getCtrlItem()
{
    return this->lastItem;
}

void inputManager::setControlItem(controlItem item)
{
    this->lastItem = item;
}
void inputManager::inputLoop()
{
    bool evCollected = false;
    ALLEGRO_EVENT event;
    while (!this->exit) {
        evCollected = al_wait_for_event_timed(this->evQueue.get(), &event, 0.1);
        if (evCollected)
            this->translateEvent(&event);
    }
}
void inputManager::stop()
{
    this->exit = true;
}
void inputManager::hapticKick(float /*strength*/)
{
    /*
        if (!this->haptic)
        {
            return;
        }
        al_set_haptic_gain(this->haptic, strength);
        this->effect.type = ALLEGRO_HAPTIC_RUMBLE;
        this->effect.data.rumble.strong_magnitude = strength;
        this->effect.data.rumble.weak_magnitude = strength;
        this->effect.replay.delay = 0.1;
        this->effect.replay.length = strength/10;
        al_upload_haptic_effect(this->haptic, &this->effect, &this->id);
        al_play_haptic_effect(&this->id, 1);

    */
}

inputManager &inputManager::getInstance(bool testmode)
{
    static inputManager instance;
    std::call_once(once, [testmode]() {
        if (!testmode)
            instance.startInput();
    });
    return instance;
}

void inputManager::startInput()
{
    al_install_keyboard();
    al_install_joystick();
    this->evQueue.reset(al_create_event_queue());
    al_register_event_source(this->evQueue.get(), al_get_keyboard_event_source());
    al_register_event_source(this->evQueue.get(), al_get_joystick_event_source());
    this->joyPresent = al_get_num_joysticks() > 0;
    this->nt = std::jthread(&inputManager::inputLoop, this);
}
