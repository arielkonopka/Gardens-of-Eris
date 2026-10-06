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
#include <algorithm>
#include "soundManager.h"
#include "soundSpace.h"
#include "gameSettings.h"
#include "randomStreams.h"
#include "performerStream.h"

namespace {
/// the Config menu's volumes, as OpenAL gain factors
float effectsVolume()
{
    return (float) gameSettings::getInstance().getEffectsVolume() / 100.0f;
}
float musicVolume()
{
    return (float) gameSettings::getInstance().getMusicVolume() / 100.0f;
}
} // namespace

soundManager::soundManager()
{
    this->cm = configManager::getInstance();
    this->gc = cm->getConfig();

    // Initialize Open AL
    this->sndDevice.reset(alcOpenDevice(nullptr)); // open default device
    if (this->sndDevice) {
        this->sndContext.reset(alcCreateContext(this->sndDevice.get(), nullptr));
        if (this->sndContext)
            alcMakeContextCurrent(this->sndContext.get()); // set active context
        // no Doppler: sounds are placed once per tick and do not carry a velocity
        alDopplerFactor(0.0f);
        // gain = reference / distance, the same falloff the manual volume used to give
        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
        ALCint frequency = 0;
        alcGetIntegerv(this->sndDevice.get(), ALC_FREQUENCY, 1, &frequency);
        if (frequency >= 8000 && frequency <= 192000)
            this->deviceRate = frequency;
        // the listener stays at the origin and every source is relative to it (see soundSpace.h)
        const ALfloat orientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        alListenerfv(AL_ORIENTATION, orientation);
        /* create the queue for sndefx and music */
        for (int c = 0; c < configManager::getInstance()->getConfig()->sndFifoSize; c++) {
            ALuint source;
            std::shared_ptr<stNode> srcNode = std::make_shared<stNode>(stNode());
            source = 0;
            alGenSources(1, &source);
            srcNode->source = source;
            srcNode->isRegistered = false;
            alSourcei(srcNode->source, AL_SOURCE_RELATIVE, AL_TRUE);
            alSourcef(srcNode->source, AL_REFERENCE_DISTANCE, soundSpace::referenceDistance);
            alSourcef(srcNode->source, AL_MAX_DISTANCE, (float) this->gc->soundDistance);
            alSourcef(srcNode->source, AL_ROLLOFF_FACTOR, 1.0f);
            alSourcef(srcNode->source, AL_PITCH, 1.0f);
            alSourcef(srcNode->source, AL_GAIN, 1.0f);
            this->registeredSounds.push_back(srcNode);
        }

    } else {
        std::cout << "Sound thinggy issue.\n Device did not exist?\n";
    }
}

soundManager::~soundManager()
{
    this->djListener = {}; // stops and joins the listening thread before anything it uses goes
    this->active = false;
    if (this->myThread.joinable())
        this->myThread.join(); // the loop checks the flag every 10 ms
    this->performer.reset(); // its source and buffers go while the context is still there
    if (this->sndContext)
        alcMakeContextCurrent(nullptr);
    // the context and then the device are released by their handles
}

soundManager &soundManager::getInstance()
{
    static soundManager instance;
    return instance;
}
/*
 * Stop all sound efx of an element, we stop it by id.
*/
void soundManager::stopSoundsByElementId(unsigned int elId)
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    this->pauseSongLocked(elId);
    for (auto n : this->registeredSounds) {
        if (n->elId == elId && n->mode > 0) // we kill only looping sounds, other will end anyway
        {
            this->stopSnd(n);
            n->isRegistered = false;
            this->sndRegister[n->elId][n->elType][n->eventType][n->event].r = false;
        }
    }
}

/**
 * @brief Examines the queue of registered sounds, ensuring that only relevant sound samples are played.
 *
 * This function is executed on periodically. It checks if the total count of sounds
 * has changed and accordingly updates the local counter. It also locates the nearest music and changes
 * the current music if necessary. For each registered sound, the function checks its status and updates
 * it based on certain conditions, including the distance from the listener and whether it is in the current sound space.
 * Finally, the volume of the sound is adjusted based on its distance from the listener.
 *
 * @note This function is thread-safe due to the usage of std::lock_guard.
 */
