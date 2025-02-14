#include <media/soundpool.h>
#include <iostream>
#include <fstream>
#include <cstring>
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

int readChunk(std::istream&is,std::vector<char>&data){
    uint8_t buf[32];
    uint32_t ret=8;
    is.read((char*)buf,8);
    uint32_t chunkSize = buf[4]|(buf[5]<<8)|(buf[6]<<16)|(buf[7]<<24);
    LOGD("chunkId:%c%c%c%c size %d",buf[0],buf[1],buf[2],buf[3],chunkSize);
    if((memcmp(buf,"RIFF",4)==0)||(memcmp(buf,"list",4)==0)){
        uint32_t subChunkSize=0;
        is.read((char*)buf,4);
        ret+=4;
        LOGD("chunkType:%c%c%c%c",buf[0],buf[1],buf[2],buf[3]);
        while(subChunkSize<chunkSize-4)subChunkSize+=readChunk(is,data);
        is.seekg(chunkSize-4,std::ios_base::cur);
    }else{
        if(memcmp(buf,"fmt",3)==0){
            is.read((char*)buf,chunkSize);
            LOGD("\tAudioFormat:%d",buf[0]|buf[1]<<8);
            LOGD("\tChannels:%d",buf[2]|buf[3]<<8);
            LOGD("\tSampleRate:%d", buf[4]|(buf[5]<<8)|(buf[6]<<16)|(buf[7]<<24));
            LOGD("\tByteRate:%d",buf[8]|(buf[9]<<8)|(buf[10]<<16)|(buf[11]<<24));
            LOGD("\tBlockAlign:%d",buf[12]|buf[13]);
            LOGD("\tBitsPerSample:%d",buf[14],buf[15]);
        }else{
            is.seekg(chunkSize,std::ios_base::cur);
        }
    }
    ret+=chunkSize;
    return ret;
}

int SoundPool::load(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        LOGE("Failed to open file: ",filePath.c_str());
        return -1;
    }
    Sound sound;
    readChunk(file,sound.data);
    file.seekg(0, std::ios::end);
    sound.data.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(sound.data.data(), sound.data.size());
    file.close();

    // For simplicity, assume the file is a raw PCM file with known parameters
    sound.sampleRate = 44100;
    sound.channels = 2;
    sound.volume = 1.0f;
    sound.playing= true;
    sound.position=0;

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
    int16_t *buffer=(int16_t*)outputBuffer;
    // Implement audio playback logic here
    for (auto& pair : soundPool->sounds) {
        Sound& sound = pair.second;
        if (sound.playing) {
            for (unsigned long i = 0; i < nBufferFrames; ++i) {
                if (sound.position < sound.data.size()) {
                    int16_t sample = sound.data[sound.position];
                    buffer[i * 2] += static_cast<int16_t>(sample * sound.volume); // Left channel
                    buffer[i * 2 + 1] += static_cast<int16_t>(sample * sound.volume); // Right channel
                    sound.position += sound.channels;
                } else {
                    sound.playing = false;
                    break;
                }
            }
        }
    }
    return 0;
}
}/*endof namespace*/
