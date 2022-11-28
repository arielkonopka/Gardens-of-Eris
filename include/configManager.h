#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include "commons.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
// Singleton pattern, we will not have multiple config files, therefore we do not need multiple instances of config manager
// this class will read the config json file, and expose gathered information to whatever class needing it

class configManager
{
    public:
       static configManager* getInstance();
        ~configManager();
        rapidjson::Document *getConfig();
        void configReload();

    protected:

    private:
        int tileWidth;
        int tileHeight;
        rapidjson::Document skinDefJson;
        configManager();
        static configManager* instance;
};

#endif // CONFIGMANAGER_H
