// *** ADDED BY HEADER FIXUP ***
#include <istream>
// *** END ***
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include "commons.h"
#include <allegro5/allegro.h>
#include <iostream>
/** \brief This class is responsible for all user input, keys, mouse clicks, touches, pad clicks, etc...
 *
 * \param none
 *
 */


class inputManager
{
    public:
        controlItem translateEvent(ALLEGRO_EVENT* ev);
        inputManager();
        virtual ~inputManager();
        bool pressed_keys[ALLEGRO_KEY_MAX];

};

#endif // INPUTMANAGER_H
