#include <media/soundpool.h>
#include <iostream>
#include <fstream>
#include <cstring>
#if ENABLE(AUDIO)
#include <rtaudio/RtAudio.h>
#endif
namespace cdroid{

SoundPool::SoundPool(){
#if ENABLE(AUDIO)
    audio = new RtAudio();
    if (audio->getDeviceCount() < 1) {
        LOGE("No audio devices found!");
    }
    LOGI("RtAudio.Version=%s",RtAudio::getVersion().c_str(),audio->getDeviceCount());
#else
    audio =nullptr;
#endif
}

SoundPool::~SoundPool() {
#if ENABLE(AUDIO)
    if (audio->isStreamOpen()) {
        audio->closeStream();
    }
    delete audio;
#endif
}

int32_t SoundPool::readChunk(std::istream&is,SoundPool::Sound&sound){
    uint8_t buf[32];
    uint32_t ret=8;
    auto& data=sound.data;
    is.read((char*)buf,8);
    uint32_t chunkSize = buf[4]|(buf[5]<<8)|(buf[6]<<16)|(buf[7]<<24);
    LOGD("chunkId:%c%c%c%c size %d",buf[0],buf[1],buf[2],buf[3],chunkSize);
    if((memcmp(buf,"RIFF",4)==0)||(memcmp(buf,"list",4)==0)){
        uint32_t subChunkSize=0;
        is.read((char*)buf,4);
        ret+=4;
        LOGD("chunkType:%c%c%c%c",buf[0],buf[1],buf[2],buf[3]);
        while(subChunkSize<chunkSize-4)subChunkSize+=readChunk(is,sound);
        is.seekg(chunkSize-4,std::ios_base::cur);
    }else{
        if(memcmp(buf,"fmt",3)==0){
            is.read((char*)buf,chunkSize);
            sound.format  = (buf[0]|buf[1]<<8);
            sound.channels= (buf[2]|buf[3]<<8);
            sound.sampleRate=buf[4]|(buf[5]<<8)|(buf[6]<<16)|(buf[7]<<24);
            sound.byteRate  =buf[8]|(buf[9]<<8)|(buf[10]<<16)|(buf[11]<<24);
            sound.blockAlign= buf[12]|(buf[13]<<8);
            sound.bitsPerSample=buf[14]|(buf[15]<<8);
        }else if(memcmp(buf,"data",4)==0){
            const size_t oldsize=data.size();
            data.resize(oldsize+chunkSize);
            is.read(data.data()+oldsize,chunkSize);
        }else{
            is.seekg(chunkSize,std::ios_base::cur);
        }
    }
    ret+=chunkSize;
    return ret;
}

int32_t SoundPool::load(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        LOGE("Failed to open file: ",filePath.c_str());
        return -1;
    }
    Sound sound;
    readChunk(file,sound);
    LOGD("\tAudioFormat:%d Channels=%d sampleRate=%d",sound.format,sound.channels,sound.sampleRate);
    LOGD("\tByteRate:%d blockAlign=%d bitsPerSample=%d",sound.byteRate,sound.blockAlign,sound.bitsPerSample);

    // For simplicity, assume the file is a raw PCM file with known parameters
    const int soundId = sounds.size();
    sound.position=0;
    sound.playing=false;
    sound.volume=1.f;
    sounds[soundId] = sound;
    return soundId;
}

void SoundPool::play(int soundId) {
    if (sounds.find(soundId) == sounds.end()) {
        LOGE("Sound ID %d not found!",soundId);
        return;
    }
    sounds[soundId].position=0;
    sounds[soundId].playing=true;
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
    sounds[soundId].playing=false;
    sounds[soundId].position=0;
#endif
}

void SoundPool::setVolume(int soundId, float volume) {
    if (sounds.find(soundId) != sounds.end()) {
        sounds[soundId].volume = volume;
    }
}

int32_t SoundPool::audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                             double streamTime, uint32_t/*RtAudioStreamStatus*/ status, void* userData) {
    SoundPool* soundPool = static_cast<SoundPool*>(userData);
    int16_t *buffer=(int16_t*)outputBuffer;
    for (auto& pair : soundPool->sounds) {
        Sound& sound = pair.second;
        if (sound.playing) {
            for (unsigned long i = 0; i < nBufferFrames; ++i) {
                if (sound.position < sound.data.size()) {
                    for(int c=0;c<sound.channels;c++){
                        int16_t sample = *(int16_t*)(sound.data.data()+sound.position);
                        buffer[i * 2+c] = static_cast<int16_t>(sample * sound.volume); // Left channel
                        sound.position += (sound.bitsPerSample>>3);
                    }
                } else {
                    sound.playing = false;
                    sound.position= 0;
                    break;
                }
            }
        }
    }
    return 0;
}
}/*endof namespace*/
