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
#include "soundManager.h"

soundManager *soundManager::instance = nullptr;
std::once_flag soundManager::_onceFlag;
soundManager::soundManager()
{
    this->cm=configManager::getInstance();
    this->gc=cm->getConfig();


    this->sndContext=nullptr;
    // Initialize Open AL
    this->sndDevice = alcOpenDevice(NULL); // open default device
    if (this->sndDevice != nullptr)
    {
        this->sndContext=alcCreateContext(this->sndDevice,nullptr); // create context
        if (this->sndContext != nullptr)
        {
            alcMakeContextCurrent(this->sndContext); // set active context
        }
        alDopplerFactor(15.0);
        alDopplerVelocity(20);
        alSpeedOfSound(300.0);
        alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
        /* create the queue for sndefx and music */
        for(int c=0; c<configManager::getInstance()->getConfig()->sndFifoSize; c++)
        {
            ALuint source;
            std::shared_ptr<stNode> srcNode=std::make_shared<stNode>(stNode());
            source = 0;
            alGenSources(1, &source);
            srcNode->source=source;
            srcNode->isRegistered=false;
            alSourcei(srcNode->source, AL_SOURCE_RELATIVE, AL_TRUE);
            alSourcef(srcNode->source, AL_MAX_DISTANCE, 0.8f); // we want to hear from the distance 10 elements 10*32=3200
            alSourcef(srcNode->source, AL_REFERENCE_DISTANCE, 0.5f);
            alSourcef(srcNode->source,AL_ROLLOFF_FACTOR,4.0f);
            alSourcef(srcNode->source, AL_PITCH, 1.0f);
            alSourcef(srcNode->source, AL_GAIN, 1.0f);
            this->registeredSounds.push_back(srcNode);
        }

    }
    else
    {
        std::cout<<"Sound thinggy issue.\n Device did not exist?\n";
    }




}

soundManager::~soundManager()
{
    alcDestroyContext(this->sndContext);
    alcCloseDevice(this->sndDevice);
}
soundManager *soundManager::getInstance()
{

    std::call_once(soundManager::_onceFlag,[]()
    {
        soundManager::instance = new soundManager();
    });
    return soundManager::instance;
}
/*
 * Stop all sound efx of an element, we stop it by id.
*/
void soundManager::stopSoundsByElementId(int elId)
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    for(unsigned int c=0; c<this->registeredSounds.size(); c++)
    {
        std::shared_ptr<stNode> n=this->registeredSounds[c];
        if(n->elId==elId && n->mode>0) // we kill only looping sounds, other will end anyway
        {
            this->stopSnd(n);
            n->isRegistered=false;
            this->sndRegister[n->elId][n->elType][n->eventType][n->event].r=false;
        }
    }
}


/**
 * @brief Examines the queue of registered sounds, ensuring that only relevant sound samples are played.
 *
 * This function is executed on receiving new sound requests. It checks if the total count of sounds
 * has changed and accordingly updates the local counter. It also locates the nearest music and changes
 * the current music if necessary. For each registered sound, the function checks its status and updates
 * it based on certain conditions, including the distance from the listener and whether it is in the current sound space.
 * Finally, the volume of the sound is adjusted based on its distance from the listener.
 *
 * @note This function is thread-safe due to the usage of std::lock_guard.
 */void soundManager::checkQueue()
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    if (this->cnt!=bElem::getCntr())
    {
        this->cnt=bElem::getCntr();
    }
    int nm=this->findNearestMusic();

    if(nm>=0 && nm!=this->currentMusic)
    {
        //this->currentMusic=this->currentMusic%his->registeredMusic.size()
        if(this->currentMusic>=0)
            alSourcePause(this->registeredMusic[this->currentMusic].source);
        this->currentMusic=nm;
        if(this->currentMusic>=0)
            alSourcePlay(this->registeredMusic[this->currentMusic].source);

    };
    /**/
    {
        this->playSong(this->currentMusic);
    }
    for(unsigned int c=0; c<this->registeredSounds.size(); c++)
    {
        std::shared_ptr<stNode> n=this->registeredSounds[c];

        /* stop sounds from different board */
        if (n->isRegistered && this->isSndPlaying(n->source) && (n->soundSpace!=this->currSoundSpace || this->listenerPos.distance(n->position)>this->gc->soundDistance))
        {
            this->stopSnd(n);
            n->isRegistered=false;
            this->sndRegister[n->elId][n->elType][n->eventType][n->event].r=false;
            continue;
        }
        if (n->isRegistered && !n->started)
        {
            if(n->delayed<0)
            {
                n->started=true;
                alSourcePlay(n->source);
            }
            else
            {
                n->delayed--;
                continue;
            }
        }
        if(n->isRegistered && this->isSndPlaying(n->source)==false)
        {
            n->isRegistered=false;
            this->sndRegister[n->elId][n->elType][n->eventType][n->event].r=false;
            continue;
        }

        float newVolume = (n->gain/(n->position.distance(this->listenerPos)));
        alSourcef(n->source, AL_GAIN, (newVolume>2)?2.0:newVolume);

    }
}
void soundManager::enableSound()
{
    if(active)
        return;
    std::thread nt=std::thread(&soundManager::threadLoop, this);
    nt.detach();
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    this->active=true;
}

