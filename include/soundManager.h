#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H
#include "commons.h"
#include <map>
#include <vector>
#include <iostream>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include "configManager.h"
#include <sndfile.h>


using coords3d=struct crd3d
{
    int x=-1;
    int y=-1;
    int z=-1;
};
using sndInstance=struct sampleInstance
{
    int sndSpace=-1; // -1 is no space assigned
    coords3d position={-1,-1,-1};
    ALuint src;
    int mode=-1;

};

using sndHolder=struct sampleS
{
    bool loaded=false; // By default it is not loaded
    ALuint buffer;
    int mode;

};

using stNode=struct sndNode
{
    bool isRegistered=false;
    ALuint source;
    int mode;
    int id=-1;
};




class soundManager
{
    public:
        ~soundManager();
        static soundManager* getInstance();
        int registerSound(int instanceId,int typeId,int subtypeId,std::string eventType,std::string event);
        bool playSound(int sndId,int chmbrId);
        void setSoundPosition(int sndId,coords3d pos);
        void setSoundVelocity(int sndId,coords3d pos);
        void setListenerPosition(coords3d pos);
        void setListenerVelocity(coords3d pos);
        void setListenerChamber(int chamberId);
        void setListenerOrientation(coords3d pos);
        int getSndStatus(int sndId);
        bool stopSnd(int sndId);
        bool pauseSnd(int sndId);
        bool resumeSnd(int sndId);
        void deregisterSnd(int sndId);
        void setActiveSndSpace(int sndSpace);


   // private:
        void stopSoundsInSpace(int space);
        void startSoundsInSpace(int space);
        ALuint loadSample(std::string fname);
        static soundManager* instance;
        soundManager();
        ALCdevice *sndDevice;
        ALCcontext *sndContext;
        std::map<int, std::map< int, std::map<std::string,std::map<std::string, sndHolder>>>> samplesLoaded;

        std::vector<stNode> registeredSounds; // the whole sample data, used to register sounds
        std::map<int,std::vector<ALint>> soundSpaces;
        int currSoundSpace=-1;




};

#endif // SOUNDMANAGER_H
