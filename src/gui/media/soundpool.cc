#include <media/soundpool.h>
#include <iostream>
#include <fstream>
#if ENABLE(AUDIO)
#include <rtaudio/RtAudio.h>
#endif
namespace cdroid{

SoundPool::SoundPool() : nextSoundId(1) {
#if ENABLE(AUDIO)
    audio = std::make_unique<RtAudio>();
    if (audio->getDeviceCount() < 1) {
        LOGE("No audio devices found!");
    }
    LOGI("RtAudio.Version=%s",RtAudio::getVersion().c_str(),audio->getDeviceCount());
#endif
}

SoundPool::~SoundPool() {
#if ENABLE(AUDIO)
    if (audio->isStreamOpen()) {
        audio->closeStream();
    }
#endif
}

int SoundPool::load(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        LOGE("Failed to open file: ",filePath.c_str());
        return -1;
    }

    Sound sound;
    file.seekg(0, std::ios::end);
    sound.data.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(sound.data.data(), sound.data.size());
    file.close();

    // For simplicity, assume the file is a raw PCM file with known parameters
    sound.sampleRate = 44100;
    sound.channels = 2;
    sound.volume = 1.0f;

    int soundId = nextSoundId++;
    sounds[soundId] = sound;
    return soundId;
}

void SoundPool::play(int soundId) {
    if (sounds.find(soundId) == sounds.end()) {
        LOGE("Sound ID %d not found!",soundId);
        return;
    }
#if ENABLE(AUDIO)
    if (!audio->isStreamOpen()) {
        RtAudio::StreamParameters parameters;
        parameters.deviceId = audio->getDefaultOutputDevice();
        parameters.nChannels = sounds[soundId].channels;
        parameters.firstChannel = 0;

        unsigned int bufferFrames = 512;
        RtAudioErrorType rtError=audio->openStream(&parameters, nullptr, RTAUDIO_SINT16, sounds[soundId].sampleRate, &bufferFrames, &audioCallback, this);
        LOGE_IF(rtError,"openStream=%d",rtError);
    }
    audio->startStream();
#endif
}

void SoundPool::stop(int soundId) {
#if ENABLE(AUDIO)
    if (audio->isStreamRunning()) {
        audio->stopStream();
    }
#endif
}

void SoundPool::setVolume(int soundId, float volume) {
    if (sounds.find(soundId) != sounds.end()) {
        sounds[soundId].volume = volume;
    }
}

int SoundPool::audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                             double streamTime, uint32_t/*RtAudioStreamStatus*/ status, void* userData) {
    SoundPool* soundPool = static_cast<SoundPool*>(userData);
    // Implement audio playback logic here
    //memcpy(outputBuffer, buffer, nBufferFrames * sizeof(float));
    return 0;
}
}/*endof namespace*/