void soundManager::checkQueue()
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    this->cnt = bElem::getCntr();
    const auto source = gameSettings::getInstance().getMusicSource();
    if (source != this->playingSource) {
        this->silenceSongsLocked(); // the new choice starts its own music
        this->playingSource = source;
    }
    if (source == gameSettings::musicSource::performer) {
        this->silenceSongsLocked();
    } else if (!this->difficultySongs.empty()) {
        if (source == gameSettings::musicSource::dj)
            this->playDJMusic();
        else
            this->playDifficultyMusic();
    } else {
        int nm = this->findNearestMusic();
        if (nm != this->currentMusic) {
            if (this->currentMusic >= 0)
                alSourcePause(this->registeredMusic[this->currentMusic].source);
            if (nm >= 0) {
                this->currentMusic = nm;
                alSourcePlay(this->registeredMusic[this->currentMusic].source);
            };
        }
        if (this->currentMusic >= 0) {
            this->playSong(this->currentMusic);
        }
    }

    const float fx = effectsVolume();
    for (auto n : this->registeredSounds) {
        /* stop sounds from different board */
        if (n->isRegistered && this->isSndPlaying(n->source)
            && (n->soundSpace != this->currSoundSpace
                || this->listenerPos.distance(n->position) > this->gc->soundDistance)) {
            this->stopSnd(n);
            n->isRegistered = false;
            this->sndRegister[n->elId][n->elType][n->eventType][n->event].r = false;
            continue;
        }
        if (n->isRegistered && !n->started) {
            if (n->delayed < 0) {
                n->started = true;
                alSourcePlay(n->source);
            } else {
                n->delayed--;
                continue;
            }
        }

        if (n->isRegistered && this->isSndPlaying(n->source) == false) {
            n->isRegistered = false;
            this->sndRegister[n->elId][n->elType][n->eventType][n->event].r = false;
            continue;
        }

        if (n->isRegistered) {
            this->setSoundPosition(n, n->position); // the listener may have moved
            alSourcef(n->source, AL_GAIN, n->gain * fx); // the volume may have changed
        }
    }
}
void soundManager::enableSound()
{
    if (active)
        return;
    this->active = true;
    this->myThread = std::jthread(&soundManager::threadLoop, this);
}

std::shared_ptr<stNode> soundManager::registerSound(int chamberId,
                                                    coords3d position,
                                                    coords3d /*velocity*/,
                                                    int elId,
                                                    int typeId,
                                                    int subtypeId,
                                                    std::string eventType,
                                                    std::string event)
{
    alGetError();
    std::lock_guard<std::mutex> guard(this->snd_mutex);

    if (!this->gc->samples[typeId][subtypeId][eventType][event].configured
        && this->gc->samples[typeId][-1][eventType][event].configured) {
        subtypeId = -1;
    }

    if (!this->active || chamberId != this->currSoundSpace
        || this->listenerPos.distance(position) > this->gc->soundDistance
        || !this->gc->samples[typeId][subtypeId][eventType][event].configured
        || (this->sndRegister[elId][typeId][eventType][event].r
            && !this->gc->samples[typeId][subtypeId][eventType][event].allowMulti)) {
        return nullptr;
    }

    if (this->samplesLoaded[typeId][subtypeId][eventType][event].get() == nullptr)
        this->samplesLoaded[typeId][subtypeId][eventType][event] = std::make_shared<sndHolder>();
    if (!this->samplesLoaded[typeId][subtypeId][eventType][event]->loaded) {
        if (!this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].r) {
            ALuint bid = this->loadSample(
                this->gc->samples[typeId][subtypeId][eventType][event].fname);
            if (bid == 0)
                return nullptr;
            this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].r = true;
            this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].buffer
                = bid;
        }

        this->samplesLoaded[typeId][subtypeId][eventType][event]->buffer
            = this->sampleFile[this->gc->samples[typeId][subtypeId][eventType][event].fname].buffer;
        this->samplesLoaded[typeId][subtypeId][eventType][event]->loaded = true;
        this->samplesLoaded[typeId][subtypeId][eventType][event]->mode
            = this->gc->samples[typeId][subtypeId][eventType][event].modeOfAction;

        //      this->samplesLoaded[typeId][subtypeId][eventType][event]->allowMulti=this->gc->samples[typeId][subtypeId][eventType][event].allowMulti;
    }
    std::shared_ptr<stNode> srcNode = this->getSndNode();
    if (this->sndRegister[elId][typeId][eventType][event].r) {
        if (!this->gc->samples[typeId][subtypeId][eventType][event].stacking) {
            this->stopSnd(this->sndRegister[elId][typeId][eventType][event].stn);
            this->sndRegister[elId][typeId][eventType][event].stn->isRegistered = false;
        } else {
            if (!this->sndRegister[elId][typeId][eventType][event].stn->started) {
                srcNode->delayed = this->sndRegister[elId][typeId][eventType][event].stn->delayed
                                   + 5;
            } else {
                srcNode->delayed = -1;
            }
        }
    }
    alSourcei(srcNode->source,
              AL_BUFFER,
              (ALint) (this->samplesLoaded[typeId][subtypeId][eventType][event]->buffer));
    srcNode->isRegistered = true;
    srcNode->started = false;
    srcNode->elType = typeId;
    srcNode->position = position;
    srcNode->mode = this->samplesLoaded[typeId][subtypeId][eventType][event]->mode;
    srcNode->elId = elId;
    srcNode->eventType = eventType;
    srcNode->event = event;
    srcNode->gain = this->gc->samples[typeId][subtypeId][eventType][event].gain;
    srcNode->soundSpace = chamberId;
    alSourcef(srcNode->source, AL_GAIN, srcNode->gain * effectsVolume()); // OpenAL adds the distance falloff
    this->setSoundPosition(srcNode, position);
    alSourcei(srcNode->source, AL_LOOPING, (srcNode->mode == 0) ? AL_FALSE : AL_TRUE);
    this->sndRegister[elId][typeId][eventType][event].r = true;
    this->sndRegister[elId][typeId][eventType][event].stn = srcNode;
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
    int dst = 65535;
    int no = -1;

    for (unsigned int c = 0; c < this->registeredMusic.size();
         c++) { // we could check it in the same time, but then we would have to apply priority
        auto tmpdist = this->listenerPos.distance(this->registeredMusic[c].position);
        if (this->registeredMusic[c].isRegistered && this->registeredMusic[c].delayed <= 0)
            if ((this->registeredMusic[c].chamberId == this->currSoundSpace
                 || this->registeredMusic[c].chamberId == -1)
                && (no < 0 || dst > tmpdist)) {
                no = c;
                dst = tmpdist;
            }
    }
    return no;
}

