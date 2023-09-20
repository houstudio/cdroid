#ifndef __SOUND_EFFECT_CONSTANTS_H__
#define __SOUND_EFFECT_CONSTANTS_H__
namespace cdroid{
class SoundEffectConstants {
private:
    static int sLastNavigationRepeatSoundEffectId;
    SoundEffectConstants() {}
public:
    static constexpr int CLICK = 0;
    static constexpr int NAVIGATION_LEFT = 1;
    static constexpr int NAVIGATION_UP = 2;
    static constexpr int NAVIGATION_RIGHT = 3;
    static constexpr int NAVIGATION_DOWN = 4;
    static constexpr int NAVIGATION_REPEAT_LEFT = 5;
    static constexpr int NAVIGATION_REPEAT_UP = 6;
    static constexpr int NAVIGATION_REPEAT_RIGHT = 7;
    static constexpr int NAVIGATION_REPEAT_DOWN = 8;

    static int getContantForFocusDirection(int direction);
    static int getConstantForFocusDirection(int direction, bool repeating);
    static bool isNavigationRepeat(int effectId);
    static int nextNavigationRepeatSoundEffectId();
};
}//namespace
#endif

