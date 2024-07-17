#include <cstring>
#include <core/context.h>
#include <core/audiomanager.h>
#include <view/soundeffectconstants.h>
#if ENABLE(AUDIO)
#include <rtaudio/RtAudio.h>
#endif
namespace cdroid{
#if ENABLE(AUDIO)
static int RtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
      double streamTime, RtAudioStreamStatus status, void *userData){
    float *buffer = (float *)userData;
    for(int i=0;i<nBufferFrames;i++)
	buffer[i]=float(i)/nBufferFrames;
    /*将数据复制到输出缓冲区*/
    memcpy(outputBuffer, buffer, nBufferFrames * sizeof(float));
    return 0;
}

static RtAudio dac;
RtAudio::StreamParameters parameters;
unsigned int bufferFrames = 256; // 缓冲区帧数
float *buffer = new float[bufferFrames]; // 要播放的音频数据，这里简单初始化为示例音频数据
#endif

AudioManager::AudioManager(){
    mContext = nullptr;
#if ENABLE(AUDIO)
    RtAudio::StreamParameters parameters;
    LOGD("%d AudioDevice Found",dac.getDeviceCount());
    for (unsigned int i = 0; i < dac.getDeviceCount(); i++) {
        RtAudio::DeviceInfo info = dac.getDeviceInfo(i);
        LOGD("Device[%d](%s) %dxIn + %dxOut", i,info.name.c_str(),info.duplexChannels,info.outputChannels);
    }
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = 1; // 单声道
    parameters.firstChannel = 0; // 第一个声道
    RtAudioFormat format = RTAUDIO_FLOAT32; // 32位浮点数格式
    dac.openStream(&parameters, nullptr, format, 44100, &bufferFrames, &RtAudioCallback, (void *)buffer);
    dac.startStream();
#endif
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

