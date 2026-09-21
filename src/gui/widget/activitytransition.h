#ifndef __CDROID_ACTIVITYTRANSITION_H__
#define __CDROID_ACTIVITYTRANSITION_H__
#include <cstdint>
#include <animation/animation.h>

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

    /* An authored translate delta carried over from the window-animation
     * resource (android.view.animation.TranslateAnimation units). AOSP's WMS
     * plays the resource's own motion — popup_enter_material rises a mere
     * 20dp — so when the spec carries these the slide starts/ends at the
     * authored offset, and the edge-based full-offscreen formula is only the
     * fallback for specs without usable deltas (programmatic edge slides). */
    struct SlideDelta {
        int type = Animation::ABSOLUTE;   // ABSOLUTE px / RELATIVE_TO_SELF x size / RELATIVE_TO_PARENT x parent
        float value = 0.f;
        bool authored = false;            // extracted from a resource's translate child
        int resolve(int size, int parentSize) const {
            return (int)Animation::resolveSize(type, value, size, parentSize);
        }
    };

    Type    getType()       const { return mType; }
    int     getSlideEdge()  const { return mSlideEdge; }  // Gravity::LEFT/RIGHT/TOP/BOTTOM
    int64_t getDuration()   const { return mDuration; }   // milliseconds
    // The resource's interpolator and startOffset (AOSP window animations carry both).
    // BORROWED: window animations come from the process-wide style cache in
    // cdwindowtransitions.cc, which outlives every window; nullptr = the
    // animator default (AccelerateDecelerate), 0 = no delay.
    const TimeInterpolator* getInterpolator() const { return mInterpolator; }
    int64_t getStartOffset() const { return mStartOffset; }

    bool hasAuthoredDeltas() const { return mAuthoredDeltas; }
    const SlideDelta& fromX() const { return mFromX; }
    const SlideDelta& fromY() const { return mFromY; }
    const SlideDelta& toX() const { return mToX; }
    const SlideDelta& toY() const { return mToY; }
    /* The resource paired its translate with an alpha (popup_enter/exit_
     * material) — AOSP plays BOTH; the slide drives the translation and this
     * flag drives a parallel 0<->1 surface alpha so the motion fades in/out
     * instead of popping fully opaque and cutting at the end. */
    bool fadesAlong() const { return mFadeAlong; }
    void setFadeAlong(bool fade) { mFadeAlong = fade; }

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
    // Legacy edge-only slide (programmatic transitions; no authored deltas —
    // the edge's full-offscreen formula drives the motion).
    static ActivityTransition* slide(int edge, int64_t durationMs = 300,
            const TimeInterpolator* interpolator = nullptr, int64_t startOffset = 0) {
        return slide(edge, durationMs, interpolator, startOffset,
                SlideDelta(), SlideDelta(), SlideDelta(), SlideDelta());
    }
    static ActivityTransition* slide(int edge, int64_t durationMs,
            const TimeInterpolator* interpolator, int64_t startOffset,
            const SlideDelta& fromX, const SlideDelta& fromY,
            const SlideDelta& toX, const SlideDelta& toY) {
        ActivityTransition* t = new ActivityTransition();
        t->mType = Type::SLIDE;
        t->mSlideEdge = edge;
        t->mDuration = durationMs;
        t->mInterpolator = interpolator;
        t->mStartOffset = startOffset;
        t->mFromX = fromX; t->mFromY = fromY; t->mToX = toX; t->mToY = toY;
        t->mAuthoredDeltas = fromX.authored || fromY.authored || toX.authored || toY.authored;
        return t;
    }

private:
    ActivityTransition() = default;
    Type    mType = Type::NONE;
    int     mSlideEdge = 0;
    int64_t mDuration = 300;
    const TimeInterpolator* mInterpolator = nullptr;  // borrowed (style cache)
    int64_t mStartOffset = 0;
    SlideDelta mFromX, mFromY, mToX, mToY;
    bool mAuthoredDeltas = false;
    bool mFadeAlong = false;
};

} // namespace cdroid
#endif
