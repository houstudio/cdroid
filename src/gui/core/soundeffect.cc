#include <core/soundeffect.h>
#include <widget/view.h>
namespace cdroid{    
int SoundEffectConstants::getContantForFocusDirection(int direction) {
    switch (direction) {
    case View::FOCUS_RIGHT:    return SoundEffectConstants::NAVIGATION_RIGHT;
    case View::FOCUS_FORWARD:
    case View::FOCUS_DOWN:     return SoundEffectConstants::NAVIGATION_DOWN;
    case View::FOCUS_LEFT:     return SoundEffectConstants::NAVIGATION_LEFT;
    case View::FOCUS_BACKWARD:
    case View::FOCUS_UP:       return SoundEffectConstants::NAVIGATION_UP;
    default:
    throw "direction must be one of {FOCUS_UP, FOCUS_DOWN, FOCUS_LEFT, FOCUS_RIGHT, FOCUS_FORWARD, FOCUS_BACKWARD}.";
    }
    return SoundEffectConstants::NAVIGATION_DOWN;//NOT REACHED
}
}//namespace

