#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__
#include <core/sparsearray.h>
namespace cdroid{
class Context;
class AudioManager{
private:
    Context*mContext;
    SparseArray<std::string>mSoundEffects;
private:
    void setContext(Context* context);
public:
    AudioManager();
    AudioManager(Context*);
    void  playSoundEffect(int effectType);
    void  playSoundEffect(int effectType, float volume);
};

}/*endof namespace*/
#endif/*__AUDIO_MANAGER_H__*/

