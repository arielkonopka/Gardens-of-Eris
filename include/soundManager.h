#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H
#include "commons.h"
#include <map>
#include <vector>



typedef struct sampleInstance
{
    int sndSpace=-1; // -1 is no space assigned



} sndInstance;

typedef struct sampleS
{
    bool loaded=false; // By default it is not loaded


} sndHolder;

typedef struct stNode
{
    bool isRegistered=false;
    int id=-1;
} stNode;



class soundManager
{
    public:
        ~soundManager();
        soundManager* getInstance();
        int registerSound(int instanceId,int typeId,int subtypeId,std::string eventType,std::string event);
        bool playSound(int sndId,int chmbrId);
        int getSndStatus(int sndId);
        bool stopSnd(int sndId);
        bool pauseSnd(int sndId);
        bool resumeSnd(int sndId);
        void deregisterSnd(int sndId);



    private:
        bool checkSmpExists(int instanceId,int typeId,int subtypeId,std::string eventType,std::string event); // check if the sample is already registered
        static soundManager* instance;
        soundManager();

        std::map<int, std::map< int, std::map<std::string,std::map<std::string, sndHolder>>>> samplesLoaded;
        std::vector<std::shared_ptr<sndInstance>> registeredSampleTab;
        std::map<int , std::map<int, std::map< int, std::map<std::string,std::map<std::string, stNode>>>>> registeredSounds; // the whole sample data, used to register sounds
        std::map<int,std::vector<sndInstance>> soundSpaces;




};

#endif // SOUNDMANAGER_H
