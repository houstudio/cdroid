#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__
#include <core/sparsearray.h>
class RtAudio;
namespace cdroid{
class Context;
class SoundPool;
class AudioManager{
public:
    /* Sound effect identifiers */
    static constexpr int FX_KEY_CLICK = 0;
    static constexpr int FX_FOCUS_NAVIGATION_UP = 1;
    static constexpr int FX_FOCUS_NAVIGATION_DOWN = 2;
    static constexpr int FX_FOCUS_NAVIGATION_LEFT = 3;
    static constexpr int FX_FOCUS_NAVIGATION_RIGHT= 4;
    static constexpr int FX_KEYPRESS_STANDARD = 5;
    static constexpr int FX_KEYPRESS_SPACEBAR = 6;
    static constexpr int FX_KEYPRESS_DELETE = 7;
    static constexpr int FX_KEYPRESS_RETURN = 8;
    static constexpr int FX_KEYPRESS_INVALID= 9;
    static constexpr int NUM_SOUND_EFFECTS = 10;
private:
    Context*mContext;
    std::unique_ptr<SoundPool>mSoundPool;
    std::vector<std::string>SOUND_EFFECT_FILES;
    std::vector<int>SOUND_EFFECT_FILES_MAP;
private:
    AudioManager(Context*);
    void loadTouchSoundAssetDefaults();
public:
    static AudioManager&getInstance();
    void playSoundEffect(int effectType);
    void playSoundEffect(int effectType,int userId);
    void playSoundEffect(int effectType, float volume);
    void loadSoundEffects();
    void unloadSoundEffects();
};

}/*endof namespace*/
#endif/*__AUDIO_MANAGER_H__*/

