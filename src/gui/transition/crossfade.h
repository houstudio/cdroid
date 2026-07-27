/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.Crossfade (@hide).
 *
 * Bitmap note: android Bitmap → CDROID Cairo::ImageSurface. The view is snapshotted by
 * drawing it onto a Canvas backed by an ImageSurface. sameAs() (pixel comparison) is stubbed
 * as always-different (always crossfade) — pixel-diff is expensive and not ported.
 * alpha/bounds properties are driven by-Property (the by-name form resolves differently).
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_CROSSFADE_H__
#define __CDROID_TRANSITION_CROSSFADE_H__

#include <cairomm/surface.h>

#include <transition/transition.h>

namespace cdroid {

class Context;
class AttributeSet;
class BitmapDrawable;

/**
 * Captures bitmap (ImageSurface) representations of targets before and after the scene
 * change and fades between them. Ported from android-36 android.transition.Crossfade (@hide).
 */
class Crossfade: public Transition {
  public:
    static constexpr int FADE_BEHAVIOR_CROSSFADE = 0;
    static constexpr int FADE_BEHAVIOR_REVEAL    = 1;
    static constexpr int FADE_BEHAVIOR_OUT_IN    = 2;

    static constexpr int RESIZE_BEHAVIOR_NONE  = 0;
    static constexpr int RESIZE_BEHAVIOR_SCALE = 1;

    Crossfade() = default;
    Crossfade(Context* context, AttributeSet* attrs): Transition(context, attrs) {}

    Crossfade& setFadeBehavior(int fadeBehavior);
    int getFadeBehavior() const {
        return mFadeBehavior;
    }
    Crossfade& setResizeBehavior(int resizeBehavior);
    int getResizeBehavior() const {
        return mResizeBehavior;
    }

    Animator* createAnimator(ViewGroup* sceneRoot,
                             TransitionValues* startValues, TransitionValues* endValues) override;
    void captureStartValues(TransitionValues& transitionValues) override;
    void captureEndValues(TransitionValues& transitionValues) override;

    Crossfade* clone() const override {
        Crossfade* c = new Crossfade(*this);
        copyCloneFields(c);
        return c;
    }

  private:
    void captureValues(TransitionValues& transitionValues);

    static constexpr const char* PROPNAME_BITMAP   = "android:crossfade:bitmap";
    static constexpr const char* PROPNAME_DRAWABLE = "android:crossfade:drawable";
    static constexpr const char* PROPNAME_BOUNDS   = "android:crossfade:bounds";

    int mFadeBehavior = FADE_BEHAVIOR_REVEAL;
    int mResizeBehavior = RESIZE_BEHAVIOR_SCALE;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_CROSSFADE_H__
