#ifndef __CDROID_ACTIVITYTRANSITION_H__
#define __CDROID_ACTIVITYTRANSITION_H__
#include <cstdint>

namespace cdroid {

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

    // Factories return heap instances whose ownership transfers to Window::setEnterTransition etc.
    static ActivityTransition* fade(int64_t durationMs = 300) {
        ActivityTransition* t = new ActivityTransition();
        t->mType = Type::FADE;
        t->mDuration = durationMs;
        return t;
    }
    static ActivityTransition* slide(int edge, int64_t durationMs = 300) {
        ActivityTransition* t = new ActivityTransition();
        t->mType = Type::SLIDE;
        t->mSlideEdge = edge;
        t->mDuration = durationMs;
        return t;
    }

private:
    ActivityTransition() = default;
    Type    mType = Type::NONE;
    int     mSlideEdge = 0;
    int64_t mDuration = 300;
};

} // namespace cdroid
#endif
