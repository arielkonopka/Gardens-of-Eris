#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H
#include "commons.h"

class soundManager
{
    public:
        ~soundManager();
        soundManager* getInstance();



    private:
        static soundManager* instance;
        soundManager();

};

#endif // SOUNDMANAGER_H