std::shared_ptr<stNode> soundManager::registerSound(int chamberId, coords3d position,coords3d velocity,int elId,int typeId, int subtypeId, std::string eventType, std::string event)
{

    alGetError();
    std::lock_guard<std::mutex> guard(this->snd_mutex);

    if(!this->gc->samples[typeId][subtypeId][eventType][event].configured && this->gc->samples[typeId][-1][eventType][event].configured)
    {
        subtypeId=-1;
    }

    if (!this->active || chamberId!=this->currSoundSpace
            || this->listenerPos.distance(position)>this->gc->soundDistance
            || !this->gc->samples[typeId][subtypeId][eventType][event].configured
            || (this->sndRegister[elId][typeId][eventType][event].r && this->gc->samples[typeId][subtypeId][eventType][event].allowMulti==false)
       )
    {
        return nullptr;
    }

    if(this->samplesLoaded[typeId][subtypeId][eventType][event].get()==nullptr )
        this->samplesLoaded[typeId][subtypeId][eventType][event]=std::make_shared<sndHolder>();
    if (this->samplesLoaded[typeId][subtypeId][eventType][event]->loaded==false)
    {
        if(this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].r==false)
        {
            ALuint bid=this->loadSample(this->gc->samples[typeId][subtypeId][eventType][event].fname);
            if(bid==0)
                return nullptr;
            this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].r=true;
            this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].buffer=bid;
        }

        this->samplesLoaded[typeId][subtypeId][eventType][event]->buffer=this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].buffer;
        this->samplesLoaded[typeId][subtypeId][eventType][event]->loaded=true;
        this->samplesLoaded[typeId][subtypeId][eventType][event]->mode=this->gc->samples[typeId][subtypeId][eventType][event].modeOfAction;

        //      this->samplesLoaded[typeId][subtypeId][eventType][event]->allowMulti=this->gc->samples[typeId][subtypeId][eventType][event].allowMulti;
    }
    std::shared_ptr<stNode> srcNode=this->getSndNode();
    if(this->sndRegister[elId][typeId][eventType][event].r)
    {
        if(this->gc->samples[typeId][subtypeId][eventType][event].stacking==false)
        {

            this->stopSnd(this->sndRegister[elId][typeId][eventType][event].stn);
            this->sndRegister[elId][typeId][eventType][event].stn->isRegistered=false;
        }
        else
        {
            if(!this->sndRegister[elId][typeId][eventType][event].stn->started)
            {
                srcNode->delayed=this->sndRegister[elId][typeId][eventType][event].stn->delayed+5;
            }
            else
            {
                srcNode->delayed=-1;
            }
        }
    }
    alSourcei(srcNode->source, AL_BUFFER, (ALint)(this->samplesLoaded[typeId][subtypeId][eventType][event]->buffer));
    srcNode->isRegistered=true;
    srcNode->started=false;
    srcNode->elType=typeId;
    srcNode->position=position;
    srcNode->mode=this->samplesLoaded[typeId][subtypeId][eventType][event]->mode;
    srcNode->elId=elId;
    srcNode->eventType=eventType;
    srcNode->event=event;
    srcNode->gain=this->gc->samples[typeId][subtypeId][eventType][event].gain;
    srcNode->soundSpace=chamberId;
    float newVolume = (srcNode->gain/(position.distance(this->listenerPos)));
    alSourcef(srcNode->source, AL_GAIN, (newVolume>2)?2.0:newVolume);
    alSourcei(srcNode->source,AL_LOOPING,(srcNode->mode==0)?AL_FALSE:AL_TRUE);
    this->sndRegister[elId][typeId][eventType][event].r=true;
    this->sndRegister[elId][typeId][eventType][event].stn=srcNode;
    return srcNode;
};