/*
 * we get a source from the queue, if it is available, we return it.
 * available means: not registered, not playing at the moment, from other sound space
 */
std::shared_ptr<stNode> soundManager::getSndNode()
{
    std::shared_ptr<stNode> n = this->registeredSounds[this->regSndPos];
    unsigned int c = 0;
    this->regSndPos = (this->regSndPos + 1) % this->registeredSounds.size();
    while (n.get() != nullptr && n->isRegistered) {
        if ((!this->isSndPlaying(n->source)) || (n->mode > 0 && c > this->registeredSounds.size())
            || (c > this->registeredSounds.size() * 2)) {
            this->sndRegister[n->elId][n->elType][n->eventType][n->event].r = false;
            if (this->isSndPlaying(n->source))
                this->stopSnd(n);
            n->isRegistered = false;
            break;
        };
        c++;
        n = this->registeredSounds[this->regSndPos];
        this->regSndPos = (this->regSndPos + 1) % this->registeredSounds.size();
    }
    return n;
}

void soundManager::setListenerPosition(coords3d pos)
{
    // OpenAL's listener stays at the origin; sources are placed relative to this position
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    this->listenerPos = pos;
}

/* we just teleported, we need to switch the context, which means stopping all the currently played samples from the previous chamber*/
void soundManager::setListenerChamber(int chamberId)
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    this->currSoundSpace = chamberId;
}

void soundManager::setSoundPosition(std::shared_ptr<stNode> snd, coords3d pos)
{
    this->placeSource(snd->source, pos);
}

void soundManager::placeSource(ALuint source, coords3d pos)
{
    auto [x, y, z] = soundSpace::relative(pos, this->listenerPos);
    alSource3f(source, AL_POSITION, x, y, z);
}

ALenum soundManager::determineFormat(SF_INFO fileInfo, SNDFILE *sndfile)
{
    ALenum format = AL_NONE;
    if (fileInfo.channels == 1)
        format = AL_FORMAT_MONO16;
    else if (fileInfo.channels == 2)
        format = AL_FORMAT_STEREO16;
    else if (fileInfo.channels == 3) {
        if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT2D_16;
    } else if (fileInfo.channels == 4) {
        if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT3D_16;
    }
    if (!format) {
        return AL_NONE;
    }
    return format;
}

