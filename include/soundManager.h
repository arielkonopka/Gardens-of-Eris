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
#include <deque>


using coords3d=struct crd3d
{
    int x=-1;
    int y=-1;
    int z=-1;
};


using sndHolder=struct sampleS
{
    bool loaded=false; // By default it is not loaded
    ALuint buffer;
    int mode;
    bool allowMulti=false;
    coords3d position;
    coords3d velocity;

};




using stNode=struct sndNode
{
    bool isRegistered=false;
    ALuint source;
    int mode;
    bool allowMulti=false;
    int soundSpace;
    int elId=0;
    std::string eventType;
    std::string event;
    inline sndNode operator=(sndNode a)
    {
        isRegistered=a.isRegistered;
        source=a.source;
        mode=a.mode;
        allowMulti=a.allowMulti;
        soundSpace=a.soundSpace;
        return a;
    }
};

using regNode=struct _regnd
{
    bool r=false;
};


class soundManager
{
public:
    ~soundManager();
    static soundManager* getInstance();
    void registerSound(int chamberId,coords3d position,coords3d velocity,int elId,int typeId,int subtypeId,std::string eventType,std::string event);

    void setListenerPosition(coords3d pos);
    void setListenerVelocity(coords3d pos);
    void setListenerChamber(int chamberId);
    void setListenerOrientation(coords3d pos);
private:
    void setSoundPosition(stNode snd,coords3d pos);
    void setSoundVelocity(stNode snd,coords3d pos);
    bool stopSnd(stNode n);
    stNode getSndNode();
    ALuint loadSample(std::string fname);
    static soundManager* instance;
    soundManager();
    ALCdevice *sndDevice;
    ALCcontext *sndContext;
    std::map<int, std::map< int, std::map<std::string,std::map<std::string, sndHolder>>>> samplesLoaded;
    std::deque<stNode> registeredSounds; // the whole sample data, used to register sounds
    std::map<int,std::map<std::string,std::map<std::string,regNode>>> sndRegister;
    int currSoundSpace=-1;
    bool isSndPlaying(ALint sndId);
    void checkQueue();
    coords3d listenerPos= {0,0,0};
    std::shared_ptr<gameConfig> gc;
     std::shared_ptr<configManager> cm;

};

#endif // SOUNDMANAGER_H
