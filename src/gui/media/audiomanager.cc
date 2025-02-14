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

AudioManager::AudioManager(){
    mContext = nullptr;
    mSoundPool = std::make_unique<SoundPool>();
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
    mSoundPool->play(effectType);
}

void  AudioManager::playSoundEffect(int effectType, float volume){
    const std::string url = mSoundEffects.get(effectType);
    mSoundPool->play(effectType);
    mSoundPool->setVolume(effectType,volume);
}

}