/**
 * @brief Determines the nearest music source in the same sound space as the listener.
 *
 * This function iterates over all registered music sources. For each music source, it checks if it is
 * located in the same sound space as the listener's current sound space. If so, it calculates the distance
 * from the listener. The function keeps track of the nearest music source and its distance from the listener.
 *
 * @return The index of the nearest music source. If no music source is found in the same sound space, returns -1.
 */
int soundManager::findNearestMusic()
{
    int dst=65535;
    int no=-1;
    for(unsigned int c=0; c<this->registeredMusic.size(); c++)
    { // we could check it in the same time, but then we would have to apply priority
        if(this->registeredMusic[c].chamberId==this->currSoundSpace && (no<0 || dst>this->listenerPos.distance(this->registeredMusic[c].position)))
        {
            no=c;
            dst=this->listenerPos.distance(this->registeredMusic[c].position);
        } else if (this->registeredMusic[c].chamberId==-1 && (no<0 || dst>this->listenerPos.distance(this->registeredMusic[c].position)))
        {
            no=c;
            dst=this->listenerPos.distance(this->registeredMusic[c].position);
        }
    }
    return no;
}

void soundManager::registerMusic(int musicNo, int chamberId, coords3d position)
{
    /*TBC! we should have all the registered samples for music in the config structure*/
}


/*
 * we get a source from the queue, if it is available, we return it.
 * available means: not registered, not playing at the moment, from other sound space
 */
std::shared_ptr<stNode> soundManager::getSndNode()
{
    std::shared_ptr<stNode> n=this->registeredSounds[this->regSndPos];
    unsigned int c=0;
    this->regSndPos=(this->regSndPos+1)%this->registeredSounds.size();
    while(n.get()!=nullptr && n->isRegistered)
    {
        if ((!this->isSndPlaying(n->source)) || (n->mode>0 && c>this->registeredSounds.size()) || (c>this->registeredSounds.size()*2))
        {
            this->sndRegister[n->elId][n->elType][n->eventType][n->event].r=false;
            if(this->isSndPlaying(n->source))
                this->stopSnd(n);
            n->isRegistered=false;
            break;
        };
        c++;
        n=this->registeredSounds[this->regSndPos];
        this->regSndPos=(this->regSndPos+1)%this->registeredSounds.size();
    }
    return n;
}


void soundManager::setListenerPosition(coords3d pos)
{
    alListener3f(AL_POSITION, (float)pos.x, (float)pos.y, (float)pos.z);
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    this->listenerPos=pos;
}

void soundManager::setListenerOrientation(coords3d pos)
{
    ALfloat listenerOri[] = { (float)pos.x, (float)pos.y, (float)pos.z,0.0,0.0,1.0};
    alListenerfv(AL_ORIENTATION, listenerOri);

}

void soundManager::setListenerVelocity(coords3d pos)
{
    alListener3f(AL_VELOCITY, (float)pos.x, (float)pos.y, (float)pos.z);
}


/* we just teleported, we need to switch the context, which means stopping all the currently played samples from the previous chamber*/
void soundManager::setListenerChamber(int chamberId)
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    this->currSoundSpace=chamberId;
}

void soundManager::setSoundVelocity(std::shared_ptr<stNode>  snd, coords3d pos)
{
    alSource3f(snd->source,AL_VELOCITY,(float)pos.x,(float)pos.y,(float)pos.z);
}
void soundManager::setSoundPosition(std::shared_ptr<stNode> snd, coords3d pos)
{
    alSource3f(snd->source,AL_POSITION,(float)pos.x,(float)pos.y,(float)pos.z);
}


ALenum soundManager::determineFormat(SF_INFO fileInfo,SNDFILE *sndfile)
{
    ALenum format=AL_NONE;
    if(fileInfo.channels == 1)
        format = AL_FORMAT_MONO16;
    else if(fileInfo.channels == 2)
        format = AL_FORMAT_STEREO16;
    else if(fileInfo.channels == 3)
    {
        if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT2D_16;
    }
    else if(fileInfo.channels == 4)
    {
        if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT3D_16;
    }
    if(!format)
    {
        return AL_NONE;
    }
    return format;
}



