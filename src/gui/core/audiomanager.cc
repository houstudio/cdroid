#include <core/audiomanager.h>
#include <view/soundeffectconstants.h>
//#include <rtaudio/RtAudio.h>
namespace cdroid{

/*static int rtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
      double streamTime, RtAudioStreamStatus status, void *userData){

    return 0;
}*/

AudioManager::AudioManager(){
    mContext = nullptr;
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
    for(int i=SoundEffectConstants::CLICK;i<=SoundEffectConstants::NAVIGATION_REPEAT_DOWN+1;i++)
        playSoundEffect(i);
}

void  AudioManager::playSoundEffect(int effectType){
    const std::string res = mSoundEffects.get(effectType,"");
    LOGD("%d=%s",effectType,res.c_str());
}

void  AudioManager::playSoundEffect(int effectType, float volume){

}

}

