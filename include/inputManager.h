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


#include <istream>
// *** END ***
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include "commons.h"


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
    static inputManager* getInstance();
    static inputManager* getInstance(bool testmode);
    void hapticKick(float strength);
    void stop();
private:
    float sesitivity=0.4;

    std::thread nt;
    bool joyPresent=false;
    ALLEGRO_JOYSTICK *joystick;
    bool exit=false;
    void inputLoop();
    controlItem translateEvent(ALLEGRO_EVENT* ev);
    inputManager();
    static inputManager* _instance;
    controlItem lastItem=(controlItem)
    {
        0,dir::direction::NODIRECTION
    };
    static std::once_flag once;
    ALLEGRO_EVENT_QUEUE* evQueue;

};

#endif // INPUTMANAGER_H
