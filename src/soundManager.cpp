#include "soundManager.h"

soundManager *soundManager::instance = nullptr;
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

        alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
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
            alSourcef(srcNode->source, AL_MAX_DISTANCE, 1000.f); // we want to hear from the distance 100 elements 100*32=3200
            alSourcef(srcNode->source, AL_REFERENCE_DISTANCE, 0.5f);
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
    if (soundManager::instance == nullptr)
        soundManager::instance = new soundManager();
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
            this->sndRegister[n->elId][n->eventType][n->event].r=false;
        }
    }
}



/* validates sounds in the queue, we do not have to do it often, only on new sound requests, just to control which samples are already not playing */
void soundManager::checkQueue()
{
   std::lock_guard<std::mutex> guard(this->snd_mutex);

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
        if (n->isRegistered && this->isSndPlaying(n->source) && (n->soundSpace!=this->currSoundSpace || this->calcDistance(this->listenerPos,n->position)>this->gc->soundDistance))
        {
            this->stopSnd(n);
            n->isRegistered=false;
            this->sndRegister[n->elId][n->eventType][n->event].r=false;
            continue;
        }
        if (n->isRegistered && !n->started)
        {
            n->started=true;
            alSourcePlay(n->source);
            continue;
        }
        if(n->isRegistered && this->isSndPlaying(n->source)==false)
        {
            n->isRegistered=false;
            this->sndRegister[n->elId][n->eventType][n->event].r=false;
        }
        if (this->isSndPlaying(n->source)==true)
        {
            float newVolume = 32.0/this->calcDistance(n->position,this->listenerPos);

            alSourcef(n->source, AL_GAIN, (newVolume>1)?1.0:newVolume);
        }
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

void soundManager::registerSound(int chamberId, coords3d position,coords3d velocity,int elId,int typeId, int subtypeId, std::string eventType, std::string event)
{

    alGetError();
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    /*
    That is a "parasite" kind of activity. That will change when sound manager gets its very own thread.
    It should be called periodically, but not too fast. we don't care for stale samples so much
    */
    if (this->cnt!=bElem::getCntr())
    {
        this->cnt=bElem::getCntr();
    }
    /*************************************************************/
    if (!this->active || chamberId!=this->currSoundSpace
            || this->calcDistance(this->listenerPos,position)>this->gc->soundDistance
            || !this->gc->samples[typeId][subtypeId][eventType][event].configured
            || (this->sndRegister[elId][eventType][event].r && this->gc->samples[typeId][subtypeId][eventType][event].allowMulti==false)
       )
        return;
    if(this->samplesLoaded[typeId][subtypeId][eventType][event].get()==nullptr )
        this->samplesLoaded[typeId][subtypeId][eventType][event]=std::make_shared<sndHolder>();
    if (this->samplesLoaded[typeId][subtypeId][eventType][event]->loaded==false)
    {
        if(this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].r==false)
        {
            ALuint bid=this->loadSample(this->gc->samples[typeId][subtypeId][eventType][event].fname);
            if(bid==0)
                return;
            this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].r=true;
            this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].buffer=bid;
        }

        this->samplesLoaded[typeId][subtypeId][eventType][event]->buffer=this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].buffer;
        this->samplesLoaded[typeId][subtypeId][eventType][event]->loaded=true;
        this->samplesLoaded[typeId][subtypeId][eventType][event]->mode=this->gc->samples[typeId][subtypeId][eventType][event].modeOfAction;
        this->samplesLoaded[typeId][subtypeId][eventType][event]->allowMulti=this->gc->samples[typeId][subtypeId][eventType][event].allowMulti;
    }
    std::shared_ptr<stNode> srcNode=this->getSndNode();
    alSourcei(srcNode->source, AL_BUFFER, (ALint)(this->samplesLoaded[typeId][subtypeId][eventType][event]->buffer));
    srcNode->isRegistered=true;
    srcNode->started=false;
    srcNode->position=position;
    srcNode->mode=this->samplesLoaded[typeId][subtypeId][eventType][event]->mode;
    srcNode->allowMulti=this->samplesLoaded[typeId][subtypeId][eventType][event]->allowMulti;
    srcNode->elId=elId;
    srcNode->eventType=eventType;
    srcNode->event=event;
    srcNode->soundSpace=chamberId;
    float newVolume = 32.0/this->calcDistance(position,this->listenerPos);
    alSourcef(srcNode->source, AL_GAIN, (newVolume>1)?1.0:newVolume);
    alSourcei(srcNode->source,AL_LOOPING,(srcNode->mode==0)?AL_FALSE:AL_TRUE);
    this->sndRegister[elId][eventType][event].r=true;
    this->sndRegister[elId][eventType][event].stn=srcNode;
    return;
};