ALuint soundManager::loadSample(std::string fname)
{
    ALenum err, format;
    ALuint buffer;
    SF_INFO sfinfo;
    sf_count_t num_frames;
    ALsizei num_bytes;
    /* Open the audio file and check that it's usable. */
    std::unique_ptr<SNDFILE, goe::destroyWith<sf_close>> sndfile(sf_open(fname.c_str(), SFM_READ, &sfinfo));
    if (!sndfile)
        return 0;
    format = this->determineFormat(sfinfo,
                                   sndfile.get()); /* Get the sound format, and figure out the OpenAL format */
    // OpenAL only places mono sounds in space; stereo ones would play the same from everywhere
    const bool downmix = (format == AL_FORMAT_STEREO16);
    if (sfinfo.frames < 1
        || sfinfo.frames > (sf_count_t) (INT_MAX / sizeof(short)) / sfinfo.channels
        || format == AL_NONE) {
        return 0;
    }
    /* Decode the whole audio file to a buffer. */
    {
        std::vector<short> buff(sfinfo.frames * sfinfo.channels);
        num_frames = sf_readf_short(sndfile.get(), buff.data(), sfinfo.frames);
        sndfile.reset();
        if (num_frames < 1)
            return 0;
        int channels = sfinfo.channels;
        if (downmix) {
            for (sf_count_t f = 0; f < num_frames; f++)
                buff[f] = (short) (((int) buff[2 * f] + (int) buff[2 * f + 1]) / 2);
            channels = 1;
            format = AL_FORMAT_MONO16;
        }
        num_bytes = (ALsizei) (num_frames * channels) * (ALsizei) sizeof(short);
        /* Buffer the audio data into a new buffer object, then free the data and
         * close the file.
         */
        buffer = 0;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, format, buff.data(), num_bytes, sfinfo.samplerate);
    }

    /* Check if an error occured, and clean up if so. */
    err = alGetError();
    if (err != AL_NO_ERROR) {
        fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
        if (buffer && alIsBuffer(buffer))
            alDeleteBuffers(1, &buffer);
        return 0;
    }
    return buffer;
}

bool soundManager::isSndPlaying(ALint sndId)
{
    ALint source_state;
    alGetSourcei(sndId, AL_SOURCE_STATE, &source_state);
    if (source_state == AL_PLAYING)
        return true;
    return false;
}

bool soundManager::stopSnd(std::shared_ptr<stNode> n)
{
    alSourceStop(n->source);
    return this->isSndPlaying(n->source);
};

bool soundManager::isSongConfigured(int songNo, coords3d position, int chamberId)
{
    for (unsigned int c = 0; c < this->registeredMusic.size(); c++) {
        if (this->registeredMusic[c].songNo == songNo && this->registeredMusic[c].isRegistered
            && this->registeredMusic[c].position == position
            && this->registeredMusic[c].chamberId == chamberId)
            return true;
    }
    return false;
}

int soundManager::setupSong(
    unsigned int bElemInstanceId, int songNo, coords3d position, int chamberId, bool vaiableVolume)
{
    // levels built in the background register music too, while the sound thread plays it
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    /* no music configured? */
    if (this->gc->music.size() <= 0 || this->isSongConfigured(songNo, position, chamberId)) {
        return -1;
    }
    /* we deal with the problem of code and configuration mismatch */
    if (songNo < 0 || this->gc->music.size() <= (unsigned int) songNo) {
        songNo = goe::rng::audio() % this->gc->music.size();
    }
    muNode muNd;
    ALuint source;
    muNd.bElemInstanceId = bElemInstanceId;
    muNd.variableVol = vaiableVolume;
    muNd.delayed = 0;
    muNd.songNo = songNo;
    muNd.position = position;
    muNd.chamberId = chamberId;
    muNd.musFileinfo = {}; // sf_open in read mode needs format 0
    muNd.musicFile.reset(sf_open(this->gc->music[songNo].filename.c_str(), SFM_READ, &(muNd.musFileinfo)),
                         goe::destroyWith<sf_close>());
    if (!muNd.musicFile) {
        std::cout << "Music file cannot be open " << this->gc->music[songNo].filename << "!\n";
        return -1;
    }
    if (muNd.musFileinfo.frames < 1
        || muNd.musFileinfo.frames
               > (sf_count_t) (INT_MAX / sizeof(short)) / muNd.musFileinfo.channels) {
        return -1; /* music file contains no data */
    }
    muNd.format = this->determineFormat(muNd.musFileinfo, muNd.musicFile.get());
    if (!muNd.format)
        return -1;
    source = 0;
    alGenSources(1, &source);
    muNd.source = source;
    muNd.gain = this->gc->music[songNo].gain;
    // playSong sets the music volume by distance itself, so OpenAL only pans it
    alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);
    this->placeSource(source, position);
    alSourcef(source, AL_GAIN, std::min(muNd.gain, (float) 1.0) * musicVolume());
    const int buffersNum = 3;
    alGenBuffers(buffersNum, &muNd.Abuffers[0]);
    for (int n = 0; n < buffersNum; n++)
        if (!this->queueNextPiece(muNd, muNd.Abuffers[n]))
            break;
    muNd.isRegistered = true;
    this->registeredMusic.push_back(muNd);

    return this->registeredMusic.size() - 1;
}

