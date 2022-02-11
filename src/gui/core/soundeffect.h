#ifndef __SOUND_EFFECT_CONSTANTS_H__
#define __SOUND_EFFECT_CONSTANTS_H__
namespace cdroid{
class SoundEffectConstants {
private:
    SoundEffectConstants() {}
public:
    static constexpr int CLICK = 0;
    static constexpr int NAVIGATION_LEFT = 1;
    static constexpr int NAVIGATION_UP = 2;
    static constexpr int NAVIGATION_RIGHT = 3;
    static constexpr int NAVIGATION_DOWN = 4;

    static int getContantForFocusDirection(int direction);
};
}//namespace
#endif

