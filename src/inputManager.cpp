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
inputManager* inputManager::_instance;
std::once_flag inputManager::once;

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
    dir::direction dir=dir::direction::NODIRECTION;
    int type=-1;

    switch (ev->type)
    {
    case (ALLEGRO_EVENT_JOYSTICK_AXIS):

        this->pressed_keys[ALLEGRO_KEY_DOWN]=false;
        this->pressed_keys[ALLEGRO_KEY_UP]=false;
        this->pressed_keys[ALLEGRO_KEY_LEFT]=false;
        this->pressed_keys[ALLEGRO_KEY_RIGHT]=false;
        if(ev->joystick.axis==0)
        {
            if(ev->joystick.pos>this->sesitivity)
                this->pressed_keys[ALLEGRO_KEY_RIGHT]=true;
            else if(ev->joystick.pos<-this->sesitivity)
                this->pressed_keys[ALLEGRO_KEY_LEFT]=true;
        }
        else if(ev->joystick.axis==1)
        {
            if(ev->joystick.pos<-this->sesitivity)
                this->pressed_keys[ALLEGRO_KEY_UP]=true;
            else if(ev->joystick.pos>this->sesitivity)
                this->pressed_keys[ALLEGRO_KEY_DOWN]=true;

        }

        break;
    case (ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN):
        switch (ev->joystick.button)
        {
        case 5:

            this->pressed_keys[ALLEGRO_KEY_RSHIFT]=true;
            break;
        case 6:
            this->pressed_keys[ALLEGRO_KEY_SPACE]=true;
            break;
        case 0:
            this->pressed_keys[ALLEGRO_KEY_LCTRL]=true;
            break;
        case 1:
            this->pressed_keys[ALLEGRO_KEY_ALT]=true;
            break;
        case 2:
            this->pressed_keys[ALLEGRO_KEY_Z]=true;
            break;
        case 3:
            this->pressed_keys[ALLEGRO_KEY_X]=true;
            break;
        case 9:
            this->pressed_keys[ALLEGRO_KEY_ESCAPE]=true;
            break;
        case 8:
            this->pressed_keys[ALLEGRO_KEY_R]=true;
            break;
        case 4:
            this->pressed_keys[ALLEGRO_KEY_SPACE]=true;
            break;


        }
        break;
    case (ALLEGRO_EVENT_JOYSTICK_BUTTON_UP):
        switch (ev->joystick.button)
        {
        case 5:
            this->pressed_keys[ALLEGRO_KEY_RSHIFT]=false;
            break;
        case 6:
            this->pressed_keys[ALLEGRO_KEY_SPACE]=false;
            break;
        case 0:
            this->pressed_keys[ALLEGRO_KEY_LCTRL]=false;
            break;
        case 1:
            this->pressed_keys[ALLEGRO_KEY_ALT]=false;
            break;
        case 2:
            this->pressed_keys[ALLEGRO_KEY_Z]=false;
            break;
        case 3:
            this->pressed_keys[ALLEGRO_KEY_X]=false;
            break;
        case 9:
            this->pressed_keys[ALLEGRO_KEY_ESCAPE]=false;
            break;
        case 8:
            this->pressed_keys[ALLEGRO_KEY_R]=false;
            break;
        case 4:
            this->pressed_keys[ALLEGRO_KEY_SPACE]=false;
            break;


        }


        break;

    case (ALLEGRO_EVENT_KEY_DOWN ):
    {

        int c=ev->keyboard.keycode;
        this->pressed_keys[c] = true;
        break;
    };
    case (ALLEGRO_EVENT_KEY_UP ):
    {

        int c=ev->keyboard.keycode;
        this->pressed_keys[c] = false;
        break;
    }
    }
    if (this->pressed_keys[ALLEGRO_KEY_W] || this->pressed_keys[ALLEGRO_KEY_UP])
    {
        type=0;
        dir=dir::direction::UP;
    }
    if (this->pressed_keys[ALLEGRO_KEY_S] || this->pressed_keys[ALLEGRO_KEY_DOWN] )
    {
        type=0;
        dir=dir::direction::DOWN;
    }
    if (this->pressed_keys[ALLEGRO_KEY_A] || this->pressed_keys[ALLEGRO_KEY_LEFT])
    {
        type=0;
        dir=dir::direction::LEFT;
    }
    if (this->pressed_keys[ALLEGRO_KEY_D] || this->pressed_keys[ALLEGRO_KEY_RIGHT])
    {
        dir=dir::direction::RIGHT;
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
    if(this->pressed_keys[ALLEGRO_KEY_SPACE])
        type=8;
    if(this->pressed_keys[ALLEGRO_KEY_R])
        type=9;
    this->lastItem=(controlItem)
    {
        type,dir
    };
    return this->lastItem;
}
controlItem inputManager::getCtrlItem()
{
    return this->lastItem;
}

void inputManager::setControlItem(controlItem item)
{
    this->lastItem=item;
}
void inputManager::inputLoop()
{
    bool evCollected=false;
    ALLEGRO_EVENT event;
    while (!this->exit)
    {
        evCollected=al_wait_for_event_timed(this->evQueue, &event,0.1);
        if(evCollected)
            this->translateEvent(&event);
    }

}
void inputManager::stop()
{
    std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    this->exit=true;


}
void inputManager::hapticKick(float strength)
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

inputManager* inputManager::getInstance()
{
    return inputManager::getInstance(false);
}

inputManager* inputManager::getInstance(bool testmode)
{
    std::call_once(once, [testmode]()
    {
        inputManager::_instance=new inputManager();
        if(!testmode)
        {
            al_install_keyboard();
            al_install_joystick();
            _instance->evQueue= al_create_event_queue();
            al_register_event_source(_instance->evQueue, al_get_keyboard_event_source());
            al_register_event_source(_instance->evQueue, al_get_joystick_event_source());
            if (al_get_num_joysticks() > 0)
            {
                _instance->joystick = al_get_joystick(al_get_num_joysticks()-1); // take first joystick
                _instance->joyPresent=true;
                //  std::cout<<"getting haptic "<<al_get_num_joysticks()<<"\n";
                //   _instance->haptic=al_get_haptic_from_joystick(_instance->joystick);
                //   std::cout<<"took haptic\n";
            }

            _instance->nt=std::thread(&inputManager::inputLoop, _instance);
            _instance->nt.detach();
        }

    });
    return inputManager::_instance;
}