void soundManager::playDifficultyMusic()
{
    const auto now = goe::music::byDifficulty::clock::now();
    const bool threatened = this->situationNow.load() != (int) goe::musician::situation::calm;
    const int pick
        = this->musicChoice.choose(this->difficultyNow, threatened, this->difficultySlots, now, goe::rng::audio());
    if (pick < 0)
        return;
    const int wanted = this->difficultySongs[pick];
    if (wanted != this->currentMusic) {
        // the song still fading out from an earlier change stops; the playing one fades out
        if (this->fadingMusic >= 0 && this->fadingMusic != wanted)
            alSourcePause(this->registeredMusic[this->fadingMusic].source);
        this->fadingMusic = this->currentMusic;
        this->currentMusic = wanted;
        alSourcePlay(this->registeredMusic[wanted].source);
    }
    const float mix = this->musicChoice.mix(now);
    this->playSong(this->currentMusic, mix);
    if (this->fadingMusic < 0)
        return;
    if (mix >= 1.0f) {
        alSourcePause(this->registeredMusic[this->fadingMusic].source);
        this->fadingMusic = -1;
    } else {
        this->playSong(this->fadingMusic, 1.0f - mix);
    }
}

void soundManager::setupDifficultyMusic()
{
    const int songs = (int) this->gc->music.size();
    std::vector<int> made;
    std::vector<goe::music::songSlot> slots;
    std::vector<std::string> files;
    for (int c = 0; c < songs; c++) {
        const int at = this->setupSong(0, c, {0.0f, 0.0f, 0.0f}, -1, false);
        if (at < 0)
            continue; // a missing song is left out, the others keep their levels
        made.push_back(at);
        slots.push_back({this->gc->music[c].difficulty, this->gc->music[c].danger});
        files.push_back(this->gc->music[c].filename);
    }
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    for (int at : made)
        this->registeredMusic[at].followsListener = true;
    this->difficultySongs = std::move(made);
    // the DJ knows only the danger marks until it has listened to the songs
    this->djSongs.clear();
    for (const auto &slot : slots)
        this->djSongs.push_back({.info = {}, .danger = slot.danger});
    this->djMap = goe::dj::mapSongs(this->djSongs);
    std::vector<float> gains;
    for (int at : this->difficultySongs)
        gains.push_back(this->registeredMusic[at].gain);
    if (!gains.empty()) {
        std::ranges::nth_element(gains, gains.begin() + gains.size() / 2);
        this->djGain = gains[gains.size() / 2];
    }
    this->difficultySlots = std::move(slots);
    this->djFiles = std::move(files);
}

void soundManager::followDifficulty(int d)
{
    this->difficultyNow = d;
}

void soundManager::playSong(int songNo, float mix)
{
    if (this->registeredMusic[songNo].followsListener)
        this->registeredMusic[songNo].position = this->listenerPos;
    ALint buffersProcessed = 0;
    float newVol = 5.5
                   * (this->registeredMusic[songNo].gain
                      / (0.01 + this->listenerPos.distance(this->registeredMusic[songNo].position)));
    newVol = (this->registeredMusic[songNo].variableVol)
                 ? std::min((float) this->registeredMusic[songNo].gain, newVol)
                 : this->registeredMusic[songNo].gain;
    newVol *= musicVolume() * mix;
    alGetSourcei(this->registeredMusic[songNo].source, AL_BUFFERS_PROCESSED, &buffersProcessed);
    alSourcef(this->registeredMusic[songNo].source, AL_GAIN, newVol);
    this->placeSource(this->registeredMusic[songNo].source, this->registeredMusic[songNo].position);
    if (buffersProcessed <= 0 || !this->registeredMusic[songNo].isRegistered
        || this->registeredMusic[songNo].delayed > 0) {
        return;
    }

    auto &song = this->registeredMusic[songNo];
    while (buffersProcessed--) {
        ALuint buffer;
        alSourceUnqueueBuffers(song.source, 1, &buffer);
        if (!song.queuedFrames.empty()) {
            song.framesDone += song.queuedFrames.front();
            song.queuedFrames.pop_front();
        }
        if (!this->queueNextPiece(song, buffer))
            break;
    }
}

