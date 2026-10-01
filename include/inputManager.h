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


#include <atomic>
#include <cstdint>
#include <istream>
// *** END ***
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include "commons.h"
#include "allegroHandles.h"
#include "controlBindings.h"


#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <iostream>
#include <mutex>
#include <memory>
#include <allegro5/allegro5.h>
#include <thread>

/** \brief This class is responsible for all user input, keys, mouse clicks, touches, pad clicks, etc...
 *
 * \param none
 *
 */


class inputManager
{
public:

    controlItem getCtrlItem();
    void setControlItem(controlItem item);
    virtual ~inputManager();
    bool pressed_keys[ALLEGRO_KEY_MAX];
    /// testmode skips the keyboard, joystick and input thread, for unit tests
    static inputManager& getInstance(bool testmode = false);
    void hapticKick(float strength);
    void stop();
    /// whether the save and exit control went down since the last call; a tap shorter than
    /// a game tick counts too
    bool takeExitRequest();
    /// counts the player's presses: keys and pad buttons going down, and the stick pushed; the
    /// demo ends when it changes
    std::uint64_t activity() const { return this->presses; }
private:
    std::jthread nt;
    bool joyPresent=false;
    std::atomic<bool> exit=false; ///< set by the game thread, read by the input thread
    void inputLoop();
    controlItem translateEvent(ALLEGRO_EVENT* ev);
    inputManager();
    void startInput();
    controlItem lastItem=controlItem(0,dir::direction::NODIRECTION);
    goe::controls::inputState held; ///< keys, pad buttons and stick, as the input thread last saw them
    std::atomic<bool> exitRequest=false; ///< set by the input thread, taken by the game thread
    std::atomic<std::uint64_t> presses=0; ///< written by the input thread, read by the game thread

    static std::once_flag once;
    goe::eventQueueHandle evQueue;

};

#endif // INPUTMANAGER_H
