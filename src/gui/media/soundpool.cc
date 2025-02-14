#include <media/soundpool.h>
#include <iostream>
#include <fstream>
#include <cstring>
#if ENABLE(AUDIO)
#include <rtaudio/RtAudio.h>
#endif
namespace cdroid{
struct SoundPool::Channel{
    std::shared_ptr<RtAudio>audio;
    int playingSounds;
};
struct SoundPool::Sound{
    std::vector<char> data;
    int32_t format;
    int32_t channels;
    int32_t sampleRate;
    int32_t byteRate;
    int16_t blockAlign;
    int16_t bitsPerSample;
    uint32_t position;
    float volume;
    bool playing;
};

SoundPool::SoundPool(){
}

SoundPool::~SoundPool() {
#if ENABLE(AUDIO)
    for(int i=0;i<mAudioChannels.size();i++){
	auto audio=mAudioChannels.valueAt(i)->audio;
        if (audio->isStreamOpen()) {
            audio->closeStream();
        }
    }
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
    auto sound=std::make_shared<Sound>();
    readChunk(file,*sound);
    LOGD("\tAudioFormat:%d Channels=%d sampleRate=%d",sound->format,sound->channels,sound->sampleRate);
    LOGD("\tByteRate:%d blockAlign=%d bitsPerSample=%d",sound->byteRate,sound->blockAlign,sound->bitsPerSample);

    // For simplicity, assume the file is a raw PCM file with known parameters
    const int soundId = mSounds.size();
    sound->position=0;
    sound->playing=false;
    sound->volume=1.f;
    switch(sound->format){
    case 1:switch(sound->bitsPerSample){
           case  8:sound->format = RTAUDIO_SINT8 ;break;
           case 16:sound->format = RTAUDIO_SINT16;break;
           case 32:sound->format = RTAUDIO_SINT32;break;
           }break;
    case 2:/*TODO:MS-ADPCM(Microsoft Adaptive Differential Pulse Code Modulation)*/break;
    case 3:sound->format=(sound->bitsPerSample=32)?RTAUDIO_FLOAT32:RTAUDIO_FLOAT64;break;
    }
    mSounds.put(soundId,sound);
    return soundId;
}

void SoundPool::play(int soundId) {
    auto sound=mSounds.get(soundId);
    if (sound==nullptr) {
        LOGE("Sound ID %d not found!",soundId);
        return;
    }
#if ENABLE(AUDIO)
    auto channel = mAudioChannels.get(sound->format);
    if(channel==nullptr){
	channel =std::make_shared<Channel>();
	channel->playingSounds=0;
	channel->audio = std::make_shared<RtAudio>();
	mAudioChannels.put(sound->format,channel);
    }
    if(sound->playing==false){
        sound->position=0;
        sound->playing =true;
	channel->playingSounds++;
    }
    auto& audio = channel->audio;
    if (!audio->isStreamOpen()) {
        RtAudio::StreamParameters parameters;
        parameters.deviceId = audio->getDefaultOutputDevice();
        parameters.nChannels = sound->channels;
        parameters.firstChannel = 0;

        unsigned int bufferFrames = 512;/*0 to detected the lowest allowable value*/;
#if RTAUDIO_VERSION_MAJOR>5
        RtAudioErrorType rtError=audio->openStream(&parameters, nullptr, RTAUDIO_SINT16, sound->sampleRate, &bufferFrames, &audioCallback, this);
        LOGD("openStream=%d bufferFrames=%d",rtError,bufferFrames);
#else
        audio->openStream(&parameters, nullptr,sound->format, sound->sampleRate, &bufferFrames, &audioCallback, this);
        LOGD("openStream.bufferFrames=%d",bufferFrames);
#endif
    }
    if(!audio->isStreamRunning())
        audio->startStream();
#endif
}

void SoundPool::stop(int soundId) {
#if ENABLE(AUDIO)
    auto sound = mSounds.get(soundId);
    if( (sound==nullptr)||(sound->playing==false))
	return;
    auto channel = mAudioChannels.get(sound->format);
    sound->playing = false;
    sound->position= 0;
    channel->playingSounds--;
    if(channel->playingSounds==0){
	auto audio = channel->audio;
	audio->stopStream();
	mAudioChannels.remove(sound->format);
    }
#endif
}

void SoundPool::setVolume(int soundId, float volume) {
    auto sound=mSounds.get(soundId);
    if (sound!=nullptr){
        sound->volume = volume;
    }
}

void SoundPool::sendOneSample(SoundPool::Sound&sound,void*outputBuffer,uint32_t i){
    for(int c=0;c<sound.channels;c++){
        switch(sound.format){
        case RTAUDIO_SINT8 :{
                int8_t sample = *(int8_t*)(sound.data.data()+sound.position);
                ((int8_t*)outputBuffer)[2*i+c] = static_cast<int8_t>(sample * sound.volume);
            }break;
        case RTAUDIO_SINT16:{
                int16_t sample = *(int16_t*)(sound.data.data()+sound.position);
                ((int16_t*)outputBuffer)[2*i+c] = static_cast<int16_t>(sample * sound.volume);
            }break;
        case RTAUDIO_SINT32:{
                int32_t sample = *(int32_t*)(sound.data.data()+sound.position);
                ((int32_t*)outputBuffer)[2*i+c] = static_cast<int32_t>(sample * sound.volume);
            }break;
        case RTAUDIO_FLOAT32:{
                float sample = *(float*)(sound.data.data()+sound.position);
                ((float*)outputBuffer)[2*i+c] = static_cast<float>(sample * sound.volume);
            }break;
        }
        sound.position += (sound.bitsPerSample>>3);
    }
}

int32_t SoundPool::audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                             double streamTime, uint32_t/*RtAudioStreamStatus*/ status, void* userData) {
    SoundPool* soundPool = static_cast<SoundPool*>(userData);
    int16_t *buffer=(int16_t*)outputBuffer;
    for (int k=0;k<soundPool->mSounds.size();k++) {
        auto sound = soundPool->mSounds.valueAt(k);
        if (!sound->playing) continue;
        for (unsigned long i = 0; i < nBufferFrames; ++i) {
            if (sound->position < sound->data.size()) {
                soundPool->sendOneSample(*sound,outputBuffer,i);
            } else {
                sound->playing = false;
                sound->position= 0;
                break;
            }
        }
    }
    return 0;
}
}/*endof namespace*/
