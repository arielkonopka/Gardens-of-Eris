// *** ADDED BY HEADER FIXUP ***
#include <istream>
// *** END ***
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include "commons.h"
#include <allegro5/allegro.h>
#include <iostream>
#include <mutex>
#include <memory>
/** \brief This class is responsible for all user input, keys, mouse clicks, touches, pad clicks, etc...
 *
 * \param none
 *
 */


class inputManager
{
public:
    controlItem translateEvent(ALLEGRO_EVENT* ev);
    controlItem getCtrlItem();
    virtual ~inputManager();
    bool pressed_keys[ALLEGRO_KEY_MAX];
    static inputManager* getInstance();
private:
    inputManager();
    static inputManager* _instance;
    controlItem lastItem=(controlItem)
    {
        0,NODIRECTION
    };
    static std::once_flag once;

};

#endif // INPUTMANAGER_H
