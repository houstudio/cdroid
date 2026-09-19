#ifndef __CDROID_ACTIVITYTRANSITION_H__
#define __CDROID_ACTIVITYTRANSITION_H__
#include <cstdint>

namespace cdroid {

class TimeInterpolator;

// Window-level Activity transition descriptor. This is NOT android.transition.Transition — it is a
// lightweight description of a whole-Window animation. CDROID's Window is the composition root, so
// only moving the Window itself (setPos) or setting its surface opacity (setAlpha) produces a
// visible transition; a content view's translationX cannot move the surface (composeSurfaces blits
// by getBound() and bypasses the View transform). Window owns instances set via setEnterTransition.
//   FADE  -> Window::setAlpha         (GFXSurfaceSetOpacity, whole-surface opacity)
//   SLIDE -> Window::setPos           (WindowManager::moveWindow, translates the whole window)
class ActivityTransition {
public:
    enum class Type { NONE, FADE, SLIDE };

    Type    getType()       const { return mType; }
    int     getSlideEdge()  const { return mSlideEdge; }  // Gravity::LEFT/RIGHT/TOP/BOTTOM
    int64_t getDuration()   const { return mDuration; }   // milliseconds
    // The resource's interpolator and startOffset (AOSP window animations carry both).
    // BORROWED: window animations come from the process-wide style cache in
    // cdwindowtransitions.cc, which outlives every window; nullptr = the
    // animator default (AccelerateDecelerate), 0 = no delay.
    const TimeInterpolator* getInterpolator() const { return mInterpolator; }
    int64_t getStartOffset() const { return mStartOffset; }

    // Factories return heap instances whose ownership transfers to Window::setEnterTransition etc.
    static ActivityTransition* fade(int64_t durationMs = 300,
            const TimeInterpolator* interpolator = nullptr, int64_t startOffset = 0) {
        ActivityTransition* t = new ActivityTransition();
        t->mType = Type::FADE;
        t->mDuration = durationMs;
        t->mInterpolator = interpolator;
        t->mStartOffset = startOffset;
        return t;
    }
    static ActivityTransition* slide(int edge, int64_t durationMs = 300,
            const TimeInterpolator* interpolator = nullptr, int64_t startOffset = 0) {
        ActivityTransition* t = new ActivityTransition();
        t->mType = Type::SLIDE;
        t->mSlideEdge = edge;
        t->mDuration = durationMs;
        t->mInterpolator = interpolator;
        t->mStartOffset = startOffset;
        return t;
    }

private:
    ActivityTransition() = default;
    Type    mType = Type::NONE;
    int     mSlideEdge = 0;
    int64_t mDuration = 300;
    const TimeInterpolator* mInterpolator = nullptr;  // borrowed (style cache)
    int64_t mStartOffset = 0;
};

} // namespace cdroid
#endif
