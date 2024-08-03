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

#include <istream>
// *** END ***
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
    ALuint buffer=0;
    int mode=0;
  //  bool allowMulti=false;


};


using muNode=struct mudNode
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
    bool variableVol=true;
    float gain=1.0;
    unsigned int bElemInstanceId=0;
};

using stNode=struct sndNode
{
    bool isRegistered=false;
    int delayed=-1;
    bool started=false;
    ALuint source;
    int mode;
    int soundSpace;
    unsigned int elId=0;
    int elType=0;
    std::string eventType;
    std::string event;
    float gain=1.0;
    coords3d position;
    coords3d velocity;
};

using regNode=struct regnd
{
    bool r=false;
    std::shared_ptr<stNode> stn;
};

using regSmpFileBuf=struct rsmpFile
{
    bool r=false;
    ALuint buffer=0;
};

class soundManager
{
public:
    ~soundManager();
    static soundManager* getInstance();
    std::shared_ptr<stNode> registerSound(int chamberId,coords3d position,coords3d velocity,int elId,int typeId,int subtypeId,std::string eventType,std::string event);
    void registerMusic(int musicNo,int chamberId, coords3d position);
    void setListenerPosition(coords3d pos);
    void setListenerVelocity(coords3d pos);
    void setListenerChamber(int chamberId);
    void setListenerOrientation(coords3d pos);
    void enableSound();
    void stopSoundsByElementId(unsigned int elId);
    int setupSong(unsigned int bElemInstanceId,int songNo,coords3d position,int chamberId,bool vaiableVolume);
    void pauseSong(unsigned int bElemInstanceId);
    void resumeSong(unsigned int bElemInstanceId);
    void playSong(int songNo);
    void moveSong(int songNo, coords3d newPosition,int newChamber);
private:
   const bool isSongConfigured(int songNo,coords3d position,int chamberId);
   int findNearestMusic();
    void threadLoop();
    std::mutex snd_mutex;
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
    std::map<int,std::map<int,std::map<std::string,std::map<std::string,regNode>>>> sndRegister;
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

    static std::once_flag _onceFlag;
};

#endif // SOUNDMANAGER_H
