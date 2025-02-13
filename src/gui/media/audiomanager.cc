#include <cstring>
#include <fstream>
#include <core/context.h>
#include <media/soundpool.h>
#include <media/audiomanager.h>
#include <view/soundeffectconstants.h>
#include <porting/cdlog.h>
#if ENABLE(AUDIO)
#include <rtaudio/RtAudio.h>
#endif

namespace cdroid{

int readChunk(std::istream&is){
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
        while(subChunkSize<chunkSize-4)subChunkSize+=readChunk(is);
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

AudioManager::AudioManager(){
    mContext = nullptr;
    mSoundPool=std::make_unique<SoundPool>();
    //readChunk(f);
}

AudioManager::AudioManager(Context*ctx):AudioManager(){
    setContext(ctx);
}

void AudioManager::setContext(Context* context){
    mSoundEffects.put(SoundEffectConstants::CLICK,"click");
    mSoundEffects.put(SoundEffectConstants::NAVIGATION_LEFT,"navigation_left");
    mSoundEffects.put(SoundEffectConstants::NAVIGATION_UP,"navigation_up");
    mSoundEffects.put(SoundEffectConstants::NAVIGATION_RIGHT,"navigation_right");
    mSoundEffects.put(SoundEffectConstants::NAVIGATION_DOWN,"navigation_down");
    mSoundEffects.put(SoundEffectConstants::NAVIGATION_REPEAT_LEFT,"navigation_repeat_left");
    mSoundEffects.put(SoundEffectConstants::NAVIGATION_REPEAT_UP,"navigation_repeat_up");
    mSoundEffects.put(SoundEffectConstants::NAVIGATION_REPEAT_RIGHT,"navigation_repeat_right");
    mSoundEffects.put(SoundEffectConstants::NAVIGATION_REPEAT_DOWN,"navigation_repeat_down");
    for(int i=SoundEffectConstants::CLICK;i<=SoundEffectConstants::NAVIGATION_REPEAT_DOWN+1;i++){
        //mSoundPool.load();
        playSoundEffect(i);
    }
}

void  AudioManager::playSoundEffect(int effectType){
    const std::string res = mSoundEffects.get(effectType,"");
    LOGD("%d=%s",effectType,res.c_str());

}

void  AudioManager::playSoundEffect(int effectType, float volume){
    const std::string url = mSoundEffects.get(effectType);
}

}
