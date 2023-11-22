// *** ADDED BY HEADER FIXUP ***
#include <istream>
// *** END ***
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include "commons.h"
#define ALLEGRO_UNSTABLE

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/haptic.h>
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
    void hapticKick(float strength);
    void stop();
private:

/*  ALLEGRO_HAPTIC *haptic;
    ALLEGRO_HAPTIC_EFFECT effect;
    ALLEGRO_HAPTIC_EFFECT_ID id;
*/

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
        0,NODIRECTION
    };
    static std::once_flag once;
    ALLEGRO_EVENT_QUEUE* evQueue;

};

#endif // INPUTMANAGER_H