bool soundManager::queueNextPiece(muNode &song, ALuint buffer)
{
    std::vector<short> buff(65536);
    const int channels = song.musFileinfo.channels;
    sf_count_t frames = sf_readf_short(song.musicFile.get(), buff.data(), (sf_count_t) buff.size() / channels);
    if (frames < 1) {
        sf_seek(song.musicFile.get(), 0, SEEK_SET);
        frames = sf_readf_short(song.musicFile.get(), buff.data(), (sf_count_t) buff.size() / channels);
        if (frames < 1)
            return false;
    }
    // only what was read: a buffer padded with silence would be a gap at the end of every loop
    alBufferData(buffer, song.format, buff.data(), (ALsizei) (frames * channels * sizeof(short)), song.musFileinfo.samplerate);
    alSourceQueueBuffers(song.source, 1, &buffer);
    song.queuedFrames.push_back((int) frames);
    return true;
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
void soundManager::moveSong(int songNo, coords3d newPosition, int newChamber)
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);

    // Sprawdź czy songNo jest prawidłowe
    if (songNo < 0 || (unsigned int) songNo >= this->registeredMusic.size())
        return;
    this->registeredMusic[songNo].position = newPosition;
    this->registeredMusic[songNo].chamberId = newChamber;
    this->placeSource(this->registeredMusic[songNo].source, newPosition);
}

bool soundManager::performerChosen()
{
    return gameSettings::getInstance().getMusicSource() == gameSettings::musicSource::performer;
}

void soundManager::silenceSongsLocked()
{
    for (int song : {this->currentMusic, this->fadingMusic, this->djPending})
        if (song >= 0) {
            alSourcePause(this->registeredMusic[(std::size_t) song].source);
            alSourcef(this->registeredMusic[(std::size_t) song].source, AL_PITCH, 1.0f);
        }
    // when the songs come back, the difficulty music (or the nearest song) starts again
    this->currentMusic = this->fadingMusic = this->djPending = -1;
    this->musicChoice = {};
    this->djChoice = {};
}

void soundManager::followSituation(goe::musician::situation s)
{
    this->situationNow = (int) s;
}

void soundManager::streamPerformer()
{
    const bool chosen = performerChosen();
    if (!this->sndContext || (!chosen && !this->performer))
        return;
    if (!this->performer) {
        const auto seed = ((std::uint64_t) goe::rng::audio() << 32) | goe::rng::audio();
        this->performer = std::make_unique<performerStream>(this->deviceRate, seed);
        std::cout << "Performer seed: " << seed << "\n";
    }
    if (chosen != this->performer->playing())
        this->performer->play(chosen);
    // the Config choices, every pump: they only store atomics, and a change is heard at once
    const auto &settings = gameSettings::getInstance();
    auto &musician = this->performer->musician();
    musician.setStyle(settings.getPerformerSound());
    musician.setGenre(settings.getMusicStyle());
    musician.setVariety((float) settings.getMusicVariety() / 100.0f);
    musician.setTempoScale((float) settings.getMusicTempo() / 100.0f);
    this->performer->pump(difficulty::musicianLevel(this->difficultyNow),
                          (goe::musician::situation) this->situationNow.load(),
                          musicVolume());
}

void soundManager::threadLoop()
{
    while (this->active) {
        this->checkQueue();
        this->streamPerformer();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void soundManager::pauseSong(unsigned int bElemInstanceId)
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    this->pauseSongLocked(bElemInstanceId);
}

void soundManager::pauseSongLocked(unsigned int bElemInstanceId)
{
    for (auto &c : this->registeredMusic) {
        if (c.bElemInstanceId == bElemInstanceId) {
            c.isRegistered = false;
            c.delayed = 555;
        }
    }
}

bool soundManager::hasSong(unsigned int bElemInstanceId)
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    return std::any_of(this->registeredMusic.begin(), this->registeredMusic.end(), [bElemInstanceId](const muNode &m) {
        return m.bElemInstanceId == bElemInstanceId;
    });
}

void soundManager::resumeSong(unsigned int bElemInstanceId)
{
    std::lock_guard<std::mutex> guard(this->snd_mutex);
    for (auto &c : this->registeredMusic) {
        if (c.bElemInstanceId == bElemInstanceId) {
            c.isRegistered = true;
            c.delayed = 0;
        }
    }
}
