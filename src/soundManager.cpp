#include "soundManager.h"

soundManager *soundManager::instance = nullptr;
soundManager::soundManager()
{
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
    }
    else
    {
        std::cout<<"Sound thinggy issue. Device did not exist?\n";
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

int soundManager::registerSound(int instanceId, int typeId, int subtypeId, std::string eventType, std::string event)
{
    std::shared_ptr<gameConfig> gc=configManager::getInstance()->getConfig(); // obtain game configuration structure
    if(!gc->samples[typeId][subtypeId][eventType][event].configured) // no configured samples, we return non existent
      {
          std::cout<<"Samples are not configured!\n";
         return -1;
      }
    // is Sample already in the sample table?
    if (this->samplesLoaded[typeId][subtypeId][eventType][event].loaded==false)
    {
        ALuint bid=this->loadSample(gc->samples[typeId][subtypeId][eventType][event].fname);
        if(bid==0)
            return -1;
        this->samplesLoaded[typeId][subtypeId][eventType][event].buffer=bid;
        this->samplesLoaded[typeId][subtypeId][eventType][event].loaded=true;
        this->samplesLoaded[typeId][subtypeId][eventType][event].mode=gc->samples[typeId][subtypeId][eventType][event].modeOfAction;
    }
    ALuint source;
    stNode srcNode;
    source = 0;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, (ALint)(this->samplesLoaded[typeId][subtypeId][eventType][event].buffer));
    assert(alGetError()==AL_NO_ERROR && "Failed to setup sound source");

    srcNode.source=source;
    srcNode.isRegistered=true;
    srcNode.id=(int)this->registeredSounds.size();
    srcNode.mode=this->samplesLoaded[typeId][subtypeId][eventType][event].mode;
    this->registeredSounds.push_back(srcNode);
    return (int)this->registeredSounds.size();
};



void soundManager::setListenerPosition(coords3d pos)
{
    alListener3f(AL_POSITION, (float)pos.x, (float)pos.y, (float)pos.z);
}

void soundManager::setListenerOrientation(coords3d pos)
{
    ALfloat listenerOri[] = { (float)pos.x, (float)pos.y, (float)pos.z,0.0,1.0,0.0};
    alListenerfv(AL_ORIENTATION, listenerOri);

}

void soundManager::setListenerVelocity(coords3d pos)
{
    alListener3f(AL_VELOCITY, (float)pos.x, (float)pos.y, (float)pos.z);
}


/* we just teleported, we need to switch the context, which means stopping all the currently played samples from the previous chamber*/
void soundManager::setListenerChamber(int chamberId)
{
    std::cout<<"We set the chamber for a listener\n";
    if(chamberId!=this->currSoundSpace && this->currSoundSpace!=-1)
    {
        std::cout<<"chamberChange\n";

        //Ok, now we have to do something about it
        for(unsigned int c=0;c<this->soundSpaces[this->currSoundSpace].size();c++)
        {
            alSourceStop(this->soundSpaces[this->currSoundSpace][c]);
        }
        soundSpaces[currSoundSpace].clear();
        currSoundSpace=chamberId;
    }
    currSoundSpace=chamberId;

}

void soundManager::setSoundVelocity(int sndId, coords3d pos)
{
    if(sndId<0 || (unsigned int)sndId>this->registeredSounds.size())
        return;
    alSource3f(this->registeredSounds[sndId].source,AL_VELOCITY,(float)pos.x,(float)pos.y,(float)pos.z);

}
void soundManager::setSoundPosition(int sndId, coords3d pos)
{
    if(sndId<0 || (unsigned int)sndId>this->registeredSounds.size())
        return;
    alSource3f(this->registeredSounds[sndId].source,AL_POSITION,(float)pos.x,(float)pos.y,(float)pos.z);
}

ALuint soundManager::loadSample(std::string fname)
{
    ALenum err, format;
    ALuint buffer;
    SNDFILE *sndfile;
    SF_INFO sfinfo;
    short *membuf;
    sf_count_t num_frames;
    ALsizei num_bytes;

    /* Open the audio file and check that it's usable. */
    sndfile = sf_open(fname.c_str(), SFM_READ, &sfinfo);
    if(!sndfile)
    {
        fprintf(stderr, "Could not open audio in %s: %s\n", fname.c_str(), sf_strerror(sndfile));
        return 0;
    }
    if(sfinfo.frames < 1 || sfinfo.frames > (sf_count_t)(INT_MAX/sizeof(short))/sfinfo.channels)
    {
        fprintf(stderr, "Bad sample count in %s (%" PRId64 ")\n", fname.c_str(), sfinfo.frames);
        sf_close(sndfile);
        return 0;
    }

    /* Get the sound format, and figure out the OpenAL format */
    format = AL_NONE;
    if(sfinfo.channels == 1)
        format = AL_FORMAT_MONO16;
    else if(sfinfo.channels == 2)
        format = AL_FORMAT_STEREO16;
    else if(sfinfo.channels == 3)
    {
        if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT2D_16;
    }
    else if(sfinfo.channels == 4)
    {
        if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT3D_16;
    }
    if(!format)
    {
        fprintf(stderr, "Unsupported channel count: %d\n", sfinfo.channels);
        sf_close(sndfile);
        return 0;
    }
    std::cout<<"buffer size needed "<<sfinfo.frames * sfinfo.channels* sizeof(short)<<"\n";
    /* Decode the whole audio file to a buffer. */
    membuf = (short *)malloc ((size_t)(sfinfo.frames * sfinfo.channels * sizeof(short)));

    num_frames = sf_readf_short(sndfile, membuf, sfinfo.frames);
    if(num_frames < 1)
    {
        free( membuf);
        sf_close(sndfile);
        fprintf(stderr, "Failed to read samples in %s (%" PRId64 ")\n", fname.c_str(), num_frames);
        return 0;
    }
    num_bytes = (ALsizei)(num_frames * sfinfo.channels) * (ALsizei)sizeof(short);
    std::cout<<"Num bytes: "<<num_bytes<<"\n";
    /* Buffer the audio data into a new buffer object, then free the data and
     * close the file.
     */
    buffer = 0;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate);

    free(membuf);
    sf_close(sndfile);

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
void soundManager::stopSoundsInSpace(int space)
{
     for(unsigned int c=0;c<this->soundSpaces[space].size();c++)
        {
            alSourceStop(this->soundSpaces[space][c]);
        }
}

void soundManager::startSoundsInSpace(int space)
{
    for(auto c:this->soundSpaces[space])
        alSourcePlay(c);
}


// We change active sound space
void soundManager::setActiveSndSpace(int sndSpace)
{
    if(this->currSoundSpace!=sndSpace)
    {
        this->stopSoundsInSpace(this->currSoundSpace);
        this->startSoundsInSpace(sndSpace);
        this->currSoundSpace=sndSpace;
    }
}


bool soundManager::playSound(int sndId, int chmbrId)
{
    if(chmbrId!=this->currSoundSpace || sndId<0)
        return false;
    alSourcePlay(this->registeredSounds[sndId].source);
    return false;
};
int soundManager::getSndStatus(int sndId)
{
    return -1;
};
bool soundManager::stopSnd(int sndId)
{
    alSourceStop(this->registeredSounds[sndId].source);
    return false;
};
bool soundManager::pauseSnd(int sndId)
{
    return false;
}
bool soundManager::resumeSnd(int sndId)
{
    return false;
}
void soundManager::deregisterSnd(int sndId)
{
}


