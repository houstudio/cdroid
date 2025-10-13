/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <media/soundpool.h>
#include <core/context.h>
#include <porting/cdlog.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <rtaudio/RtAudio.h>

namespace cdroid{

struct SoundPool::Channel{
    SoundPool*pool;
    std::shared_ptr<RtAudio>audio;
    int deviceId;
    int format;
    int channels;
    int channelKey;
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
    int playingSounds;
};

struct SoundPool::Stream{
    int32_t soundId;
    int32_t priority;
    uint32_t position;
    float leftVolume;
    float rightVolume;
    float rate;
    int loop;
    bool playing;
};

SoundPool::SoundPool(int maxStreams, int streamType, int srcQuality){
    mNextStreamId=0;
    mMaxStreams=std::max(1,maxStreams);
#if ENABLE(AUDIO)
    std::ostringstream oss;
    std::vector<RtAudio::Api>apis;
    RtAudio::getCompiledApi(apis);
    oss<<"RtAudio "<<RtAudio::getVersion();
    oss<<" buildin with Apis [";
    for(int i=0;i<apis.size();i++){
        oss<<RtAudio::getApiDisplayName(apis.at(i));
        if(i<int(apis.size()-1))oss<<",";
    }
    if(apis.empty())oss<<"Dummy";
    oss<<"]";
    LOG(INFO)<<oss.str();
#else
    LOG(INFO)<<"RtAudio is not supported";
#endif
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
    LOGD("chunkId:%c%c%c%c size %d gc=%d",buf[0],buf[1],buf[2],buf[3],chunkSize,is.gcount());
    if((memcmp(buf,"RIFF",4)==0)||(memcmp(buf,"list",4)==0)){
        uint32_t subChunkSize=0;
        is.read((char*)buf,4);
        ret+=4;
        LOGD("chunkType:%c%c%c%c",buf[0],buf[1],buf[2],buf[3]);
        while(subChunkSize<chunkSize-4)subChunkSize+=readChunk(is,sound);
        //is.seekg(chunkSize-4,std::ios_base::cur);
    }else{
        if(memcmp(buf,"fmt",3)==0){
            is.read((char*)buf,chunkSize);
            sound.format  = (buf[0]|buf[1]<<8);
            sound.channels= (buf[2]|buf[3]<<8);
            sound.sampleRate= buf[4]|(buf[5]<<8)|(buf[6]<<16)|(buf[7]<<24);
            sound.byteRate  = buf[8]|(buf[9]<<8)|(buf[10]<<16)|(buf[11]<<24);
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

int32_t SoundPool::load(Context* context, const std::string& resId, int priority){
    auto sound = std::make_shared<Sound>();
    std::unique_ptr<std::istream> is;
#if ENABLE(AUDIO)
    if(context) is = context->getInputStream(resId);
    if((is==nullptr)||!(*is)){
        is = std::make_unique<std::ifstream>(resId, std::ios::in|std::ios::binary);
        if((is==nullptr)||(!*is)){
            LOGW("failed to load %s",resId.c_str());
            return -1;
        }
    }
    readChunk(*is,*sound);
    if(sound->format==0){
        LOGW("invalid ot unsupport audio format %s",resId.c_str());
        return -1;
    }
    LOGD("\tAudioFormat:%d Channels=%d sampleRate=%d",sound->format,sound->channels,sound->sampleRate);
    LOGD("\tByteRate:%d blockAlign=%d bitsPerSample=%d",sound->byteRate,sound->blockAlign,sound->bitsPerSample);
    // For simplicity, assume the file is a raw PCM file with known parameters
    std::lock_guard<std::recursive_mutex>_l(mLock);
    const int soundId = mSounds.size()+1;
    sound->playingSounds=0;
    switch(sound->format){
    case 1:
        switch(sound->bitsPerSample){
        case  8:sound->format = RTAUDIO_SINT8 ;break;
        case 16:sound->format = RTAUDIO_SINT16;break;
        case 32:sound->format = RTAUDIO_SINT32;break;
        }break;
    case 2:/*TODO:MS-ADPCM(Microsoft Adaptive Differential Pulse Code Modulation)*/break;
    case 3:sound->format=(sound->bitsPerSample==32)?RTAUDIO_FLOAT32:RTAUDIO_FLOAT64;break;
    }
    if(sound->format){
        mSounds.put(soundId,sound);
        return soundId;
    }
#endif
    return -1;
}

int32_t SoundPool::load(const std::string& filePath,int priority) {
    return load(nullptr,filePath,priority);
}

#define SOUNDKEY(s) ((s)->sampleRate*100+(s)->channels*10+(s)->format)
bool SoundPool::unload(int soundId){
    std::lock_guard<std::recursive_mutex>_l(mLock);
    auto sound = mSounds.get(soundId);
    if(sound){
        if(sound->playingSounds==0){
            mSounds.remove(soundId);
            return true;
        }
    }
    return false;
}

int SoundPool::play(int soundId){
    return play(soundId,1.f);
}

int SoundPool::play(int soundId,float volume){
    return play(soundId,volume,volume);
}

int SoundPool::play(int soundId,float leftVolume, float rightVolume) {
    return play(soundId,leftVolume,rightVolume,0,1,1.f);
}

int SoundPool::play(int soundId,float leftVolume, float rightVolume,int priority, int loop, float rate){
#if ENABLE(AUDIO)
    std::lock_guard<std::recursive_mutex>_l(mLock);
    auto sound = mSounds.get(soundId);
    if (sound==nullptr) {
        LOGE("Sound ID %d not found!",soundId);
        return -1;
    }
    const int channelKey = SOUNDKEY(sound);
    auto channel = mAudioChannels.get(channelKey);
    if(channel==nullptr){
        channel = std::make_shared<Channel>();
        channel->pool = this;
        channel->playingSounds = 0;
        channel->channels = sound->channels;
        channel->format = sound->format;
        channel->channelKey = channelKey;
        channel->audio = std::make_shared<RtAudio>();
        mAudioChannels.put(channelKey,channel);
        LOGD("create channel: %p  key:%d",channel,channelKey);
#if RTAUDIO_VERSION_MAJOR>5
        channel->audio->setErrorCallback([channel](RtAudioErrorType type,const std::string&errorText){
            LOGW("deviceId=%d Error:%d:%s",channel->deviceId,type,errorText.c_str());
        });
#endif
    }
    const int32_t streamId = ++mNextStreamId;
    auto stream = std::make_shared<Stream>();
    stream->soundId = soundId;
    stream->position= 0;
    stream->leftVolume = leftVolume;
    stream->rightVolume= rightVolume;
    stream->playing = true;
    stream->loop = loop;
    stream->rate = rate;
    mStreams.put(streamId,stream);
    channel->playingSounds++;

    auto& audio = channel->audio;
    if (!audio->isStreamOpen()) {
        RtAudio::StreamParameters parameters;
        const int deviceId = audio->getDefaultOutputDevice();
        unsigned int bufferFrames = 512;/*0 to detected the lowest allowable value*/;
        RtAudio::DeviceInfo dinfo = audio->getDeviceInfo(deviceId);
        parameters.deviceId = deviceId;
        parameters.nChannels = std::min(2U,dinfo.outputChannels);
        parameters.firstChannel = 0;
        channel->channels = parameters.nChannels;
        channel->deviceId = deviceId;
#if RTAUDIO_VERSION_MAJOR>5
        RtAudioErrorType rtError=audio->openStream(&parameters, nullptr, sound->format, sound->sampleRate, &bufferFrames, &audioCallback, channel.get());
        LOGD("%s openStream=%d bufferFrames=%d outputChanels=%d inputChannels=%d",dinfo.name.c_str(),rtError,bufferFrames,dinfo.outputChannels,dinfo.inputChannels);
#else
        audio->openStream(&parameters, nullptr,sound->format, sound->sampleRate, &bufferFrames, &audioCallback, channel.get(),nullptr/*StreamOptions*/,
                [](RtAudioError::Type type, const std::string &errorText){
            LOGW("%d:%s",type,errorText.c_str());
        });
        LOGD("%s openStream.bufferFrames=%d outputChanels=%d inputChannels=%d",dinfo.name.c_str(),bufferFrames,dinfo.outputChannels,dinfo.inputChannels);
#endif
        LOGD("vol=%.2f,%.2f loop=%d rate=%.2f",leftVolume,rightVolume,loop,rate);
    }
    if(!audio->isStreamRunning())
        audio->startStream();
    sound->playingSounds++;
    return streamId;
#else/*!ENABLE(AUDIO)*/
    return -1;
#endif
}

void SoundPool::stop(int streamId) {
#if ENABLE(AUDIO)
    std::lock_guard<std::recursive_mutex>_l(mLock);
    auto stream = mStreams.get(streamId);
    auto sound  = mSounds.get(stream->soundId);
    if(sound == nullptr)return;
    if(stream->playing==false)return;

    const int channelKey=SOUNDKEY(sound);
    auto channel = mAudioChannels.get(channelKey);
    stream->playing = false;
    stream->position= 0;
    sound->playingSounds--;
    channel->playingSounds--;
    if(channel->playingSounds==0){
        auto audio = channel->audio;
        audio->stopStream();
        audio->closeStream();
        mAudioChannels.remove(channelKey);
    }
#endif
}

void SoundPool::pause(int streamId){
    std::lock_guard<std::recursive_mutex>_l(mLock);
    auto stream =mStreams.get(streamId);
    if(stream&&stream->playing){
        stream->playing=false;
        auto sound  = mSounds.get(stream->soundId);
        const int channelKey=SOUNDKEY(sound);
        auto channel = mAudioChannels.get(channelKey);
        sound->playingSounds--;
        channel->playingSounds--;
    }
}

void SoundPool::resume(int streamId){
    std::lock_guard<std::recursive_mutex>_l(mLock);
    auto stream =mStreams.get(streamId);
    if(stream){
        stream->playing=true;
        auto sound  = mSounds.get(stream->soundId);
        const int channelKey=SOUNDKEY(sound);
        auto channel = mAudioChannels.get(channelKey);
        sound->playingSounds++;
        channel->playingSounds++;
    }
}

void SoundPool::autoPause(){
    std::lock_guard<std::recursive_mutex>_l(mLock);
    for(int i=0;i<mStreams.size();i++){
        pause(mStreams.keyAt(i));
    }
}

void SoundPool::autoResume(){
    std::lock_guard<std::recursive_mutex>_l(mLock);
    for(int i=0;i<mStreams.size();i++){
        resume(mStreams.keyAt(i));
    }
}

void SoundPool::setVolume(int streamId, float leftVolume, float rightVolume){
    std::lock_guard<std::recursive_mutex>_l(mLock);
    auto stream =mStreams.get(streamId);
    if (stream!=nullptr){
        stream->leftVolume = leftVolume;
        stream->rightVolume= rightVolume;
    }
}

void SoundPool::setVolume(int streamId, float volume) {
    setVolume(streamId,volume,volume);
}

void SoundPool::setPriority(int streamId, int priority){
    std::lock_guard<std::recursive_mutex>_l(mLock);
    auto stream = mStreams.get(streamId);
    if(stream)stream->priority = priority;
}

void SoundPool::setLoop(int streamId, int loop){
    std::lock_guard<std::recursive_mutex>_l(mLock);
    auto stream = mStreams.get(streamId);
    if(stream!=nullptr)stream->loop = loop;
}

void SoundPool::setRate(int streamId, float rate){
    auto stream = mStreams.get(streamId);
    if(stream)stream->rate = rate;
}

void SoundPool::sendOneSample(SoundPool::Channel*channel,void*outputBuffer,uint32_t i){
#if ENABLE(AUDIO)
    const int playingSounds = channel->playingSounds;
    std::vector<int64_t> sumedSampleInt(playingSounds*4,0);
    std::vector<float> sumedSampleFloat(playingSounds*4,0.f);
    for(int c = 0;c < channel->channels;c++){
        for(int i = 0;i < mStreams.size();i++){
            auto streamId = mStreams.keyAt(i);
            auto stream= mStreams.valueAt(i);
            auto sound = mSounds.get(stream->soundId);
            const uint32_t position = stream->position*stream->rate;
            const int32_t ccIndex = (c<sound->channels?c:sound->channels-1);/*for monochannels copy left to right*/
            const void*sample = (void*)(sound->data.data() + position + (sound->bitsPerSample>>3) * ccIndex);
            if((SOUNDKEY(sound)!=channel->channelKey)||(stream->playing==false))continue;
            switch(sound->format){
            case RTAUDIO_SINT8 : sumedSampleInt[c]+=static_cast<int8_t>(*((int8_t*)sample)*stream->leftVolume); break;
            case RTAUDIO_SINT16: sumedSampleInt[c]+=static_cast<int16_t>(*((int16_t*)sample)*stream->leftVolume);break;
            case RTAUDIO_SINT32: sumedSampleInt[c]+=static_cast<int32_t>(*((int32_t*)sample)*stream->leftVolume);break;
            case RTAUDIO_FLOAT32:sumedSampleFloat[c]+=static_cast<float>(*((float*)sample)*stream->leftVolume);break;
            }
            if((position>=sound->data.size())&&stream->playing){
                stream->loop--;
                stream->position= 0;
                if(!(stream->playing = (stream->loop>0))){
                    sound->playingSounds--;
                    channel->playingSounds--;
                }
                LOGD("soundid:%d streamid=%d loop=%d",stream->soundId,streamId,stream->loop);
            }
        }
    }
    for(int i=0;i<mStreams.size();i++){
        auto stream=mStreams.valueAt(i);
        auto sound =mSounds.get(stream->soundId);
        if(stream->playing)stream->position+=(sound->bitsPerSample>>3)*sound->channels;
    }
    for(int c=0;c<channel->channels&&playingSounds;c++){
        switch(channel->format){
        case RTAUDIO_SINT8  : ((int8_t*)outputBuffer)[2*i+c] = static_cast<int8_t>(sumedSampleInt[c]/playingSounds);break;
        case RTAUDIO_SINT16 : ((int16_t*)outputBuffer)[2*i+c] = static_cast<int16_t>(sumedSampleInt[c]/playingSounds);break;
        case RTAUDIO_SINT32 : ((int32_t*)outputBuffer)[2*i+c] = static_cast<int32_t>(sumedSampleInt[c]/playingSounds);break;
        case RTAUDIO_FLOAT32: ((float*)outputBuffer)[2*i+c] = static_cast<float>(sumedSampleFloat[c]/playingSounds);break;
        }
    }
#endif
}

int32_t SoundPool::audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                             double streamTime, uint32_t/*RtAudioStreamStatus*/ status, void* userData) {
    Channel* channel = static_cast<Channel*>(userData);
    SoundPool*soundPool = channel->pool;
    std::lock_guard<std::recursive_mutex>_l(soundPool->mLock);
    for(int i = 0; i < nBufferFrames ; i++)
        soundPool->sendOneSample(channel,outputBuffer,i);
    for(int i = soundPool->mStreams.size() - 1; i >= 0 ; i--){
        auto s = soundPool->mStreams.valueAt(i);
        if(s->playing==false){
            soundPool->mStreams.removeAt(i);
        }
    }
    return 0;
}
}/*endof namespace*/