ALuint soundManager::loadSample(std::string fname)
{
    ALenum err, format;
    ALuint buffer;
    SNDFILE *sndfile;
    SF_INFO sfinfo;
    sf_count_t num_frames;
    ALsizei num_bytes;
    /* Open the audio file and check that it's usable. */
    sndfile = sf_open(fname.c_str(), SFM_READ, &sfinfo);
    if(!sndfile)
        return 0;
    format = this->determineFormat(sfinfo,sndfile); /* Get the sound format, and figure out the OpenAL format */
    if(sfinfo.frames < 1 || sfinfo.frames > (sf_count_t)(INT_MAX/sizeof(short))/sfinfo.channels || format==AL_NONE)
    {
        sf_close(sndfile);
        return 0;
    }
    /* Decode the whole audio file to a buffer. */
    {
        std::vector<short> buff(sfinfo.frames * sfinfo.channels);
        num_frames = sf_readf_short(sndfile, buff.data(), sfinfo.frames);
        sf_close(sndfile);
        if(num_frames < 1)
            return 0;
        num_bytes = (ALsizei)(num_frames * sfinfo.channels) * (ALsizei)sizeof(short);
        /* Buffer the audio data into a new buffer object, then free the data and
         * close the file.
         */
        buffer = 0;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, format, buff.data(), num_bytes, sfinfo.samplerate);
    }

    /* Check if an error occured, and clean up if so. */
    err = alGetError();
    if(err != AL_NO_ERROR)
    {
        fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
        if(buffer && alIsBuffer(buffer))
            alDeleteBuffers(1, &buffer);
        return 0;
    }
    return buffer;
}




bool soundManager::isSndPlaying(ALint sndId)
{
    ALint source_state;
    alGetSourcei(sndId, AL_SOURCE_STATE, &source_state);
    if (source_state==AL_PLAYING)
        return true;
    return false;

}


bool soundManager::stopSnd(std::shared_ptr<stNode> n)
{
    alSourceStop(n->source);
    return this->isSndPlaying(n->source);
};

const bool soundManager::isSongConfigured(int songNo, coords3d position, int chamberId)
{
    for(unsigned int c=0; c<this->registeredMusic.size(); c++)
    {
        if(this->registeredMusic[c].songNo==songNo && this->registeredMusic[c].isRegistered && this->registeredMusic[c].position==position &&  this->registeredMusic[c].chamberId==chamberId)
            return true;
    }
    return false;
}


int soundManager::setupSong(unsigned int bElemInstanceId,int songNo,coords3d position,int chamberId,bool vaiableVolume)
{
    // std::lock_guard<std::mutex> guard(this->snd_mutex);
    /* no music configured? */
    if (this->gc->music.size()<=0 || this->isSongConfigured(songNo,position,chamberId))
    {
        std::cout<<"No music configured!\n";
        return -1;
    }
    /* we deal with the problem of code and configuration mismatch */
    if (songNo<0 || this->gc->music.size()<(unsigned int)songNo) {
        songNo = bElem::randomNumberGenerator() % this->gc->music.size();
    }
    muNode muNd;
    muNd.bElemInstanceId=bElemInstanceId;
    muNd.variableVol=vaiableVolume;
    muNd.songNo=songNo;
    muNd.position=position;
    muNd.chamberId=chamberId;
    muNd.musicFile=sf_open(this->gc->music[songNo].filename.c_str(), SFM_READ, &(muNd.musFileinfo));
    if(!muNd.musicFile)
    {
        std::cout<<"Music file cannot be open "<<this->gc->music[songNo].filename<<"!\n";
        return -1;
    }
    if(muNd.musFileinfo.frames < 1 || muNd.musFileinfo.frames > (sf_count_t)(INT_MAX/sizeof(short))/muNd.musFileinfo.channels)
    {
        sf_close(muNd.musicFile); /* music file contains no data */
        return -1;
    }
    muNd.format = this->determineFormat(muNd.musFileinfo,muNd.musicFile);
    if(!muNd.format)
    {
        sf_close(muNd.musicFile);
        return -1;
    }
    ALuint source;
    source = 0;
    alGenSources(1, &source);
    muNd.source=source;

    muNd.gain=this->gc->music[songNo].gain;
    alSource3f(source,AL_POSITION,(float)position.x,(float)position.y,(float)position.z);
    alSourcef(source,AL_GAIN,std::min(muNd.gain,(float)1.0));
    const int buffersNum=3;
    alGenBuffers(buffersNum, &muNd.Abuffers[0]);
    for(int n=0; n<buffersNum; n++)
    {
        std::vector<short> buff(65536);
        int num_frames = sf_readf_short(muNd.musicFile, buff.data(), buff.size()/muNd.musFileinfo.channels);
        if(num_frames < 1)
            break;
        alBufferData(muNd.Abuffers[n],muNd.format,buff.data(),buff.size()*sizeof(short),muNd.musFileinfo.samplerate);
    }
    alSourceQueueBuffers(muNd.source,buffersNum,&muNd.Abuffers[0]);
    muNd.isRegistered=true;


    // alSourcePlay(muNd.source);
    //      alSourcef(muNd.source, AL_GAIN, 0.10f);

    std::lock_guard<std::mutex> guard(this->snd_mutex);

     this->registeredMusic.push_back(muNd);

     return this->registeredMusic.size()-1;


}


