#include "inputManager.h"

inputManager::inputManager()
{
    for(int c=0; c<ALLEGRO_KEY_MAX; c++)
    {
        this->pressed_keys[c]=false;
    }
}

inputManager::~inputManager()
{
    //dtor
}
controlItem inputManager::translateEvent(ALLEGRO_EVENT* ev)
{
    direction dir=UP;
    int type=-1;

    switch (ev->type)
    {

    case (ALLEGRO_EVENT_KEY_DOWN):
    {


        int c=ev->keyboard.keycode;
        this->pressed_keys[c] = true;
        break;
    }
    case (ALLEGRO_EVENT_KEY_UP):
    {

        int c=ev->keyboard.keycode;
        this->pressed_keys[c] = false;
        break;
    }
    }
    if (this->pressed_keys[ALLEGRO_KEY_W] || this->pressed_keys[ALLEGRO_KEY_UP])
    {
        type=0;
        dir=UP;
    }
    if (this->pressed_keys[ALLEGRO_KEY_S] || this->pressed_keys[ALLEGRO_KEY_DOWN] )
    {
        type=0;
        dir=DOWN;
    }
    if (this->pressed_keys[ALLEGRO_KEY_A] || this->pressed_keys[ALLEGRO_KEY_LEFT])
    {
        type=0;
        dir=LEFT;
    }
    if (this->pressed_keys[ALLEGRO_KEY_D] || this->pressed_keys[ALLEGRO_KEY_RIGHT])
    {
        dir=RIGHT;
        type=0;
    }
    if ((this->pressed_keys[ALLEGRO_KEY_RSHIFT] || this->pressed_keys[ALLEGRO_KEY_LSHIFT]) && type>=0)
        type=1;
    if ((this->pressed_keys[ALLEGRO_KEY_RCTRL] || this->pressed_keys[ALLEGRO_KEY_LCTRL]) && type>=0)
        type=2;
    if (this->pressed_keys[ALLEGRO_KEY_X])
        type=3;
    if ((this->pressed_keys[ALLEGRO_KEY_ALT] || this->pressed_keys[ALLEGRO_KEY_ALTGR]) && type>=0)
        type=4;
    if (this->pressed_keys[ALLEGRO_KEY_Z])
        type=5;
    if (this->pressed_keys[ALLEGRO_KEY_ESCAPE] )
        type=6;
    if (this->pressed_keys[ALLEGRO_KEY_ESCAPE] && this->pressed_keys[ALLEGRO_KEY_LSHIFT])
        type=7;
    return (controlItem)
    {
        type,dir
    };
}
