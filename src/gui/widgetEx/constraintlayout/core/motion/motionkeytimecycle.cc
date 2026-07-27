#include <widgetEx/constraintlayout/core/motion/motionkeytimecycle.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>
namespace cdroid {
bool MotionKeyTimeCycle::setValue(int type, float value) {
    using C = TypedValues::CycleType;
    using A = TypedValues::AttributesType;
    switch (type) {
    case C::TYPE_WAVE_PERIOD:
        mWavePeriod = value;
        break;
    case C::TYPE_WAVE_OFFSET:
        mWaveOffset = value;
        break;
    case C::TYPE_WAVE_PHASE:
        break; // not stored in TimeCycle
    case A::TYPE_ALPHA:
        mAlpha = value;
        break;
    case A::TYPE_ROTATION_Z:
        mRotation = value;
        break;
    case A::TYPE_ROTATION_X:
        mRotationX = value;
        break;
    case A::TYPE_ROTATION_Y:
        mRotationY = value;
        break;
    case A::TYPE_SCALE_X:
        mScaleX = value;
        break;
    case A::TYPE_SCALE_Y:
        mScaleY = value;
        break;
    case A::TYPE_TRANSLATION_X:
        mTranslationX = value;
        break;
    case A::TYPE_TRANSLATION_Y:
        mTranslationY = value;
        break;
    case A::TYPE_PATH_ROTATE:
        mTransitionPathRotate = value;
        break;
    case A::TYPE_PROGRESS:
        mProgress = value;
        break;
    default:
        return false;
    }
    return true;
}
} // namespace cdroid