void soundManager::playSong(int songNo)
{
    ALint buffersProcessed = 0;
    float newVol=5.5*(this->registeredMusic[songNo].gain/(0.01+this->listenerPos.distance(this->registeredMusic[songNo].position)));
    newVol=(this->registeredMusic[songNo].variableVol)?std::min((float)this->registeredMusic[songNo].gain,newVol):this->registeredMusic[songNo].gain;
    alGetSourcei(this->registeredMusic[songNo].source, AL_BUFFERS_PROCESSED, &buffersProcessed);
    alSourcef(this->registeredMusic[songNo].source, AL_GAIN, newVol);
    if(buffersProcessed <= 0)
        return;

    while(buffersProcessed--)
    {
        ALuint buffer;
        alSourceUnqueueBuffers( this->registeredMusic[songNo].source, 1, &buffer);

        std::vector<short> buff(65536);
        int num_frames = sf_readf_short(this->registeredMusic[songNo].musicFile, buff.data(), buff.size()/this->registeredMusic[songNo].musFileinfo.channels);
        if(num_frames < 1)
        {
            sf_seek(this->registeredMusic[songNo].musicFile,0,0);
            num_frames = sf_readf_short(this->registeredMusic[songNo].musicFile, buff.data(), buff.size()/this->registeredMusic[songNo].musFileinfo.channels);
            if(num_frames < 1)
                break;
        }
        alBufferData(buffer,this->registeredMusic[songNo].format,buff.data(),buff.size()*sizeof(short),this->registeredMusic[songNo].musFileinfo.samplerate);
        alSourceQueueBuffers(this->registeredMusic[songNo].source, 1, &buffer);


    }


}

/**
 * @brief Adjust the playback position for a specified song.
 *
 * This method affords the luxury of altering the spatial positioning of a particular song's playback, thus endowing the sound with the capacity for movement during its performance. It represents a somewhat more advanced feature in audio management, providing an aural environment that can adapt dynamically according to the requirements of the listener.
 *
 * @param songNo The specific designation, or number, attributed to the song that is due for relocation. This should correspond to a valid index within the registeredMusic vector.
 * @param newPosition The intended three-dimensional coordinates where the song shall henceforth be situated, creating an auditory illusion of spatial displacement.
 * @param newChamber The board, that would be our sound chamber.
 * @return Void as the main purpose of this function is to produce side effects, namely the adjustment of audio position, rather than a data return.
 *
 * @note This method employs a thread-safe design, utilising the class's mutex for synchronisation purposes. Therefore, it is perfectly suited to an environment where multiple threads are in operation, ensuring no untoward clashes or conflicts arise in the process of adjusting the song's position.
 */
void soundManager::moveSong( int songNo, coords3d newPosition,int newChamber)
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);


    // Sprawdź czy songNo jest prawidłowe
    if (songNo < 0 || (unsigned int )songNo >= this->registeredMusic.size())
        return;
    this->registeredMusic[songNo].position = newPosition;
    this->registeredMusic[songNo].chamberId=newChamber;
    alSource3f(this->registeredMusic[songNo].source, AL_POSITION,
               (float)newPosition.x, (float)newPosition.y, (float)newPosition.z);
}

void soundManager::threadLoop()
{
    while(this->active)
    {
        this->checkQueue();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}








