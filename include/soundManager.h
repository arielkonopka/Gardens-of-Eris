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
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>
#include "bElem.h"
#include <memory>
#include "commons.h"

using sndHolder=struct sampleS
{
    bool loaded=false; // By default it is not loaded
    ALuint buffer;
    int mode;
    bool allowMulti=false;


};


using muNode=struct _mudNode
{
    bool isRegistered=false;
    int songNo=-1;
    int delayed=-1;
    ALuint source=0;
    ALuint Abuffers[5];
    std::vector<std::vector<short>> dataStuff;
    SNDFILE *musicFile=nullptr;
    SF_INFO musFileinfo;
    coords3d position;
    int chamberId;
    ALenum format;
    bool variableVol=false;
};

using stNode=struct sndNode
{
    bool isRegistered=false;
    int delayed=-1;
    bool started=false;
    ALuint source;
    int mode;
    bool allowMulti=false;
    int soundSpace;
    int elId=0;
    std::string eventType;
    std::string event;
    coords3d position;
    coords3d velocity;
};

using regNode=struct _regnd
{
    bool r=false;
    std::shared_ptr<stNode> stn;
};

using regSmpFileBuf=struct _rsmpFile
{
    bool r=false;
    ALuint buffer;
};

class soundManager
{
public:
    ~soundManager();
    static soundManager* getInstance();
    void registerSound(int chamberId,coords3d position,coords3d velocity,int elId,int typeId,int subtypeId,std::string eventType,std::string event);
    void registerMusic(int musicNo,int chamberId, coords3d position);
    void setListenerPosition(coords3d pos);
    void setListenerVelocity(coords3d pos);
    void setListenerChamber(int chamberId);
    void setListenerOrientation(coords3d pos);
    void enableSound();
    void stopSoundsByElementId(int elId);
    void setupSong(int songNo,coords3d position,int chamberId,bool vaiableVolume);
    void playSong(int songNo);

private:
   const bool isSongConfigured(int songNo,coords3d position,int chamberId);
   int findNearestMusic();
    void threadLoop();
    std::mutex snd_mutex;
    int calcDistance(coords3d a,coords3d b);
    ALenum determineFormat(SF_INFO fileInfo,SNDFILE *sndfile);
    void setSoundPosition(std::shared_ptr<stNode> snd,coords3d pos);
    void setSoundVelocity(std::shared_ptr<stNode> snd,coords3d pos);
    bool stopSnd(std::shared_ptr<stNode> n);
    std::shared_ptr<stNode> getSndNode();
    ALuint loadSample(std::string fname);
    static soundManager* instance;
    soundManager();
    ALCdevice *sndDevice;
    ALCcontext *sndContext;
    std::map<int, std::map< int, std::map<std::string,std::map<std::string, std::shared_ptr<sndHolder>>>>> samplesLoaded;
    std::map<std::string,ALuint> sampleBuffers; // Refactor me! - we need to load each file only once.
    std::vector<std::shared_ptr<stNode>> registeredSounds; // the whole sample data, used to register sounds
    std::map<int,std::map<std::string,std::map<std::string,regNode>>> sndRegister;
    std::vector<muNode> registeredMusic;
    std::map<std::string,regSmpFileBuf> sampleFile;
    int currSoundSpace=-1;
    bool isSndPlaying(ALint sndId);
    void checkQueue();
    coords3d listenerPos= {0,0,0};
    std::shared_ptr<gameConfig> gc;
    std::shared_ptr<configManager> cm;
    unsigned int cnt=0;
    bool active=false;
    int regSndPos=0;
    int currentMusic=1;
    std::thread myThread;
};

#endif // SOUNDMANAGER_H