int soundManager::calcDistance(coords3d a, coords3d b)
{
    return (int)sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z));
}
int soundManager::findNearestMusic()
{
    int dst=65535;
    int no=-1;
    for(unsigned int c=0;c<this->registeredMusic.size();c++)
    {
        if(this->registeredMusic[c].chamberId==this->currSoundSpace && (no<0 || (no>=0 && dst>this->calcDistance(this->listenerPos,this->registeredMusic[c].position))))
        {
            no=c;
            dst=this->calcDistance(this->listenerPos,this->registeredMusic[c].position);
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
            this->sndRegister[n->elId][n->eventType][n->event].r=false;
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
    for(unsigned int c=0;c<this->registeredMusic.size();c++)
    {
        if(this->registeredMusic[c].isRegistered && this->registeredMusic[c].position==position && this->registeredMusic[c].songNo==songNo && this->registeredMusic[c].chamberId==chamberId)
            return true;
    }
    return false;
}


void soundManager::setupSong(int songNo,coords3d position,int chamberId,bool vaiableVolume)
{
    // std::lock_guard<std::mutex> guard(this->snd_mutex);
    /* no music configured? */
    if (this->gc->music.size()<=0 || this->isSongConfigured(songNo,position,chamberId))
    {
        std::cout<<"No music configured!\n";
        return;
    }
    /* we deal with the problem of code and configuration mismatch */
    if (songNo<0 || this->gc->music.size()<(unsigned int)songNo)
        songNo=bElem::randomNumberGenerator()%this->gc->music.size();
    {
        muNode muNd;
        muNd.songNo=songNo;
        muNd.musicFile=sf_open(this->gc->music[songNo].filename.c_str(), SFM_READ, &(muNd.musFileinfo));
        if(!muNd.musicFile)
        {

            std::cout<<"Music file cannot be open "<<this->gc->music[songNo].filename<<"!\n";
            return;
        }
        if(muNd.musFileinfo.frames < 1 || muNd.musFileinfo.frames > (sf_count_t)(INT_MAX/sizeof(short))/muNd.musFileinfo.channels)
        {
            sf_close(muNd.musicFile); /* file contains no data */
            return;
        }
        muNd.format = this->determineFormat(muNd.musFileinfo,muNd.musicFile);
        if(!muNd.format)
        {
            sf_close(muNd.musicFile);
            return;
        }
        ALuint source;
        source = 0;
        alGenSources(1, &source);
        muNd.source=source;
        muNd.position=position;
        muNd.chamberId=chamberId;
        alSource3f(source,AL_POSITION,(float)position.x,(float)position.y,(float)position.z);

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
        muNd.variableVol=vaiableVolume;
       // alSourcePlay(muNd.source);
        alSourcef(muNd.source, AL_GAIN, 0.20f);
        std::lock_guard<std::mutex> guard(this->snd_mutex);
        this->registeredMusic.push_back(muNd);

    }
}


void soundManager::playSong(int songNo)
{
    ALint buffersProcessed = 0;
    float newVol=32.0/((float)this->calcDistance(this->listenerPos,this->registeredMusic[songNo].position));
    newVol=(newVol>0.5|| !this->registeredMusic[songNo].variableVol)?0.5:newVol;
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


void soundManager::threadLoop()
{
    while(this->active)
    {
        this->checkQueue();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}








