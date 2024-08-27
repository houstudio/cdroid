#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__
#include <core/sparsearray.h>
class RtAudio;
namespace cdroid{
class Context;
class AudioManager{
private:
    Context*mContext;
    std::shared_ptr<RtAudio>mDAC;
    char*mBuffer;
    uint32_t mBufferFrames;
    SparseArray<std::string>mSoundEffects;
private:
    void setContext(Context* context);
    static int AudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
          double streamTime,uint32_t status, void *userData);
public:
    AudioManager();
    AudioManager(Context*);
    void  playSoundEffect(int effectType);
    void  playSoundEffect(int effectType, float volume);
};

}/*endof namespace*/
#endif/*__AUDIO_MANAGER_H__*/

